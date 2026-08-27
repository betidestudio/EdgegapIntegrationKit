// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_Monitor.h"
#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_SB_Monitor* UEGIK_SB_Monitor::Monitor(const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_Monitor* Node = NewObject<UEGIK_SB_Monitor>();
	Node->Var_Overrides = Overrides;
	return Node;
}

FString UEGIK_SB_Monitor::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return BaseURL + TEXT("/monitor");
}

EEGIK_HttpVerb UEGIK_SB_Monitor::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_SB_Monitor::GetAuthorizationHeader() const
{
	// Monitor endpoint doesn't require authentication
	return TEXT("");
}

void UEGIK_SB_Monitor::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_MonitorResponse Response(JsonObject);
	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_SB_Monitor::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_MonitorResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
