// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_ListAllDeployments.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_DeploymentListData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString FQDN;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString StartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bReady = false;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString PublicIP;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TMap<FString, FEGIK_DeploymentPorts> Ports;
	
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Tags;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Sockets;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 SocketsUsage;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bIsJoinableBySession = false;
};

USTRUCT(BlueprintType)
struct FEGIK_Pagination
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Number = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 NextPageNumber = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 PreviousPageNumber = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 TotalPages = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bHasNextPage = false;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bHasPreviousPage = false;
};

USTRUCT(BlueprintType)
struct FEGIK_ListAllDeploymentsResponse
{
	GENERATED_BODY()

	/** List of active deployments */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_DeploymentListData> Deployments;

	/** Total Objects in the Database */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 TotalCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_Pagination Pagination;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Message;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEGIK_ListAllDeploymentsDelegate, const FEGIK_ListAllDeploymentsResponse&, Response, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_ListAllDeployments : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_ListAllDeployments* ListAllDeployments(const FString& Query);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);

	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_ListAllDeploymentsDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_ListAllDeploymentsDelegate OnFailure;

private:
	FString Var_Query;

	TArray<FString> JsonArrayToStringArray(const TArray<TSharedPtr<FJsonValue>>* JsonArray)
	{
		TArray<FString> StringArray;
		if (JsonArray)
		{
			for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
			{
				if (Value->Type == EJson::String)
				{
					StringArray.Add(Value->AsString());
				}
			}
		}
		return StringArray;
	}

};
