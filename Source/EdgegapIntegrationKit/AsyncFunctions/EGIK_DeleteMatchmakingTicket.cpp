// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeleteMatchmakingTicket.h"

UEGIK_DeleteMatchmakingTicket* UEGIK_DeleteMatchmakingTicket::DeleteMatchmakingTicket(FEGIK_DeleteMatchmakingRequest Request)
{
	UEGIK_DeleteMatchmakingTicket* Node = NewObject<UEGIK_DeleteMatchmakingTicket>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_DeleteMatchmakingTicket::GetEndpointURL() const
{
	FString BaseURL = Var_Request.MatchmakingURL.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
		: Var_Request.MatchmakingURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/tickets/%s"), *BaseURL, *Var_Request.TicketId);
}

EEGIK_HttpVerb UEGIK_DeleteMatchmakingTicket::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_DeleteMatchmakingTicket::GetAuthorizationHeader() const
{
	return Var_Request.AuthToken.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
		: Var_Request.AuthToken;
}

void UEGIK_DeleteMatchmakingTicket::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	// 204 No Content indicates successful deletion
	OnSuccess.Broadcast(FEGIK_ErrorStruct());
}

void UEGIK_DeleteMatchmakingTicket::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	if (ErrorCode == 409)
	{
		// 409 Conflict indicates the ticket cannot be deleted because deployment is starting
		OnFailure.Broadcast(FEGIK_ErrorStruct(409, TEXT("Cannot delete ticket as deployment is starting. To replace leavers, use a Backfill issued by the server.")));
	}
	else
	{
		OnFailure.Broadcast(FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
	}
}

void UEGIK_DeleteMatchmakingTicket::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_ErrorStruct(429, ResponseContent));
}
