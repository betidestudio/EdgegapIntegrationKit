// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
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
class EDGEGAPINTEGRATIONKIT_API UEGIK_ListLocationBeacons : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Locations")
	static UEGIK_ListLocationBeacons* ListLocationBeacons();

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Locations")
	FListLocationBeaconsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Locations")
	FListLocationBeaconsResponse OnFailure;
	
};
