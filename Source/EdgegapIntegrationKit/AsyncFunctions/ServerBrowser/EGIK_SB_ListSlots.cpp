// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_ListSlots.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UEGIK_SB_ListSlots* UEGIK_SB_ListSlots::ListSlots(
	const FString& RequestId,
	const FString& Cursor,
	int32 Limit,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_ListSlots* Node = NewObject<UEGIK_SB_ListSlots>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	Node->Var_Cursor = Cursor;
	Node->Var_Limit = FMath::Clamp(Limit, 1, 100);
	return Node;
}

FString UEGIK_SB_ListSlots::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}

	FString URL = FString::Printf(TEXT("%s/server-instances/%s/slots"), *BaseURL, *Var_RequestId);

	// Build query parameters
	TArray<FString> QueryParams;

	if (!Var_Cursor.IsEmpty())
	{
		QueryParams.Add(FString::Printf(TEXT("cursor=%s"), *FGenericPlatformHttp::UrlEncode(Var_Cursor)));
	}

	QueryParams.Add(FString::Printf(TEXT("limit=%d"), Var_Limit));

	if (QueryParams.Num() > 0)
	{
		URL += TEXT("?") + FString::Join(QueryParams, TEXT("&"));
	}

	return URL;
}

EEGIK_HttpVerb UEGIK_SB_ListSlots::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_SB_ListSlots::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

void UEGIK_SB_ListSlots::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_ListSlotsResponse Response(JsonObject);
	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_SB_ListSlots::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_ListSlotsResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
