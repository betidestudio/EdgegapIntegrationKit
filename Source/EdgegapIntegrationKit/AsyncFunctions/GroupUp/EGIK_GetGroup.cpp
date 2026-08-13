// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetGroup.h"
#include "EGIK_MatchmakingRequestUtils.h"

UEGIK_GetGroup* UEGIK_GetGroup::GetGroup(const FEGIK_GetGroupRequest& Request)
{
	UEGIK_GetGroup* Node = NewObject<UEGIK_GetGroup>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_GetGroup::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s"),
		*EGIK_MatchmakingRequest::ResolveURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId);
}

EEGIK_HttpVerb UEGIK_GetGroup::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_GetGroup::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_Request.AuthToken);
}

bool UEGIK_GetGroup::ValidateRequest(FString& OutErrorMessage) const
{
	return EGIK_MatchmakingRequest::ValidateConnection(Var_Request.MatchmakingURL, Var_Request.AuthToken, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("GroupId"), Var_Request.GroupId, OutErrorMessage);
}

void UEGIK_GetGroup::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	OnSuccess.Broadcast(FEGIK_GroupUpGroupResponse(JsonObject), FEGIK_ErrorStruct());
}

void UEGIK_GetGroup::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_GroupUpGroupResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_GetGroup::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_GroupUpGroupResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
