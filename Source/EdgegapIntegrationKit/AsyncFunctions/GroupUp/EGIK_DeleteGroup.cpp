// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeleteGroup.h"
#include "EGIKBlueprintFunctionLibrary.h"

namespace EGIK_GroupUp_DeleteGroup
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

UEGIK_DeleteGroup* UEGIK_DeleteGroup::DeleteGroup(const FEGIK_DeleteGroupRequest& Request)
{
	UEGIK_DeleteGroup* Node = NewObject<UEGIK_DeleteGroup>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_DeleteGroup::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s"),
		*EGIK_GroupUp_DeleteGroup::ResolveMatchmakingURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId);
}

EEGIK_HttpVerb UEGIK_DeleteGroup::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_DeleteGroup::GetAuthorizationHeader() const
{
	return EGIK_GroupUp_DeleteGroup::ResolveAuthToken(Var_Request.AuthToken);
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
