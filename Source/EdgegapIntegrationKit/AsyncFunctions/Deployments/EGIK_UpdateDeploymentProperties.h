// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_UpdateDeploymentProperties.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_UpdateDeploymentPropertiesRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bIsJoinableBySession = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpdateDeploymentPropertiesResponse, bool, bIsJoinableBySession, FEGIK_ErrorStruct, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_UpdateDeploymentProperties : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Deployment")
	static UEGIK_UpdateDeploymentProperties* UpdateDeploymentProperties(FEGIK_UpdateDeploymentPropertiesRequest Request);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FUpdateDeploymentPropertiesResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FUpdateDeploymentPropertiesResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Deployment"); }

private:
	FEGIK_UpdateDeploymentPropertiesRequest Var_Request;
};
