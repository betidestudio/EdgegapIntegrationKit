// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_AuthorizeUserOnRelaySession.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_AuthorizeUserOnRelaySessionInput
{
	GENERATED_BODY()

	//Session Request ID (ex: 0724828881e2-S)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	FString SessionId;

	//User Ip to Authorize
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Edgegap Integration Kit")
	FString UserIp;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthorizeUserOnRelaySession, FEGIK_RelaySessionInfo, RelaySessionInfo, FEGIK_ErrorStruct, Error);
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_AuthorizeUserOnRelaySession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Relay Sessions", meta = (BlueprintInternalUseOnly = "true"))
	static UEGIK_AuthorizeUserOnRelaySession* AuthorizeUserOnRelaySession(FEGIK_AuthorizeUserOnRelaySessionInput Input);

	virtual void Activate() override;
	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Relay Sessions")
	FOnAuthorizeUserOnRelaySession OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Relay Sessions")
	FOnAuthorizeUserOnRelaySession OnFailure;

private:
	FEGIK_AuthorizeUserOnRelaySessionInput Var_Input;
};
