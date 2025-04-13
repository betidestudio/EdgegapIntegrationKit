// Copyright (c) Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/BlueprintAsyncActionBase.h"
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
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateBackfillResponse, const FEGIK_CreateBackFillResponse&, Response, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateBackfill : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Backfill", meta = (BlueprintInternalUseOnly = "true"))
	static UEGIK_CreateBackfill* CreateBackFillRequest(const FEGIK_CreateBackFillRequest& Request);


	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FCreateBackfillResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Backfill")
	FCreateBackfillResponse OnFailure;

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	FEGIK_CreateBackFillRequest Var_Request;
};
