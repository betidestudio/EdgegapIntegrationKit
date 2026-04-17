// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_CreateMatchmakingTicket.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateMatchmakingTicketResponse, const FEGIK_MatchmakingResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Creates a new matchmaking ticket for a player.
 * POST {MatchmakingURL}/tickets
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateMatchmakingTicket : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", DeprecatedFunction, DeprecationMessage = "Tickets are deprecated for new client matchmaking flows. Use Group Up endpoints under Edgegap Integration Kit | Group Up."), Category = "Edgegap Integration Kit | Matchmaking")
	static UEGIK_CreateMatchmakingTicket* CreateMatchmakingTicket(const FEGIK_CreateMatchmakingStruct& MatchmakingStruct);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FCreateMatchmakingTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FCreateMatchmakingTicketResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FCreateMatchmakingTicketResponse OnRateLimited;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual TMap<FString, FString> GetAdditionalHeaders() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual void HandleRateLimited(const FString& ResponseContent) override;
	virtual FString GetLogCategory() const override { return TEXT("Matchmaking"); }

private:
	FEGIK_CreateMatchmakingStruct Var_MatchmakingStruct;
};
