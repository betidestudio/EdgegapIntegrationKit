// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_DeploymentStatusAndInfo.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDeploymentStatusAndInfoResponse, const FEGIK_DeploymentStatusAndInfoResponse&, Response, FEGIK_ErrorStruct, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeploymentStatusAndInfo : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_DeploymentStatusAndInfo* GetDeploymentStatusAndInfo(const FString& RequestId);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FDeploymentStatusAndInfoResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FDeploymentStatusAndInfoResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	FString Var_RequestId;
};
