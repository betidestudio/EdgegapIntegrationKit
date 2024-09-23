// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_CreateRelaySession.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_CreateRelaySessionInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	TArray<FString> Users;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	TArray<FEGIK_FiltersStruct> Filters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	FString WebhookUrl;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreateRelaySession, FEGIK_RelaySessionInfo, RelaySessionInfo, FEGIK_ErrorStruct, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateRelaySession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Relay Sessions", meta = (BlueprintInternalUseOnly = "true"))
	static UEGIK_CreateRelaySession* CreateRelaySession(FEGIK_CreateRelaySessionInput Input);

	virtual void Activate() override;
	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Relay Sessions")
	FOnCreateRelaySession OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Relay Sessions")
	FOnCreateRelaySession OnFailure;

private:
	FEGIK_CreateRelaySessionInput Var_Input;
};
