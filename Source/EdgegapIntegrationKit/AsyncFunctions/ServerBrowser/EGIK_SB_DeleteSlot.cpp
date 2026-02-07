// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_DeleteSlot.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UEGIK_SB_DeleteSlot* UEGIK_SB_DeleteSlot::DeleteSlot(
	const FString& RequestId,
	const FString& SlotName,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_DeleteSlot* Node = NewObject<UEGIK_SB_DeleteSlot>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	Node->Var_SlotName = SlotName;
	return Node;
}

FString UEGIK_SB_DeleteSlot::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s/slots/%s"), *BaseURL, *Var_RequestId, *FGenericPlatformHttp::UrlEncode(Var_SlotName));
}

EEGIK_HttpVerb UEGIK_SB_DeleteSlot::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_SB_DeleteSlot::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

void UEGIK_SB_DeleteSlot::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	// DELETE returns 204 No Content on success
	OnSuccess.Broadcast(FEGIK_ErrorStruct());
}

void UEGIK_SB_DeleteSlot::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
