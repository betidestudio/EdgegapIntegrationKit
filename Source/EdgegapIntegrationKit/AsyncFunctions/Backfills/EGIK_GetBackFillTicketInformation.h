// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_GetBackFillTicketInformation.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_GetBackFillTicketInformationResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FString Id;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FString Profile;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	TMap<FString, FEGIK_MatchmakingResponse> Tickets;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FString Status;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FEGIK_MatchmakingResponse AssignedTicket;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FEGIK_AssignmentStruct Assignment;

	/** Raw JSON string of attributes.assignment for flexible handling of variable structures */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FString AssignmentDetailsJson;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetBackFillTicketInformationResponse, const FEGIK_GetBackFillTicketInformationResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Gets information about a backfill request.
 * GET {MatchmakingURL}/backfills/{BackfillId}
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetBackFillTicketInformation : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Backfill")
	static UEGIK_GetBackFillTicketInformation* GetBackFillTicketInformation(const FString& backfillId, const FString& MatchmakingURL, const FString& AuthToken);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FGetBackFillTicketInformationResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FGetBackFillTicketInformationResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FGetBackFillTicketInformationResponse OnRateLimited;

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
