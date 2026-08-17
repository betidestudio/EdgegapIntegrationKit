// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateGroup.h"
#include "EGIK_MatchmakingProfileDebug.h"
#include "EGIK_MatchmakingRequestUtils.h"

UEGIK_CreateGroup* UEGIK_CreateGroup::CreateGroup(const FEGIK_CreateGroupRequest& Request)
{
	UEGIK_CreateGroup* Node = NewObject<UEGIK_CreateGroup>();
	Node->Var_Request = Request;
	if (Request.bLogProfileDebugInfo)
	{
		EGIK_MatchmakingProfileDebug::LogProfileAlignment(
			Request.MatchmakingURL,
			Request.AuthToken,
			TEXT("CreateGroup"),
			Request.Profile,
			Request.StructuredAttributes,
			Request.Attributes);
	}
	return Node;
}

FString UEGIK_CreateGroup::GetEndpointURL() const
{
	return EGIK_MatchmakingRequest::ResolveURL(Var_Request.MatchmakingURL) + TEXT("/groups");
}

EEGIK_HttpVerb UEGIK_CreateGroup::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_CreateGroup::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_Request.AuthToken);
}

bool UEGIK_CreateGroup::ValidateRequest(FString& OutErrorMessage) const
{
	if (!EGIK_MatchmakingRequest::ValidateConnection(Var_Request.MatchmakingURL, Var_Request.AuthToken, OutErrorMessage)
		|| !EGIK_MatchmakingRequest::ValidateProfile(Var_Request.Profile, OutErrorMessage))
	{
		return false;
	}

	TSharedPtr<FJsonObject> AttributesObject;
	return EGIK_MatchmakingAttributeUtils::Build(
		Var_Request.StructuredAttributes,
		Var_Request.Attributes,
		AttributesObject,
		OutErrorMessage);
}

TSharedPtr<FJsonObject> UEGIK_CreateGroup::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("profile"), Var_Request.Profile);
	JsonObject->SetBoolField(TEXT("is_ready"), Var_Request.bIsReady);

	if (!Var_Request.PlayerIp.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("player_ip"), Var_Request.PlayerIp);
	}

	TSharedPtr<FJsonObject> AttributesObject;
	FString AttributesError;
	if (EGIK_MatchmakingAttributeUtils::Build(Var_Request.StructuredAttributes, Var_Request.Attributes, AttributesObject, AttributesError))
	{
		JsonObject->SetObjectField(TEXT("attributes"), AttributesObject);
	}

	return JsonObject;
}

void UEGIK_CreateGroup::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	const FEGIK_GroupUpMemberResponse Response(JsonObject);
	if (Response.GroupId.IsEmpty() || Response.MemberId.IsEmpty())
	{
		HandleError(0, TEXT("Edgegap returned a successful Create Group response without the required group_id or member_id."));
		return;
	}

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_CreateGroup::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_CreateGroup::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
