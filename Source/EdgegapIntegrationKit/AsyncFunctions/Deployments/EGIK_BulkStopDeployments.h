// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_BulkStopDeployments.generated.h"

/**
 * Result for a single deployment in bulk stop operation
 */
USTRUCT(BlueprintType)
struct FEGIK_BulkStopResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Message;
};

/**
 * Response from bulk stop deployments
 */
USTRUCT(BlueprintType)
struct FEGIK_BulkStopDeploymentsResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_BulkStopResult> Results;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 SuccessCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 FailureCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulkStopDeploymentsResponse, const FEGIK_BulkStopDeploymentsResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Stops multiple deployments in a single request.
 * POST /v1/deployments/bulk-stop
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_BulkStopDeployments : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Stop multiple deployments at once
	 * @param RequestIds Array of deployment request IDs to stop
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_BulkStopDeployments* BulkStopDeployments(const TArray<FString>& RequestIds);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FBulkStopDeploymentsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FBulkStopDeploymentsResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	TArray<FString> Var_RequestIds;
};
