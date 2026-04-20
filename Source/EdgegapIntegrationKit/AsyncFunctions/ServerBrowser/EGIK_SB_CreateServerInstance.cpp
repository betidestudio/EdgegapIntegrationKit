// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_CreateServerInstance.h"
#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_SB_CreateServerInstance* UEGIK_SB_CreateServerInstance::CreateServerInstance(FEGIK_SB_CreateServerInstanceRequest Request)
{
	UEGIK_SB_CreateServerInstance* Node = NewObject<UEGIK_SB_CreateServerInstance>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_SB_CreateServerInstance::GetEndpointURL() const
{
	FString BaseURL = Var_Request.Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Request.Overrides.ServerBrowserURL;
	// Remove trailing slash if present
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return BaseURL + TEXT("/server-instances");
}

EEGIK_HttpVerb UEGIK_SB_CreateServerInstance::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_SB_CreateServerInstance::GetAuthorizationHeader() const
{
	return Var_Request.Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Request.Overrides.AuthToken;
}

TSharedPtr<FJsonObject> UEGIK_SB_CreateServerInstance::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField(TEXT("request_id"), Var_Request.RequestId);

	// Parse metadata JSON string to object
	if (!Var_Request.MetadataJson.IsEmpty())
	{
		TSharedPtr<FJsonObject> MetadataObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Var_Request.MetadataJson);
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

	// Build slots array
	TArray<TSharedPtr<FJsonValue>> SlotsArray;
	for (const auto& Slot : Var_Request.Slots)
	{
		SlotsArray.Add(MakeShareable(new FJsonValueObject(Slot.ToJsonObject())));
	}
	JsonObject->SetArrayField(TEXT("slots"), SlotsArray);

	// Build server object
	JsonObject->SetObjectField(TEXT("server"), Var_Request.Server.ToJsonObject());

	return JsonObject;
}

void UEGIK_SB_CreateServerInstance::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_ServerInstance ServerInstance(JsonObject);
	OnSuccess.Broadcast(ServerInstance, FEGIK_ErrorStruct());
}

void UEGIK_SB_CreateServerInstance::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_ServerInstance(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
