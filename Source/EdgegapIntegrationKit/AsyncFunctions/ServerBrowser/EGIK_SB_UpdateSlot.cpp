// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_UpdateSlot.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UEGIK_SB_UpdateSlot* UEGIK_SB_UpdateSlot::UpdateSlot(
	const FString& RequestId,
	const FString& SlotName,
	int32 AvailableSeats,
	const FString& MetadataJson,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_UpdateSlot* Node = NewObject<UEGIK_SB_UpdateSlot>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	Node->Var_SlotName = SlotName;
	Node->Var_AvailableSeats = AvailableSeats;
	Node->Var_MetadataJson = MetadataJson;
	return Node;
}

FString UEGIK_SB_UpdateSlot::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s/slots/%s"), *BaseURL, *Var_RequestId, *FGenericPlatformHttp::UrlEncode(Var_SlotName));
}

EEGIK_HttpVerb UEGIK_SB_UpdateSlot::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::PATCH;
}

FString UEGIK_SB_UpdateSlot::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

TSharedPtr<FJsonObject> UEGIK_SB_UpdateSlot::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// Only include available_seats if it's a valid value (>= 0)
	if (Var_AvailableSeats >= 0)
	{
		JsonObject->SetNumberField(TEXT("available_seats"), Var_AvailableSeats);
	}

	// Only include metadata if provided
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
			LogWarning(TEXT("Failed to parse metadata JSON"));
		}
	}

	return JsonObject;
}

void UEGIK_SB_UpdateSlot::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_ServerInstanceSlot Slot(JsonObject);
	OnSuccess.Broadcast(Slot, FEGIK_ErrorStruct());
}

void UEGIK_SB_UpdateSlot::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_ServerInstanceSlot(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
