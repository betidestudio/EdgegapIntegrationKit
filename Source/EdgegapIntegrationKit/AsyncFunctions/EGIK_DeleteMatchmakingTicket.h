// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_DeleteMatchmakingTicket.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_DeleteMatchmakingRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString TicketId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString MatchmakingURL;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString AuthToken;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeleteMatchmakingTicketResponse, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeleteMatchmakingTicket : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Matchmaking")
	static UEGIK_DeleteMatchmakingTicket* DeleteMatchmakingTicket(FEGIK_DeleteMatchmakingRequest Request);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FDeleteMatchmakingTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FDeleteMatchmakingTicketResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FDeleteMatchmakingTicketResponse OnRateLimited;
	
private:
	FEGIK_DeleteMatchmakingRequest Var_Request;
};
