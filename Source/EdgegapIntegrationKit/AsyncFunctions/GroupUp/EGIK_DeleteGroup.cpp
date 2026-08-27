// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeleteGroup.h"
#include "EGIK_MatchmakingRequestUtils.h"

UEGIK_DeleteGroup* UEGIK_DeleteGroup::DeleteGroup(const FEGIK_DeleteGroupRequest& Request)
{
	UEGIK_DeleteGroup* Node = NewObject<UEGIK_DeleteGroup>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_DeleteGroup::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s"),
		*EGIK_MatchmakingRequest::ResolveURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId);
}

EEGIK_HttpVerb UEGIK_DeleteGroup::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_DeleteGroup::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_Request.AuthToken);
}

bool UEGIK_DeleteGroup::ValidateRequest(FString& OutErrorMessage) const
{
	return EGIK_MatchmakingRequest::ValidateConnection(Var_Request.MatchmakingURL, Var_Request.AuthToken, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("GroupId"), Var_Request.GroupId, OutErrorMessage);
}

void UEGIK_DeleteGroup::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	OnSuccess.Broadcast(FEGIK_ErrorStruct());
}

void UEGIK_DeleteGroup::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_DeleteGroup::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_ErrorStruct(429, ResponseContent));
}
