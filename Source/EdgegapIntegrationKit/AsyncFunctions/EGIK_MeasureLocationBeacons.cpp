// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_MeasureLocationBeacons.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

UEGIK_MeasureLocationBeacons* UEGIK_MeasureLocationBeacons::MeasureLocationBeacons(
	UObject* InWorldContextObject,
	FString InMatchmakingUrl,
	FString InAuthToken)
{
	UEGIK_MeasureLocationBeacons* Node = NewObject<UEGIK_MeasureLocationBeacons>();
	Node->WorldContextObject = InWorldContextObject;
	Node->MatchmakingUrl = MoveTemp(InMatchmakingUrl);
	Node->AuthToken = MoveTemp(InAuthToken);
	Node->RegisterWithGameInstance(InWorldContextObject);
	return Node;
}

void UEGIK_MeasureLocationBeacons::Activate()
{
	Super::Activate();

	if (!IsValid(WorldContextObject))
	{
		FinishFailure(FEGIK_ErrorStruct(0, TEXT("Measure Location Beacons requires a valid world context.")));
		return;
	}

	ActiveBeaconRequest = UEGIK_GetLocationBeacons::GetLocationBeacons(MatchmakingUrl, AuthToken);
	if (!IsValid(ActiveBeaconRequest))
	{
		FinishFailure(FEGIK_ErrorStruct(0, TEXT("Failed to create the location-beacons request.")));
		return;
	}

	ActiveBeaconRequest->OnSuccess.AddDynamic(this, &UEGIK_MeasureLocationBeacons::HandleBeaconsReceived);
	ActiveBeaconRequest->OnFailure.AddDynamic(this, &UEGIK_MeasureLocationBeacons::HandleBeaconRequestFailed);
	ActiveBeaconRequest->Activate();
}

void UEGIK_MeasureLocationBeacons::HandleBeaconsReceived(
	const FEGIK_LocationBeaconResponse& Response,
	const FEGIK_ErrorStruct& Error)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UEdgegapLatencySubsystem* LatencySubsystem = GameInstance ? GameInstance->GetSubsystem<UEdgegapLatencySubsystem>() : nullptr;
	if (!LatencySubsystem)
	{
		FinishFailure(FEGIK_ErrorStruct(0, TEXT("The Edgegap Latency Subsystem is unavailable.")));
		return;
	}

	TArray<FIPPortInfo> Targets;
	Targets.Reserve(Response.Beacons.Num());
	for (const FEGIK_LocationBeaconStruct& Beacon : Response.Beacons)
	{
		if (Beacon.PublicIP.IsEmpty() || Beacon.UDP_Port <= 0)
		{
			continue;
		}

		FIPPortInfo& Target = Targets.Emplace_GetRef(Beacon.PublicIP, Beacon.UDP_Port);
		Target.LocationName = Beacon.Location.City.IsEmpty() ? Beacon.FQDN : Beacon.Location.City;
	}

	if (Targets.IsEmpty())
	{
		FinishFailure(FEGIK_ErrorStruct(0, TEXT("The Matchmaker returned no usable UDP location beacons.")));
		return;
	}

	FOnLatencyTestCompleted Completion;
	Completion.BindDynamic(this, &UEGIK_MeasureLocationBeacons::HandleLatencyComplete);
	LatencySubsystem->StartLatencyTestForMultipleServers(Targets, Completion);
}

void UEGIK_MeasureLocationBeacons::HandleBeaconRequestFailed(
	const FEGIK_LocationBeaconResponse& Response,
	const FEGIK_ErrorStruct& Error)
{
	FinishFailure(Error);
}

void UEGIK_MeasureLocationBeacons::HandleLatencyComplete(const TArray<FIPPortInfo>& PingResults)
{
	TMap<FString, double> BeaconLatencies;
	for (const FIPPortInfo& Result : PingResults)
	{
		if (Result.bSucceeded && !Result.LocationName.IsEmpty() && Result.Latency >= 0.0f)
		{
			BeaconLatencies.Add(Result.LocationName, static_cast<double>(Result.Latency));
		}
	}

	if (BeaconLatencies.IsEmpty())
	{
		FinishFailure(FEGIK_ErrorStruct(0, TEXT("No location beacon returned a valid UDP echo response.")));
		return;
	}

	FinishSuccess(BeaconLatencies, PingResults);
}

void UEGIK_MeasureLocationBeacons::FinishSuccess(
	const TMap<FString, double>& BeaconLatencies,
	const TArray<FIPPortInfo>& PingResults)
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;
	FEGIK_MeasureLocationBeaconsResult Result;
	Result.BeaconLatencies = BeaconLatencies;
	Result.PingResults = PingResults;
	OnSuccess.Broadcast(Result, FEGIK_ErrorStruct());
	SetReadyToDestroy();
}

void UEGIK_MeasureLocationBeacons::FinishFailure(const FEGIK_ErrorStruct& Error)
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;
	OnFailure.Broadcast(FEGIK_MeasureLocationBeaconsResult(), Error);
	SetReadyToDestroy();
}
