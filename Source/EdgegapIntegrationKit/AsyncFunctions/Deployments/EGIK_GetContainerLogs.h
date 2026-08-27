// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_GetContainerLogs.generated.h"

/**
 * Container log entry
 */
USTRUCT(BlueprintType)
struct FEGIK_ContainerLogEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Message;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Level;
};

/**
 * Response containing container logs
 */
USTRUCT(BlueprintType)
struct FEGIK_ContainerLogsResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_ContainerLogEntry> Logs;

	// Raw logs as a single string (if API returns it this way)
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RawLogs;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Encoding;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetContainerLogsResponse, const FEGIK_ContainerLogsResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Retrieves container logs for a deployment.
 * GET /v1/deployment/{request_id}/container-logs
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetContainerLogs : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Get container logs for a deployment
	 * @param RequestId The deployment request ID
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_GetContainerLogs* GetContainerLogs(const FString& RequestId);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FGetContainerLogsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FGetContainerLogsResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	FString Var_RequestId;
};
