// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_DeleteBackFillTicket.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleteBackFillTicketResponse, const FEGIK_ErrorStruct&, Error);

/**
 * Deletes a backfill request.
 * DELETE {MatchmakingURL}/backfills/{BackfillId}
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeleteBackFillTicket : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Backfill")
	static UEGIK_DeleteBackFillTicket* DeleteBackFillTicket(const FString& backfillId, const FString& MatchmakingURL = TEXT(""), const FString& AuthToken = TEXT(""));

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FDeleteBackFillTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FDeleteBackFillTicketResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FDeleteBackFillTicketResponse OnRateLimited;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual void HandleRateLimited(const FString& ResponseContent) override;
	virtual FString GetLogCategory() const override { return TEXT("Backfill"); }

private:
	FString Var_BackfillId;
	FString Var_MatchmakingURL;
	FString Var_AuthToken;
};
