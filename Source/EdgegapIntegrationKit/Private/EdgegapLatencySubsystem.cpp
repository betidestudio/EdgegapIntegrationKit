// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EdgegapLatencySubsystem.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Common/UdpSocketBuilder.h"

void UEdgegapLatencySubsystem::StartLatencyTestForMultipleServers(const TArray<FIPPortInfo>& ServerList, FOnLatencyTestCompleted OnLatencyTestCompleted)
{
    OnLatencyTestCompletedDelegate = OnLatencyTestCompleted;
    // Copy the server list and initialize the index
    ServersToTest = ServerList;
    CurrentServerIndex = 0;

    if (ServersToTest.Num() > 0)
    {
        // Start by sending a ping to the first server
        MoveToNextServer();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No servers provided for latency test."));
    }
}

bool UEdgegapLatencySubsystem::InitializeSocket(int32 Port)
{
    // Create the UDP socket
    UDPSocket = FUdpSocketBuilder(TEXT("EdgegapLatencySocket"))
        .AsNonBlocking()
        .AsReusable()
        .WithReceiveBufferSize(2 * 1024 * 1024);  // Set buffer size

    return (UDPSocket != nullptr);
}

void UEdgegapLatencySubsystem::SendPingToServer(const FIPPortInfo& ServerInfo)
{
    // Parse the IP address
    FIPv4Address EdgegapAddress;
    if (!FIPv4Address::Parse(ServerInfo.IPAddress, EdgegapAddress))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid IP address format: %s"), *ServerInfo.IPAddress);
        MoveToNextServer();  // Move to next server if IP is invalid
        return;
    }

    // Clean up previous socket before creating a new one
    CleanupSocket();

    TSharedPtr<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    RemoteAddr->SetIp(EdgegapAddress.Value);
    RemoteAddr->SetPort(ServerInfo.Port);

    if(!InitializeSocket(ServerInfo.Port))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize socket for server: %s:%d"), *ServerInfo.IPAddress, ServerInfo.Port);
        MoveToNextServer();  // Move to next server if socket initialization fails
        return;
    }

    // Record the start time
    StartTime = FPlatformTime::Seconds();

    // Send the ping message
    FString PingMessage = TEXT("Ping");
    int32 BytesSent = 0;
    UDPSocket->SendTo((uint8*)TCHAR_TO_UTF8(*PingMessage), PingMessage.Len(), BytesSent, *RemoteAddr);
    UE_LOG(LogTemp, Log, TEXT("Sent ping to server: %s:%d"), *ServerInfo.IPAddress, ServerInfo.Port);

    // Set up a timer to check for the pong response
    GetWorld()->GetTimerManager().SetTimer(ResponseTimeoutHandle, this, &UEdgegapLatencySubsystem::ReceivePong, 0.1f, true);

    // Set a timeout in case no response is received
    GetWorld()->GetTimerManager().SetTimer(ResponseTimeoutHandle, [this]()
    {
        UE_LOG(LogTemp, Warning, TEXT("Ping to server timed out."));
        MoveToNextServer();
    }, 5.0f, false);  // Set the timeout to 5 seconds (adjust as necessary)
}

void UEdgegapLatencySubsystem::ReceivePong()
{
    uint32 BufferSize = 1024;
    uint8 Buffer[1024];

    int32 BytesRead = 0;
    TSharedRef<FInternetAddr> Sender = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    if (UDPSocket->RecvFrom(Buffer, BufferSize, BytesRead, *Sender))
    {
        FString Response = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Buffer)));
        if (Response.Contains(TEXT("Ping")))
        {
            // Record the end time and calculate latency
            double EndTime = FPlatformTime::Seconds();
            float Latency = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds

            UE_LOG(LogTemp, Log, TEXT("Received pong from server. Latency: %.2f ms"), Latency);

            // Store the latency in the corresponding server info
            ServersToTest[CurrentServerIndex].Latency = Latency;

            // Clear the timeout timer when a response is received
            GetWorld()->GetTimerManager().ClearTimer(ResponseTimeoutHandle);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No response yet. Retrying..."));
        return; // Keep waiting for a pong
    }

    // Move to the next server after pong is received
    MoveToNextServer();
}

void UEdgegapLatencySubsystem::MoveToNextServer()
{
    CurrentServerIndex++;
    if (CurrentServerIndex < ServersToTest.Num())
    {
        // Move to the next server and send ping
        SendPingToServer(ServersToTest[CurrentServerIndex]);
    }
    else
    {
        // All servers have been pinged, sort the array by latency
        ServersToTest.Sort([](const FIPPortInfo& A, const FIPPortInfo& B) {
            return A.Latency < B.Latency; // Sort by latency (ascending)
        });

        // Call a Blueprint event to return the sorted array
        OnLatencyTestCompletedDelegate.ExecuteIfBound(ServersToTest);

        CleanupSocket();
    }
}

void UEdgegapLatencySubsystem::Deinitialize()
{
    CleanupSocket();
    Super::Deinitialize();
}

void UEdgegapLatencySubsystem::CleanupSocket()
{
    if (UDPSocket)
    {
        UDPSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(UDPSocket);
        UDPSocket = nullptr;
    }
}
