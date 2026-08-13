// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetGroupMember.h"
#include "EGIK_MatchmakingRequestUtils.h"

UEGIK_GetGroupMember* UEGIK_GetGroupMember::GetGroupMember(const FEGIK_GetGroupMemberRequest& Request)
{
	UEGIK_GetGroupMember* Node = NewObject<UEGIK_GetGroupMember>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_GetGroupMember::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s/members/%s"),
		*EGIK_MatchmakingRequest::ResolveURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId,
		*Var_Request.MemberId);
}

EEGIK_HttpVerb UEGIK_GetGroupMember::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_GetGroupMember::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_Request.AuthToken);
}

bool UEGIK_GetGroupMember::ValidateRequest(FString& OutErrorMessage) const
{
	return EGIK_MatchmakingRequest::ValidateConnection(Var_Request.MatchmakingURL, Var_Request.AuthToken, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("GroupId"), Var_Request.GroupId, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("MemberId"), Var_Request.MemberId, OutErrorMessage);
}

void UEGIK_GetGroupMember::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	OnSuccess.Broadcast(FEGIK_GroupUpMemberResponse(JsonObject), FEGIK_ErrorStruct());
}

void UEGIK_GetGroupMember::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_GetGroupMember::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
