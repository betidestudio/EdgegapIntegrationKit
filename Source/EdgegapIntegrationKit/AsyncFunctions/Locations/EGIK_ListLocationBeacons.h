// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ListLocationBeacons.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_ListLocationBeaconsResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString Host;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString FQDN;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	int32 UdpPort;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	int32 TcpPort;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FEGIK_LocationStruct Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FEGIK_LatitudeLongitudeStruct LatitudeLongitude;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FListLocationBeaconsResponse, const TArray<FEGIK_ListLocationBeaconsResponse>&, Response, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_ListLocationBeacons : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Locations")
	static UEGIK_ListLocationBeacons* ListLocationBeacons();

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Locations")
	FListLocationBeaconsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Locations")
	FListLocationBeaconsResponse OnFailure;

protected:
	//~ Begin UEGIK_AsyncRequestBase Interface
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Locations"); }
	//~ End UEGIK_AsyncRequestBase Interface
};
