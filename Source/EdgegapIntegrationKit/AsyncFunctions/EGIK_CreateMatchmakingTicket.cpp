// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateMatchmakingTicket.h"

UEGIK_CreateMatchmakingTicket* UEGIK_CreateMatchmakingTicket::CreateMatchmakingTicket(const FEGIK_CreateMatchmakingStruct& MatchmakingStruct)
{
	UEGIK_CreateMatchmakingTicket* Node = NewObject<UEGIK_CreateMatchmakingTicket>();
	Node->Var_MatchmakingStruct = MatchmakingStruct;
	return Node;
}

FString UEGIK_CreateMatchmakingTicket::GetEndpointURL() const
{
	FString BaseURL = Var_MatchmakingStruct.MatchmakingURL.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
		: Var_MatchmakingStruct.MatchmakingURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return BaseURL + TEXT("/tickets");
}

EEGIK_HttpVerb UEGIK_CreateMatchmakingTicket::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_CreateMatchmakingTicket::GetAuthorizationHeader() const
{
	return Var_MatchmakingStruct.AuthToken.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
		: Var_MatchmakingStruct.AuthToken;
}

TMap<FString, FString> UEGIK_CreateMatchmakingTicket::GetAdditionalHeaders() const
{
	TMap<FString, FString> Headers;
	if (!Var_MatchmakingStruct.RealIp.IsEmpty())
	{
		Headers.Add(TEXT("X-Real-Ip"), Var_MatchmakingStruct.RealIp);
	}
	if (!Var_MatchmakingStruct.PlayerIp.IsEmpty())
	{
		Headers.Add(TEXT("Player-Ip"), Var_MatchmakingStruct.PlayerIp);
	}
	return Headers;
}

TSharedPtr<FJsonObject> UEGIK_CreateMatchmakingTicket::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("profile"), Var_MatchmakingStruct.Profile);

	// Parse attributes JSON string
	TSharedPtr<FJsonObject> AttributesJsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Var_MatchmakingStruct.Attributes);
	if (FJsonSerializer::Deserialize(Reader, AttributesJsonObject) && AttributesJsonObject.IsValid())
	{
		JsonObject->SetObjectField(TEXT("attributes"), AttributesJsonObject);
	}
	else
	{
		UE_LOG(LogEdgegap, Error, TEXT("[Matchmaking] Failed to parse attributes JSON string."));
	}

	return JsonObject;
}

void UEGIK_CreateMatchmakingTicket::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
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
	JsonObject->TryGetStringField(TEXT("group_id"), Response.GroupId);
	JsonObject->TryGetStringField(TEXT("team_id"), Response.TeamId);
	JsonObject->TryGetStringField(TEXT("match_id"), Response.MatchId);
	JsonObject->TryGetStringField(TEXT("status"), Response.Status);
	JsonObject->TryGetStringField(TEXT("player_ip"), Response.IP);

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

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_CreateMatchmakingTicket::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_CreateMatchmakingTicket::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
