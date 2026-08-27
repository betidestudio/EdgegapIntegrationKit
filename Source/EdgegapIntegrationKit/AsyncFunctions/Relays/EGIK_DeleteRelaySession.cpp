// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeleteRelaySession.h"

UEGIK_DeleteRelaySession* UEGIK_DeleteRelaySession::DeleteRelaySession(FString SessionId)
{
	UEGIK_DeleteRelaySession* Node = NewObject<UEGIK_DeleteRelaySession>();
	Node->Var_SessionId = SessionId;
	return Node;
}

FString UEGIK_DeleteRelaySession::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/relays/sessions/%s"), *Var_SessionId);
}

EEGIK_HttpVerb UEGIK_DeleteRelaySession::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

void UEGIK_DeleteRelaySession::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	OnSuccess.Broadcast(FEGIK_ErrorStruct());
}

void UEGIK_DeleteRelaySession::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
