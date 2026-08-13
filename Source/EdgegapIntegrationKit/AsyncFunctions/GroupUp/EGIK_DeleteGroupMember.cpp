// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeleteGroupMember.h"
#include "EGIK_MatchmakingRequestUtils.h"

UEGIK_DeleteGroupMember* UEGIK_DeleteGroupMember::DeleteGroupMember(const FEGIK_DeleteGroupMemberRequest& Request)
{
	UEGIK_DeleteGroupMember* Node = NewObject<UEGIK_DeleteGroupMember>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_DeleteGroupMember::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s/members/%s"),
		*EGIK_MatchmakingRequest::ResolveURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId,
		*Var_Request.MemberId);
}

EEGIK_HttpVerb UEGIK_DeleteGroupMember::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_DeleteGroupMember::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_Request.AuthToken);
}

bool UEGIK_DeleteGroupMember::ValidateRequest(FString& OutErrorMessage) const
{
	return EGIK_MatchmakingRequest::ValidateConnection(Var_Request.MatchmakingURL, Var_Request.AuthToken, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("GroupId"), Var_Request.GroupId, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("MemberId"), Var_Request.MemberId, OutErrorMessage);
}

void UEGIK_DeleteGroupMember::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	OnSuccess.Broadcast(FEGIK_ErrorStruct());
}

void UEGIK_DeleteGroupMember::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_DeleteGroupMember::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_ErrorStruct(429, ResponseContent));
}
