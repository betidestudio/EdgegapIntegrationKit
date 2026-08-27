// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetMatchmakingTicket.h"

UEGIK_GetMatchmakingTicket* UEGIK_GetMatchmakingTicket::GetMatchmakingTicket(FEGIK_MatchmakingRequest MatchmakingRequest)
{
	UEGIK_GetMatchmakingTicket* Node = NewObject<UEGIK_GetMatchmakingTicket>();
	Node->Var_MatchmakingRequest = MatchmakingRequest;
	return Node;
}

FString UEGIK_GetMatchmakingTicket::GetEndpointURL() const
{
	FString BaseURL = Var_MatchmakingRequest.MatchmakingURL.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
		: Var_MatchmakingRequest.MatchmakingURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/tickets/%s"), *BaseURL, *Var_MatchmakingRequest.TicketId);
}

EEGIK_HttpVerb UEGIK_GetMatchmakingTicket::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_GetMatchmakingTicket::GetAuthorizationHeader() const
{
	return Var_MatchmakingRequest.AuthToken.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
		: Var_MatchmakingRequest.AuthToken;
}

void UEGIK_GetMatchmakingTicket::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_MatchmakingResponse Response;
	Response.TicketId = JsonObject->GetStringField(TEXT("id"));
	Response.GameProfile = JsonObject->GetStringField(TEXT("profile"));
	FDateTime::ParseIso8601(*JsonObject->GetStringField(TEXT("created_at")), Response.CreatedAt);

	const TSharedPtr<FJsonObject>* AssignmentObject;
	if (JsonObject->HasTypedField<EJson::Object>(TEXT("assignment")))
	{
		if (JsonObject->TryGetObjectField(TEXT("assignment"), AssignmentObject))
		{
			Response.Assignment = FEGIK_AssignmentStruct(*AssignmentObject);
		}
		else
		{
			Response.Assignment = FEGIK_AssignmentStruct("null");
		}
	}
	else
	{
		Response.Assignment = FEGIK_AssignmentStruct("null");
	}

	if (JsonObject->HasField(TEXT("group_id")))
	{
		Response.GroupId = JsonObject->GetStringField(TEXT("group_id"));
	}
	if (JsonObject->HasField(TEXT("team_id")))
	{
		Response.TeamId = JsonObject->GetStringField(TEXT("team_id"));
	}
	if (JsonObject->HasField(TEXT("match_id")))
	{
		Response.MatchId = JsonObject->GetStringField(TEXT("match_id"));
	}
	if (JsonObject->HasField(TEXT("player_ip")))
	{
		Response.IP = JsonObject->GetStringField(TEXT("player_ip"));
	}
	if (JsonObject->HasField(TEXT("status")))
	{
		Response.Status = JsonObject->GetStringField(TEXT("status"));
	}

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_GetMatchmakingTicket::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_GetMatchmakingTicket::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
