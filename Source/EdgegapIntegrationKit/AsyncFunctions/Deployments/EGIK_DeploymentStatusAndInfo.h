// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_DeploymentStatusAndInfo.generated.h"

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
