// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_CreateSlot.h"
#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_SB_CreateSlot* UEGIK_SB_CreateSlot::CreateSlot(
	const FString& RequestId,
	const FString& SlotName,
	int32 AvailableSeats,
	const FString& MetadataJson,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_CreateSlot* Node = NewObject<UEGIK_SB_CreateSlot>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	Node->Var_SlotName = SlotName;
	Node->Var_AvailableSeats = AvailableSeats;
	Node->Var_MetadataJson = MetadataJson;
	return Node;
}

FString UEGIK_SB_CreateSlot::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s/slots"), *BaseURL, *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_SB_CreateSlot::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_SB_CreateSlot::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

TSharedPtr<FJsonObject> UEGIK_SB_CreateSlot::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField(TEXT("name"), Var_SlotName);
	JsonObject->SetNumberField(TEXT("available_seats"), Var_AvailableSeats);

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

void UEGIK_SB_CreateSlot::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_ServerInstanceSlot Slot(JsonObject);
	OnSuccess.Broadcast(Slot, FEGIK_ErrorStruct());
}

void UEGIK_SB_CreateSlot::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_ServerInstanceSlot(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
