// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_GetIpInformationBulk.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetIpInformationBulkResponse, TArray<FEGIK_IpLookUpAddress>, IpAddresses, FEGIK_ErrorStruct, Error);
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetIpInformationBulk : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | IpLookUp")
	static UEGIK_GetIpInformationBulk* GetIpInformationBulk(TArray<FString> IpAddresses);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | IpLookUp")
	FGetIpInformationBulkResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | IpLookUp")
	FGetIpInformationBulkResponse OnFailure;
	

private:
	TArray<FString> Var_IpAddresses;
};
