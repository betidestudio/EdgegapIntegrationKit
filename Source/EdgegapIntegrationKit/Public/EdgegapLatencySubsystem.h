// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "EdgegapLatencySubsystem.generated.h"

USTRUCT(BlueprintType)
struct FIPPortInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Latency")
	FString IPAddress;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Latency")
	int32 Port;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Latency")
	FString LocationName;  // New field to hold the location name

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Edgegap Integration Kit | Latency")
	float Latency;  // To store the calculated latency

	FIPPortInfo(): Port(0), Latency(0)
	{
	}

	FIPPortInfo(const FString& InIPAddress, int32 InPort) : IPAddress(InIPAddress), Port(InPort), Latency(0)
	{
	}
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnLatencyTestCompleted, const TArray<FIPPortInfo>&, SortedServers);
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEdgegapLatencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// Function to test latency for multiple IPs and ports, one by one
	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Latency")
	void StartLatencyTestForMultipleServers(const TArray<FIPPortInfo>& ServerList, FOnLatencyTestCompleted OnLatencyTestCompleted);

	FOnLatencyTestCompleted OnLatencyTestCompletedDelegate;

protected:
	virtual void Deinitialize() override;

private:
	FTimerHandle ResponseTimeoutHandle; // Timer handle for managing the pong response timeout
	
	FSocket* UDPSocket;
	double StartTime;

	TArray<FIPPortInfo> ServersToTest;
	int32 CurrentServerIndex;

	bool InitializeSocket(int32 Port);

	void SendPingToServer(const FIPPortInfo& ServerInfo);

	void ReceivePong();

	void CleanupSocket();

	// Move to the next server after finishing with the current one
	void MoveToNextServer();
};