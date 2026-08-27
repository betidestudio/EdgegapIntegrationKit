// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_GetSlot.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UEGIK_SB_GetSlot* UEGIK_SB_GetSlot::GetSlot(
	const FString& RequestId,
	const FString& SlotName,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_GetSlot* Node = NewObject<UEGIK_SB_GetSlot>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	Node->Var_SlotName = SlotName;
	return Node;
}

FString UEGIK_SB_GetSlot::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s/slots/%s"), *BaseURL, *Var_RequestId, *FGenericPlatformHttp::UrlEncode(Var_SlotName));
}

EEGIK_HttpVerb UEGIK_SB_GetSlot::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_SB_GetSlot::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

void UEGIK_SB_GetSlot::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_ServerInstanceSlot Slot(JsonObject);
	OnSuccess.Broadcast(Slot, FEGIK_ErrorStruct());
}

void UEGIK_SB_GetSlot::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_ServerInstanceSlot(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
