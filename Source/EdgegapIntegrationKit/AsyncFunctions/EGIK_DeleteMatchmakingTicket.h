// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_DeleteMatchmakingTicket.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_DeleteMatchmakingRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString TicketId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString MatchmakingURL = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString AuthToken = TEXT("");
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleteMatchmakingTicketResponse, const FEGIK_ErrorStruct&, Error);

/**
 * Deletes an existing matchmaking ticket.
 * DELETE {MatchmakingURL}/tickets/{TicketId}
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeleteMatchmakingTicket : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Matchmaking")
	static UEGIK_DeleteMatchmakingTicket* DeleteMatchmakingTicket(FEGIK_DeleteMatchmakingRequest Request);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FDeleteMatchmakingTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FDeleteMatchmakingTicketResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FDeleteMatchmakingTicketResponse OnRateLimited;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual void HandleRateLimited(const FString& ResponseContent) override;
	virtual FString GetLogCategory() const override { return TEXT("Matchmaking"); }

private:
	FEGIK_DeleteMatchmakingRequest Var_Request;
};
