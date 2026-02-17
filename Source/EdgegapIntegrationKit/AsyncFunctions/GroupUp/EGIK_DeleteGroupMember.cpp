// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeleteGroupMember.h"
#include "EGIKBlueprintFunctionLibrary.h"

namespace EGIK_GroupUp_DeleteGroupMember
{
	static FString ResolveMatchmakingURL(const FString& OverrideURL)
	{
		FString BaseURL = OverrideURL.IsEmpty()
			? UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
			: OverrideURL;
		if (BaseURL.EndsWith(TEXT("/")))
		{
			BaseURL = BaseURL.LeftChop(1);
		}
		return BaseURL;
	}

	static FString ResolveAuthToken(const FString& OverrideToken)
	{
		return OverrideToken.IsEmpty()
			? UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
			: OverrideToken;
	}
}

UEGIK_DeleteGroupMember* UEGIK_DeleteGroupMember::DeleteGroupMember(const FEGIK_DeleteGroupMemberRequest& Request)
{
	UEGIK_DeleteGroupMember* Node = NewObject<UEGIK_DeleteGroupMember>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_DeleteGroupMember::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s/members/%s"),
		*EGIK_GroupUp_DeleteGroupMember::ResolveMatchmakingURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId,
		*Var_Request.MemberId);
}

EEGIK_HttpVerb UEGIK_DeleteGroupMember::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_DeleteGroupMember::GetAuthorizationHeader() const
{
	return EGIK_GroupUp_DeleteGroupMember::ResolveAuthToken(Var_Request.AuthToken);
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
