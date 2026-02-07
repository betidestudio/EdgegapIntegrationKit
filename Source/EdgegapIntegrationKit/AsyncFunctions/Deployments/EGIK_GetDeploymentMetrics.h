// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_GetDeploymentMetrics.generated.h"

/**
 * CPU metrics for a deployment
 */
USTRUCT(BlueprintType)
struct FEGIK_CpuMetrics
{
	GENERATED_BODY()

	// CPU usage percentage
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	float UsagePercent = 0.0f;

	// CPU limit in millicores
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 LimitMillicores = 0;

	// CPU usage in millicores
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 UsageMillicores = 0;
};

/**
 * Memory metrics for a deployment
 */
USTRUCT(BlueprintType)
struct FEGIK_MemoryMetrics
{
	GENERATED_BODY()

	// Memory usage percentage
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	float UsagePercent = 0.0f;

	// Memory limit in bytes
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int64 LimitBytes = 0;

	// Memory usage in bytes
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int64 UsageBytes = 0;
};

/**
 * Network metrics for a deployment
 */
USTRUCT(BlueprintType)
struct FEGIK_NetworkMetrics
{
	GENERATED_BODY()

	// Bytes received
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int64 RxBytes = 0;

	// Bytes transmitted
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int64 TxBytes = 0;

	// Packets received
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int64 RxPackets = 0;

	// Packets transmitted
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int64 TxPackets = 0;
};

/**
 * Complete deployment metrics response
 */
USTRUCT(BlueprintType)
struct FEGIK_DeploymentMetricsResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_CpuMetrics Cpu;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_MemoryMetrics Memory;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_NetworkMetrics Network;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetDeploymentMetricsResponse, const FEGIK_DeploymentMetricsResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Retrieves resource metrics for a running deployment.
 * GET /v1/metrics/deployment/{request_id}
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetDeploymentMetrics : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Get metrics for a deployment
	 * @param RequestId The deployment request ID
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_GetDeploymentMetrics* GetDeploymentMetrics(const FString& RequestId);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FGetDeploymentMetricsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FGetDeploymentMetricsResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	FString Var_RequestId;
};
