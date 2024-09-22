// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_DeploymentStatusAndInfo.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_Session
{
	GENERATED_BODY()

	//Unique UUID
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString SessionId;

	//Current status of the session
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Status;

	//If the session is linked to a Ready deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bReady;

	//If the session is linked to a deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bLinked;

	//Type of session created
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Kind;

	//Count of user this session currently have
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 UserCount;
	
};

USTRUCT(BlueprintType)
struct FEGIK_DeploymentPorts
{
	GENERATED_BODY()

	//The Port to Connect from Internet
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 External;

	//The internal Port of the Container
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Internal;
	
	//The Protocol (i.e. 'TCP')
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Protocol;

	//If the port require TLS Upgrade
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bTLS_Upgrade;
	
	//link of the port with scheme depending of the protocol
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Link;
	
	//Internal Proxy Mapping
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Proxy;


};

USTRUCT(BlueprintType)
struct FEGIK_DeploymentLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_LocationStruct Location;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_LatitudeLongitudeStruct LatitudeLongitude;
};

USTRUCT(BlueprintType)
struct FEGIK_DeploymentStatusAndInfoResponse
{
	GENERATED_BODY()

	//The Unique ID of the Deployment's request
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	//The FQDN that allow to connect to your Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString FQDN;

	//The name of the deployed App
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString AppName;

	//The version of the deployed App
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString VersionName;

	//The current status of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString CurrentStatus;

	//True if the current Deployment is ready to be connected and running
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bRunning;

	//True if the current Deployment is ACL protected
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bWhitelistingActive;

	//Timestamp of the Deployment when it is up and running
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FDateTime StartTime;

	//Timestamp of the end of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FDateTime RemovalTime;

	//Time since the Deployment is up and running in seconds
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 ElapsedTime;

	//The last status of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString LastStatus;

	//True if there is an error with the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bError;

	//The error details of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString ErrorDetails;

	//Ports
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TMap<FString, FEGIK_DeploymentPorts> Ports;

	//The public IP
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString PublicIP;

	//List of Active Sessions if Deployment App is Session Based
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_Session> ActiveSessions;

	//Location
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_DeploymentLocation Location;

	//List of tags associated with the deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Tags;

	//The Capacity of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Sockets;

	//The Capacity Usage of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 SocketsUsage;

	//The command to use in the container, null mean it will take the default of the container
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Command;

	//The arguments to use in the container, null mean it will take the default of the container
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Arguments;
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDeploymentStatusAndInfoResponse, const FEGIK_DeploymentStatusAndInfoResponse&, Response, FEGIK_ErrorStruct, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeploymentStatusAndInfo : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_DeploymentStatusAndInfo* GetDeploymentStatusAndInfo(const FString& RequestId);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FDeploymentStatusAndInfoResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FDeploymentStatusAndInfoResponse OnFailure;
	
private:
	FString Var_RequestId;

};
