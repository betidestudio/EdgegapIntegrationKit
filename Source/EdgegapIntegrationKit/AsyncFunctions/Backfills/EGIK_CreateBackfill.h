// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_CreateBackfill.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_CreateBackFillRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FString Profile;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FString DeploymentRequestID;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	TMap<FString, FString> Tickets;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FString AttributesOverride;

	/** Full assignment details in JSON format, required by new API */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Backfill")
	FString AssignmentDetails;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString MatchmakingURL;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString AuthToken;
};

USTRUCT(BlueprintType)
struct FEGIK_CreateBackFillResponse
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateBackfillResponse, const FEGIK_CreateBackFillResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Creates a backfill request to find replacement players.
 * POST {MatchmakingURL}/backfills
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateBackfill : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Backfill", meta = (BlueprintInternalUseOnly = "true"))
	static UEGIK_CreateBackfill* CreateBackFillRequest(const FEGIK_CreateBackFillRequest& Request);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FCreateBackfillResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FCreateBackfillResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FCreateBackfillResponse OnRateLimited;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual void HandleRateLimited(const FString& ResponseContent) override;
	virtual FString GetLogCategory() const override { return TEXT("Backfill"); }

private:
	FEGIK_CreateBackFillRequest Var_Request;
};
