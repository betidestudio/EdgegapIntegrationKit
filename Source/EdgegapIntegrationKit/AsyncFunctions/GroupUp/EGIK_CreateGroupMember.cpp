// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateGroupMember.h"

namespace EGIK_GroupUp_CreateGroupMember
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

UEGIK_CreateGroupMember* UEGIK_CreateGroupMember::CreateGroupMember(const FEGIK_CreateGroupMemberRequest& Request)
{
	UEGIK_CreateGroupMember* Node = NewObject<UEGIK_CreateGroupMember>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_CreateGroupMember::GetEndpointURL() const
{
	return FString::Printf(TEXT("%s/groups/%s/members"),
		*EGIK_GroupUp_CreateGroupMember::ResolveMatchmakingURL(Var_Request.MatchmakingURL),
		*Var_Request.GroupId);
}

EEGIK_HttpVerb UEGIK_CreateGroupMember::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_CreateGroupMember::GetAuthorizationHeader() const
{
	return EGIK_GroupUp_CreateGroupMember::ResolveAuthToken(Var_Request.AuthToken);
}

TSharedPtr<FJsonObject> UEGIK_CreateGroupMember::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("profile"), Var_Request.Profile);
	JsonObject->SetBoolField(TEXT("is_ready"), Var_Request.bIsReady);

	if (!Var_Request.PlayerIp.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("player_ip"), Var_Request.PlayerIp);
	}

	if (!Var_Request.Attributes.IsEmpty())
	{
		TSharedPtr<FJsonObject> AttributesObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Var_Request.Attributes);
		if (FJsonSerializer::Deserialize(Reader, AttributesObject) && AttributesObject.IsValid())
		{
			JsonObject->SetObjectField(TEXT("attributes"), AttributesObject);
		}
		else
		{
			LogError(TEXT("Failed to parse Attributes JSON."));
		}
	}

	return JsonObject;
}

void UEGIK_CreateGroupMember::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	OnSuccess.Broadcast(FEGIK_GroupUpMemberResponse(JsonObject), FEGIK_ErrorStruct());
}

void UEGIK_CreateGroupMember::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_CreateGroupMember::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
