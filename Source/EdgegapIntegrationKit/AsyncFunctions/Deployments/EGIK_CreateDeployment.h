// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_CreateDeployment.generated.h"


USTRUCT(BlueprintType)
struct FEGIK_CreateDeploymentStruct
{
	GENERATED_BODY()

	//The name of the App you want to deploy
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString AppName;

	//The name of the App Version you want to deploy, if not present, the last version created is picked
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString VersionName;

	//If the Application is public or private. If not specified, we will look for a private Application
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool IsPublicApp;

	//The List of IP of your user - Will be used to find the best location to deploy your App
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> UserIPs;

	//The list of IP of your user with their location (latitude, longitude)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_GeoIpStruct> UserGeoIPs;

	//Array of strings
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> telemetry_profile_uuid_list;

	//A list of deployment variables
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_EnvironmentVariableStruct> DeploymentVariables;

	//If you want to skip the Telemetry and use a geolocations decision only
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bSkipTelemetry;

	//Location
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_LatitudeLongitudeStruct Location;

	//A web URL. This url will be called with method POST. The deployment status will be send in JSON format
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString WebHookURL;

	//The list of tags for your deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Tags;

	//Container Log Storage
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_ContainerLogStorageStruct ContainerLogStorage;

	//Filters to use while choosing the deployment location.
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_FiltersStruct> Filters;

	//Algorithm used to select the edge location
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TEnumAsByte<EEGIK_ApSortStrategy> SortStrategy;

	//Allows to override the Container command for this deployment.
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString ContainerCommand;

	//Allows to override the Container arguments for this deployment.
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString ContainerArguments;
};

USTRUCT(BlueprintType)
struct FEGIK_CreateDeploymentResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestDns;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestApp;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestVersion;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 RequestUserCount = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString ApSortStrategy = "basic";
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateDeploymentResponse, const FEGIK_CreateDeploymentResponse&, Response, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateDeployment : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_CreateDeployment* CreateDeployment(FEGIK_CreateDeploymentStruct DeploymentStruct);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FCreateDeploymentResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FCreateDeploymentResponse OnFailure;

private:
	FEGIK_CreateDeploymentStruct Var_DeploymentStruct;
};
