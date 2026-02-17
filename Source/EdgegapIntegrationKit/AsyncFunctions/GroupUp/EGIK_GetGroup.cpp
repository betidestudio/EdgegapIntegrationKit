// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetGroup.h"

namespace EGIK_GroupUp_GetGroup
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

UEGIK_GetGroup* UEGIK_GetGroup::GetGroup(const FEGIK_GetGroupRequest& Request)
{
	UEGIK_GetGroup* Node = NewObject<UEGIK_GetGroup>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_GetGroup::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s"),
		*EGIK_GroupUp_GetGroup::ResolveMatchmakingURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId);
}

EEGIK_HttpVerb UEGIK_GetGroup::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_GetGroup::GetAuthorizationHeader() const
{
	return EGIK_GroupUp_GetGroup::ResolveAuthToken(Var_Request.AuthToken);
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
