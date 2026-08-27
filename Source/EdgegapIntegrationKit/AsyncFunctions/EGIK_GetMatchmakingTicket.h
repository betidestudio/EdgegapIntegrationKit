// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_GetMatchmakingTicket.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_MatchmakingRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString TicketId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString MatchmakingURL = TEXT("");

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString AuthToken = TEXT("");
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetMatchmakingTicketResponse, const FEGIK_MatchmakingResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Gets the status of an existing matchmaking ticket.
 * GET {MatchmakingURL}/tickets/{TicketId}
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetMatchmakingTicket : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", DeprecatedFunction, DeprecationMessage = "Tickets are deprecated for new client matchmaking flows. Use Group Up endpoints under Edgegap Integration Kit | Group Up."), Category = "Edgegap Integration Kit | Matchmaking")
	static UEGIK_GetMatchmakingTicket* GetMatchmakingTicket(FEGIK_MatchmakingRequest MatchmakingRequest);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FGetMatchmakingTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FGetMatchmakingTicketResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FGetMatchmakingTicketResponse OnRateLimited;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual void HandleRateLimited(const FString& ResponseContent) override;
	virtual FString GetLogCategory() const override { return TEXT("Matchmaking"); }

private:
	FEGIK_MatchmakingRequest Var_MatchmakingRequest;
};
