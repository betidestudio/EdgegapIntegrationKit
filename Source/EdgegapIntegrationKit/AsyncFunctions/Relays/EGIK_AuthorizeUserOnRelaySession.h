// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
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
class EDGEGAPINTEGRATIONKIT_API UEGIK_AuthorizeUserOnRelaySession : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Relay Sessions", meta = (BlueprintInternalUseOnly = "true"))
	static UEGIK_AuthorizeUserOnRelaySession* AuthorizeUserOnRelaySession(FEGIK_AuthorizeUserOnRelaySessionInput Input);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Relay Sessions")
	FOnAuthorizeUserOnRelaySession OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Relay Sessions")
	FOnAuthorizeUserOnRelaySession OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Relays"); }

private:
	FEGIK_AuthorizeUserOnRelaySessionInput Var_Input;
};
