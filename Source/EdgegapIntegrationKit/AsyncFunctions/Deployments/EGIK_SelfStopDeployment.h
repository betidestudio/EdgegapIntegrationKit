// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_SelfStopDeployment.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSelfStopDeploymentResponse, bool, bSuccess, const FEGIK_ErrorStruct&, Error);

/**
 * Self-terminates a deployment from within the running container.
 * This endpoint is designed to be called by the game server itself using
 * Edgegap's injected ARBITRIUM_DELETE_URL and ARBITRIUM_DELETE_TOKEN values.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SelfStopDeployment : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

	/**
	 * Self-stop a deployment from within the container
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_SelfStopDeployment* SelfStopDeployment();

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FSelfStopDeploymentResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FSelfStopDeploymentResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }
};
