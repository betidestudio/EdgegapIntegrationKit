// Copyright (c) Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_DeleteBackFillTicket.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleteBackFillTicketResponse, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeleteBackFillTicket : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Backfill")
	static UEGIK_DeleteBackFillTicket* DeleteBackFillTicket(const FString& backfillId, const FString& MatchmakingURL, const FString& AuthToken);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FDeleteBackFillTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FDeleteBackFillTicketResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FDeleteBackFillTicketResponse OnRateLimited;

private:
	FString Var_BackfillId;
	FString Var_MatchmakingURL;
	FString Var_AuthToken;
	
};
