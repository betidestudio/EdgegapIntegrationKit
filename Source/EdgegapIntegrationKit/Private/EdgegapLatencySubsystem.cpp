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
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ResponseTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(ResponseTimeoutHandle);
    }
    CleanupSocket();

    OnLatencyTestCompletedDelegate = OnLatencyTestCompleted;
    // Copy the server list and initialize the index
    ServersToTest = ServerList;
    for (FIPPortInfo& Server : ServersToTest)
    {
        Server.Latency = -1.0f;
        Server.bSucceeded = false;
    }
    CurrentServerIndex = -1;

    if (ServersToTest.Num() > 0)
    {
        // Start by sending a ping to the first server
        MoveToNextServer();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No servers provided for latency test."));
        OnLatencyTestCompletedDelegate.ExecuteIfBound(ServersToTest);
    }
}

bool UEdgegapLatencySubsystem::InitializeSocket()
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

    ExpectedRemoteIp = EdgegapAddress.Value;
    ExpectedRemotePort = ServerInfo.Port;

    if(!InitializeSocket())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize socket for server: %s:%d"), *ServerInfo.IPAddress, ServerInfo.Port);
        MoveToNextServer();  // Move to next server if socket initialization fails
        return;
    }

    // Record the start time
    StartTime = FPlatformTime::Seconds();

    // Send the ping message
    const ANSICHAR* PingMessage = "Ping";
    int32 BytesSent = 0;
    const bool bSent = UDPSocket->SendTo(
        reinterpret_cast<const uint8*>(PingMessage),
        4,
        BytesSent,
        *RemoteAddr);
    if (!bSent)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send ping to server: %s:%d"), *ServerInfo.IPAddress, ServerInfo.Port);
        MoveToNextServer();  // Move to next server if ping fails
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("Sent ping to server: %s:%d"), *ServerInfo.IPAddress, ServerInfo.Port);

    // Set up a timer to check for the pong response
    GetWorld()->GetTimerManager().SetTimer(ResponseTimerHandle, this, &UEdgegapLatencySubsystem::ReceivePong, 0.01f, true);

    // Set a timeout in case no response is received
    GetWorld()->GetTimerManager().SetTimer(ResponseTimeoutHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        GetWorld()->GetTimerManager().ClearTimer(ResponseTimerHandle);
        ServersToTest[CurrentServerIndex].Latency = -1.0f;
        ServersToTest[CurrentServerIndex].bSucceeded = false;
        UE_LOG(LogTemp, Warning, TEXT("Ping to beacon %s:%d timed out."),
            *ServersToTest[CurrentServerIndex].IPAddress,
            ServersToTest[CurrentServerIndex].Port);
        CleanupSocket();
        MoveToNextServer();
    }), 2.0f, false);
}

void UEdgegapLatencySubsystem::ReceivePong()
{
    uint8 Buffer[1024];

    int32 BytesRead = 0;
    TSharedRef<FInternetAddr> Sender = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    if (!UDPSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("UDPSocket is null!"));
        return;
    }
    if (UDPSocket->RecvFrom(Buffer, sizeof(Buffer), BytesRead, *Sender))
    {
        uint32 SenderIp = 0;
        Sender->GetIp(SenderIp);
        const bool bExpectedSender = SenderIp == ExpectedRemoteIp && Sender->GetPort() == ExpectedRemotePort;
        const bool bExpectedPayload = BytesRead == 4 && FMemory::Memcmp(Buffer, "Ping", 4) == 0;
        if (bExpectedSender && bExpectedPayload)
        {
            // Record the end time and calculate latency
            double EndTime = FPlatformTime::Seconds();
            float Latency = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds

            UE_LOG(LogTemp, Log, TEXT("Received beacon pong from %s:%d. Latency: %.2f ms"),
                *ServersToTest[CurrentServerIndex].IPAddress,
                ServersToTest[CurrentServerIndex].Port,
                Latency);

            // Store the latency in the corresponding server info
            ServersToTest[CurrentServerIndex].Latency = Latency;
            ServersToTest[CurrentServerIndex].bSucceeded = true;

            // Clear the timeout timer when a response is received
            GetWorld()->GetTimerManager().ClearTimer(ResponseTimeoutHandle);
            GetWorld()->GetTimerManager().ClearTimer(ResponseTimerHandle);
        }
        else
        {
            UE_LOG(LogTemp, Verbose, TEXT("Ignored an unexpected UDP packet while measuring beacon latency."));
            return;
        }
    }
    else
    {
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
            if (A.bSucceeded != B.bSucceeded)
            {
                return A.bSucceeded;
            }
            return A.Latency < B.Latency;
        });

        // Call a Blueprint event to return the sorted array
        OnLatencyTestCompletedDelegate.ExecuteIfBound(ServersToTest);

        CleanupSocket();
    }
}

void UEdgegapLatencySubsystem::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ResponseTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(ResponseTimeoutHandle);
    }
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
