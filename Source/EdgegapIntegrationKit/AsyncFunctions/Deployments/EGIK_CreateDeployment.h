// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	//The request id of your deployment. If specified, the session will link to the deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestID;

	//Location
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_LatitudeLongitudeStruct Location;

	//If you want your session in a specific city
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString City;

	//If you want your session in a specific country
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Country;
	
	//If you want your session in a specific continent
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Continent;

	//If you want your session in a specific region
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Region;

	//List of Selectors to filter potential Deployment to link and tag the Session
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_SelectorStruct> Selectors;

	//When your Session is Linked, Unprocessable or in Error, we will POST the session's details on the webhook_url
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString WebhookURL;

	//List of location filters to apply to the session
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_FiltersStruct> Filters;

	//If system should skip the telemetry and use GeoBase decision only
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool SkipTelemetry;
};

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateDeployment : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_CreateDeployment* CreateDeployment(const FEGIK_CreateDeploymentStruct& DeploymentStruct);
};
