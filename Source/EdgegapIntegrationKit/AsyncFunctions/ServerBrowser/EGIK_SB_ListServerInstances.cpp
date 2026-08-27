// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_ListServerInstances.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UEGIK_SB_ListServerInstances* UEGIK_SB_ListServerInstances::ListServerInstances(
	const FString& Cursor,
	int32 Limit,
	const FString& Filter,
	const FString& OrderBy,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_ListServerInstances* Node = NewObject<UEGIK_SB_ListServerInstances>();
	Node->Var_Overrides = Overrides;
	Node->Var_Cursor = Cursor;
	Node->Var_Limit = FMath::Clamp(Limit, 1, 100);
	Node->Var_Filter = Filter;
	Node->Var_OrderBy = OrderBy;
	return Node;
}

FString UEGIK_SB_ListServerInstances::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}

	FString URL = BaseURL + TEXT("/server-instances");

	// Build query parameters
	TArray<FString> QueryParams;

	if (!Var_Cursor.IsEmpty())
	{
		QueryParams.Add(FString::Printf(TEXT("cursor=%s"), *FGenericPlatformHttp::UrlEncode(Var_Cursor)));
	}

	QueryParams.Add(FString::Printf(TEXT("limit=%d"), Var_Limit));

	if (!Var_Filter.IsEmpty())
	{
		QueryParams.Add(FString::Printf(TEXT("filter=%s"), *FGenericPlatformHttp::UrlEncode(Var_Filter)));
	}

	if (!Var_OrderBy.IsEmpty())
	{
		QueryParams.Add(FString::Printf(TEXT("orderby=%s"), *FGenericPlatformHttp::UrlEncode(Var_OrderBy)));
	}

	if (QueryParams.Num() > 0)
	{
		URL += TEXT("?") + FString::Join(QueryParams, TEXT("&"));
	}

	return URL;
}

EEGIK_HttpVerb UEGIK_SB_ListServerInstances::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_SB_ListServerInstances::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserClientToken() : Var_Overrides.AuthToken;
}

void UEGIK_SB_ListServerInstances::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_ListServerInstancesResponse Response(JsonObject);
	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_SB_ListServerInstances::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_ListServerInstancesResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
