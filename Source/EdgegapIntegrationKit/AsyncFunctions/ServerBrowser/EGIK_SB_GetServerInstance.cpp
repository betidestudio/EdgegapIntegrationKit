// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_GetServerInstance.h"
#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_SB_GetServerInstance* UEGIK_SB_GetServerInstance::GetServerInstance(
	const FString& RequestId,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_GetServerInstance* Node = NewObject<UEGIK_SB_GetServerInstance>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	return Node;
}

FString UEGIK_SB_GetServerInstance::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s"), *BaseURL, *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_SB_GetServerInstance::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_SB_GetServerInstance::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

void UEGIK_SB_GetServerInstance::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_ServerInstance ServerInstance(JsonObject);
	OnSuccess.Broadcast(ServerInstance, FEGIK_ErrorStruct());
}

void UEGIK_SB_GetServerInstance::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_ServerInstance(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
