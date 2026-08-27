// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_AuthorizeUserOnRelaySession.h"

UEGIK_AuthorizeUserOnRelaySession* UEGIK_AuthorizeUserOnRelaySession::AuthorizeUserOnRelaySession(
	FEGIK_AuthorizeUserOnRelaySessionInput Input)
{
	UEGIK_AuthorizeUserOnRelaySession* Node = NewObject<UEGIK_AuthorizeUserOnRelaySession>();
	Node->Var_Input = Input;
	return Node;
}

FString UEGIK_AuthorizeUserOnRelaySession::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/relays/sessions:authorize-user");
}

EEGIK_HttpVerb UEGIK_AuthorizeUserOnRelaySession::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

TSharedPtr<FJsonObject> UEGIK_AuthorizeUserOnRelaySession::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("session_id", Var_Input.SessionId);
	JsonObject->SetStringField("user_ip", Var_Input.UserIp);
	return JsonObject;
}

void UEGIK_AuthorizeUserOnRelaySession::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
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

void UEGIK_AuthorizeUserOnRelaySession::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
