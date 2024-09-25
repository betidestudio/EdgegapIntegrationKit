// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_ListLocations.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_ListAllLocationsRequest
{
	GENERATED_BODY()

	//The App Name you want to filter with capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString App;

	//The Version Name you want to filter with capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString Version;

	//The type of the location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString Type;

	//Gets locations with tags. Set to: "true" to have the tags
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	bool bTags;
};

USTRUCT(BlueprintType)
struct FEGIK_AvailableLocationStruct
{
	GENERATED_BODY()

	//City Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString City;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString Country;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString Continent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString Timezone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString AdministrativeDivision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	float Latitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	float Longitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	FString Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit | Locations")
	TMap<FString, FString> Tags;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FListLocationsResponse, const TArray<FEGIK_AvailableLocationStruct>&, Locations, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_ListLocations : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	//List all the locations available to deploy on. You can specify an application and a version to filter out the locations that don’t have enough resources to deploy this application version.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Locations")
	static UEGIK_ListLocations* ListLocations(FEGIK_ListAllLocationsRequest ListLocationsRequest);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Locations")
	FListLocationsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Locations")
	FListLocationsResponse OnFailure;

private:
	FEGIK_ListAllLocationsRequest Var_ListLocationsRequest;

	

	
};
