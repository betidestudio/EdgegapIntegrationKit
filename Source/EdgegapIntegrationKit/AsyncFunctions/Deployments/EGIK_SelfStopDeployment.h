// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_SelfStopDeployment.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSelfStopDeploymentResponse, bool, bSuccess, const FEGIK_ErrorStruct&, Error);

/**
 * Self-terminates a deployment from within the running container.
 * This endpoint is designed to be called by the game server itself.
 * DELETE /v1/self/stop/{request_id}/{access_point_id}
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SelfStopDeployment : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Self-stop a deployment from within the container
	 * @param RequestId The deployment request ID
	 * @param AccessPointId The access point ID of the deployment
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_SelfStopDeployment* SelfStopDeployment(const FString& RequestId, const FString& AccessPointId);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FSelfStopDeploymentResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FSelfStopDeploymentResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	FString Var_RequestId;
	FString Var_AccessPointId;
};
