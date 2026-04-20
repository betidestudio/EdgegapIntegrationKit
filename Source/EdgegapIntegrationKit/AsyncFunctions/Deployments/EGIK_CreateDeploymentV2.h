// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_CreateDeploymentV2.generated.h"

/**
 * Type of user data for v2 deployment location matching
 */
UENUM(BlueprintType)
enum class EEGIK_DeploymentUserType : uint8
{
	IpAddress UMETA(DisplayName = "IP Address"),
	GeoCoordinates UMETA(DisplayName = "Geo Coordinates")
};

/**
 * Represents a user for v2 deployment API.
 * Each user specifies either an IP address or geo coordinates for location matching.
 */
USTRUCT(BlueprintType)
struct FEGIK_DeploymentUser
{
	GENERATED_BODY()

	// The type of user data: IP address or geo coordinates
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	EEGIK_DeploymentUserType UserType = EEGIK_DeploymentUserType::IpAddress;

	// The IP address (used when UserType is IpAddress)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment", meta = (EditCondition = "UserType == EEGIK_DeploymentUserType::IpAddress"))
	FString IpAddress;

	// Latitude (used when UserType is GeoCoordinates)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment", meta = (EditCondition = "UserType == EEGIK_DeploymentUserType::GeoCoordinates"))
	double Latitude = 0.0;

	// Longitude (used when UserType is GeoCoordinates)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment", meta = (EditCondition = "UserType == EEGIK_DeploymentUserType::GeoCoordinates"))
	double Longitude = 0.0;

	TSharedPtr<FJsonObject> ToJsonObject() const
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		TSharedPtr<FJsonObject> UserData = MakeShareable(new FJsonObject);

		if (UserType == EEGIK_DeploymentUserType::IpAddress)
		{
			JsonObject->SetStringField(TEXT("user_type"), TEXT("ip_address"));
			UserData->SetStringField(TEXT("ip_address"), IpAddress);
		}
		else
		{
			JsonObject->SetStringField(TEXT("user_type"), TEXT("geo_coordinates"));
			UserData->SetNumberField(TEXT("latitude"), Latitude);
			UserData->SetNumberField(TEXT("longitude"), Longitude);
		}

		JsonObject->SetObjectField(TEXT("user_data"), UserData);
		return JsonObject;
	}
};

/**
 * Webhook configuration for v2 deployment events
 */
USTRUCT(BlueprintType)
struct FEGIK_DeploymentWebhook
{
	GENERATED_BODY()

	// The webhook URL to call when the event occurs (POST)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString URL;
};

/**
 * Input parameters for v2 deployment API (POST /v2/deployments)
 */
USTRUCT(BlueprintType)
struct FEGIK_CreateDeploymentV2Struct
{
	GENERATED_BODY()

	// The name of the application to deploy (required)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Application;

	// The version of the application to deploy (required)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Version;

	// The list of users for location matching (required, at least 1)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_DeploymentUser> Users;

	// If true, only deploy to locations already cached (speeds up deployment)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bRequireCachedLocations = false;

	// Environment variables to set on the deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_EnvironmentVariableStruct> EnvironmentVariables;

	// Tags for the deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Tags;

	// Webhook called when deployment becomes ready
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_DeploymentWebhook WebhookOnReady;

	// Webhook called when deployment encounters an error
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_DeploymentWebhook WebhookOnError;

	// Webhook called when deployment is terminated
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_DeploymentWebhook WebhookOnTerminated;
};

/**
 * Response from v2 deployment API (202 Accepted)
 */
USTRUCT(BlueprintType)
struct FEGIK_CreateDeploymentV2Response
{
	GENERATED_BODY()

	// The unique request ID for the deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateDeploymentV2Response, const FEGIK_CreateDeploymentV2Response&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Creates a new deployment using the v2 API (POST /v2/deployments).
 * The v2 API uses structured 'users' with user_type/user_data and separate webhook events.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateDeploymentV2 : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Create a deployment using the v2 API
	 * @param DeploymentStruct The deployment configuration
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_CreateDeploymentV2* CreateDeploymentV2(FEGIK_CreateDeploymentV2Struct DeploymentStruct);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FCreateDeploymentV2Response OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FCreateDeploymentV2Response OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	FEGIK_CreateDeploymentV2Struct Var_DeploymentStruct;
};
