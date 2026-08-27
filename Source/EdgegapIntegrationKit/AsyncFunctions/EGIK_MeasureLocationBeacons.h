// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_GetLocationBeacons.h"
#include "EdgegapLatencySubsystem.h"
#include "EGIK_MeasureLocationBeacons.generated.h"

USTRUCT(BlueprintType)
struct EDGEGAPINTEGRATIONKIT_API FEGIK_MeasureLocationBeaconsResult
{
	GENERATED_BODY()

	/** Successful city-name to round-trip milliseconds measurements. */
	UPROPERTY(BlueprintReadOnly, Category = "Edgegap Integration Kit | Location")
	TMap<FString, double> BeaconLatencies;

	/** Every attempted probe, including failed probes for diagnostics. */
	UPROPERTY(BlueprintReadOnly, Category = "Edgegap Integration Kit | Location")
	TArray<FIPPortInfo> PingResults;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FEGIK_MeasureLocationBeaconsResponse,
	const FEGIK_MeasureLocationBeaconsResult&, Result,
	const FEGIK_ErrorStruct&, Error);

/**
 * Retrieves the current Matchmaker beacon list and measures a real UDP
 * round-trip to each beacon. Failed probes are reported in Ping Results but
 * are omitted from Beacon Latencies so a timeout can never look like 0 ms.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_MeasureLocationBeacons : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Edgegap Integration Kit | Location")
	static UEGIK_MeasureLocationBeacons* MeasureLocationBeacons(
		UObject* WorldContextObject,
		FString MatchmakingUrl = TEXT(""),
		FString AuthToken = TEXT(""));

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Location")
	FEGIK_MeasureLocationBeaconsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Location")
	FEGIK_MeasureLocationBeaconsResponse OnFailure;

	virtual void Activate() override;

private:
	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY()
	TObjectPtr<UEGIK_GetLocationBeacons> ActiveBeaconRequest;

	FString MatchmakingUrl;
	FString AuthToken;
	bool bFinished = false;

	UFUNCTION()
	void HandleBeaconsReceived(const FEGIK_LocationBeaconResponse& Response, const FEGIK_ErrorStruct& Error);

	UFUNCTION()
	void HandleBeaconRequestFailed(const FEGIK_LocationBeaconResponse& Response, const FEGIK_ErrorStruct& Error);

	UFUNCTION()
	void HandleLatencyComplete(const TArray<FIPPortInfo>& PingResults);

	void FinishSuccess(const TMap<FString, double>& BeaconLatencies, const TArray<FIPPortInfo>& PingResults);
	void FinishFailure(const FEGIK_ErrorStruct& Error);
};
