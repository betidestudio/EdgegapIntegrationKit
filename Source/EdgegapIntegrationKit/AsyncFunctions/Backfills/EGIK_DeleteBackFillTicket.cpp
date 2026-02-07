// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeleteBackFillTicket.h"

UEGIK_DeleteBackFillTicket* UEGIK_DeleteBackFillTicket::DeleteBackFillTicket(const FString& backfillId, const FString& MatchmakingURL, const FString& AuthToken)
{
	UEGIK_DeleteBackFillTicket* Node = NewObject<UEGIK_DeleteBackFillTicket>();
	Node->Var_BackfillId = backfillId;
	Node->Var_MatchmakingURL = MatchmakingURL;
	Node->Var_AuthToken = AuthToken;
	return Node;
}

FString UEGIK_DeleteBackFillTicket::GetEndpointURL() const
{
	FString BaseURL = Var_MatchmakingURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/backfills/%s"), *BaseURL, *Var_BackfillId);
}

EEGIK_HttpVerb UEGIK_DeleteBackFillTicket::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_DeleteBackFillTicket::GetAuthorizationHeader() const
{
	return Var_AuthToken;
}

void UEGIK_DeleteBackFillTicket::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	OnSuccess.Broadcast(FEGIK_ErrorStruct());
}

void UEGIK_DeleteBackFillTicket::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_DeleteBackFillTicket::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_ErrorStruct(429, ResponseContent));
}
