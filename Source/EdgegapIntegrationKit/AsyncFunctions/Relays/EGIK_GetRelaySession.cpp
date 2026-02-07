// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetRelaySession.h"

UEGIK_GetRelaySession* UEGIK_GetRelaySession::GetRelaySession(FString SessionId)
{
	UEGIK_GetRelaySession* Node = NewObject<UEGIK_GetRelaySession>();
	Node->Var_SessionId = SessionId;
	return Node;
}

FString UEGIK_GetRelaySession::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/relays/sessions/%s"), *Var_SessionId);
}

EEGIK_HttpVerb UEGIK_GetRelaySession::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_GetRelaySession::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
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

void UEGIK_GetRelaySession::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
