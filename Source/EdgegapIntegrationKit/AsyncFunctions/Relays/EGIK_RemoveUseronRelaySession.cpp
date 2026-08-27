// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_RemoveUseronRelaySession.h"

UEGIK_RemoveUseronRelaySession* UEGIK_RemoveUseronRelaySession::RemoveUseronRelaySession(FString SessionId,
	FString AuthorizationToken)
{
	UEGIK_RemoveUseronRelaySession* Node = NewObject<UEGIK_RemoveUseronRelaySession>();
	Node->Var_SessionId = SessionId;
	Node->Var_AuthorizationToken = AuthorizationToken;
	return Node;
}

FString UEGIK_RemoveUseronRelaySession::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/relays/sessions:revoke-user");
}

EEGIK_HttpVerb UEGIK_RemoveUseronRelaySession::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

TSharedPtr<FJsonObject> UEGIK_RemoveUseronRelaySession::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("session_id", Var_SessionId);
	JsonObject->SetStringField("authorization_token", Var_AuthorizationToken);
	return JsonObject;
}

void UEGIK_RemoveUseronRelaySession::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (JsonObject.IsValid())
	{
		OnSuccess.Broadcast(JsonObject, FEGIK_ErrorStruct());
	}
	else
	{
		OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
	}
}

void UEGIK_RemoveUseronRelaySession::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
