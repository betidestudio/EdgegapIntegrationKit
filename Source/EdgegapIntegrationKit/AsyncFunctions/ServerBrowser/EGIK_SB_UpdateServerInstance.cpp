// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_UpdateServerInstance.h"
#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_SB_UpdateServerInstance* UEGIK_SB_UpdateServerInstance::UpdateServerInstance(
	const FString& RequestId,
	const FString& MetadataJson,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_UpdateServerInstance* Node = NewObject<UEGIK_SB_UpdateServerInstance>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	Node->Var_MetadataJson = MetadataJson;
	return Node;
}

FString UEGIK_SB_UpdateServerInstance::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s"), *BaseURL, *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_SB_UpdateServerInstance::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::PATCH;
}

FString UEGIK_SB_UpdateServerInstance::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

TSharedPtr<FJsonObject> UEGIK_SB_UpdateServerInstance::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// Parse metadata JSON string to object
	if (!Var_MetadataJson.IsEmpty())
	{
		TSharedPtr<FJsonObject> MetadataObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Var_MetadataJson);
		if (FJsonSerializer::Deserialize(Reader, MetadataObj) && MetadataObj.IsValid())
		{
			JsonObject->SetObjectField(TEXT("metadata"), MetadataObj);
		}
		else
		{
			LogWarning(TEXT("Failed to parse metadata JSON, using empty object"));
			JsonObject->SetObjectField(TEXT("metadata"), MakeShareable(new FJsonObject));
		}
	}
	else
	{
		JsonObject->SetObjectField(TEXT("metadata"), MakeShareable(new FJsonObject));
	}

	return JsonObject;
}

void UEGIK_SB_UpdateServerInstance::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_ServerInstance ServerInstance(JsonObject);
	OnSuccess.Broadcast(ServerInstance, FEGIK_ErrorStruct());
}

void UEGIK_SB_UpdateServerInstance::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_ServerInstance(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
