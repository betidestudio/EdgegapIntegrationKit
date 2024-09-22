// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_GetMatchmakingTicket.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_MatchmakingRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString TicketId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString MatchmakingURL;
	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetMatchmakingTicketResponse, const FEGIK_MatchmakingResponse&, Response, const FEGIK_ErrorStruct&, Error);
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetMatchmakingTicket : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Matchmaking")
	static UEGIK_GetMatchmakingTicket* GetMatchmakingTicket(FEGIK_MatchmakingRequest MatchmakingRequest);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FGetMatchmakingTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Matchmaking")
	FGetMatchmakingTicketResponse OnFailure;
	
private:
	FEGIK_MatchmakingRequest Var_MatchmakingRequest;
};
