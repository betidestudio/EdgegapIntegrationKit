// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_DeleteDeployment.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_DeleteDeploymentResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Message;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_DeploymentStatusAndInfoResponse DeploymentStatusAndInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDeleteDeploymentDelegate, FEGIK_DeleteDeploymentResponse, Response, FEGIK_ErrorStruct, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeleteDeployment : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, DisplayName="Delete Deployment", meta = (BlueprintInternalUseOnly = "true"), Category="Edgegap Integration Kit || Deployments")
	static UEGIK_DeleteDeployment* DeleteDeployment(FString RequestId, FString ContainerLogStorage = "false");

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FDeleteDeploymentDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FDeleteDeploymentDelegate OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	FString Var_RequestId;
	FString Var_ContainerLogStorage;
};
