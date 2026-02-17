// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateGroup.h"

namespace EGIK_GroupUp_CreateGroup
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

UEGIK_CreateGroup* UEGIK_CreateGroup::CreateGroup(const FEGIK_CreateGroupRequest& Request)
{
	UEGIK_CreateGroup* Node = NewObject<UEGIK_CreateGroup>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_CreateGroup::GetEndpointURL() const
{
	return EGIK_GroupUp_CreateGroup::ResolveMatchmakingURL(Var_Request.MatchmakingURL) + TEXT("/groups");
}

EEGIK_HttpVerb UEGIK_CreateGroup::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_CreateGroup::GetAuthorizationHeader() const
{
	return EGIK_GroupUp_CreateGroup::ResolveAuthToken(Var_Request.AuthToken);
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

void UEGIK_CreateGroup::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	OnSuccess.Broadcast(FEGIK_GroupUpMemberResponse(JsonObject), FEGIK_ErrorStruct());
}

void UEGIK_CreateGroup::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_CreateGroup::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_GroupUpMemberResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
