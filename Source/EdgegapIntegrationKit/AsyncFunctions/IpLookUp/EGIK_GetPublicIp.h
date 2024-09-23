// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_GetPublicIp.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetPublicIpResponse, FString, IpAddress, FEGIK_ErrorStruct, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetPublicIp : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	//Retrieve your public IP address.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | IpLookUp")
	static UEGIK_GetPublicIp* GetIpInformation();

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | IpLookUp")
	FGetPublicIpResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | IpLookUp")
	FGetPublicIpResponse OnFailure;
};
