// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
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
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeleteDeployment : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, DisplayName="Delete Deployment",meta = (BlueprintInternalUseOnly = "true"), Category="Edgegap Integration Kit || Deployments")
	static UEGIK_DeleteDeployment* DeleteDeployment(FString bRequestId, FString ContainerLogStorage="false");

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FDeleteDeploymentDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Deployment")
	FDeleteDeploymentDelegate OnFailure;
	
private:
	FString Var_ContainerLogStorage;
	FString Var_bRequestId;
};
