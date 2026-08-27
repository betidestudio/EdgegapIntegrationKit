// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_UpdateGroupMember.h"
#include "EGIK_MatchmakingRequestUtils.h"

UEGIK_UpdateGroupMember* UEGIK_UpdateGroupMember::UpdateGroupMember(const FEGIK_UpdateGroupMemberRequest& Request)
{
	UEGIK_UpdateGroupMember* Node = NewObject<UEGIK_UpdateGroupMember>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_UpdateGroupMember::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s/members/%s"),
		*EGIK_MatchmakingRequest::ResolveURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId,
		*Var_Request.MemberId);
}

EEGIK_HttpVerb UEGIK_UpdateGroupMember::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::PATCH;
}

FString UEGIK_UpdateGroupMember::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_Request.AuthToken);
}

bool UEGIK_UpdateGroupMember::ValidateRequest(FString& OutErrorMessage) const
{
	return EGIK_MatchmakingRequest::ValidateConnection(Var_Request.MatchmakingURL, Var_Request.AuthToken, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("GroupId"), Var_Request.GroupId, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("MemberId"), Var_Request.MemberId, OutErrorMessage);
}

TSharedPtr<FJsonObject> UEGIK_UpdateGroupMember::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetBoolField(TEXT("is_ready"), Var_Request.bIsReady);
	return JsonObject;
}

void UEGIK_UpdateGroupMember::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	OnSuccess.Broadcast(FEGIK_GroupUpMemberResponse(JsonObject), FEGIK_ErrorStruct());
}

void UEGIK_UpdateGroupMember::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_UpdateGroupMember::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
