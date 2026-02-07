// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_DeleteServerInstance.h"
#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_SB_DeleteServerInstance* UEGIK_SB_DeleteServerInstance::DeleteServerInstance(
	const FString& RequestId,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_DeleteServerInstance* Node = NewObject<UEGIK_SB_DeleteServerInstance>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	return Node;
}

FString UEGIK_SB_DeleteServerInstance::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s"), *BaseURL, *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_SB_DeleteServerInstance::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_SB_DeleteServerInstance::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

void UEGIK_SB_DeleteServerInstance::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	// DELETE returns 204 No Content on success
	OnSuccess.Broadcast(FEGIK_ErrorStruct());
}

void UEGIK_SB_DeleteServerInstance::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
