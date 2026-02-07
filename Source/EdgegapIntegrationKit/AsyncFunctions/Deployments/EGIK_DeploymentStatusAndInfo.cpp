// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeploymentStatusAndInfo.h"

UEGIK_DeploymentStatusAndInfo* UEGIK_DeploymentStatusAndInfo::GetDeploymentStatusAndInfo(const FString& RequestId)
{
	UEGIK_DeploymentStatusAndInfo* Node = NewObject<UEGIK_DeploymentStatusAndInfo>();
	Node->Var_RequestId = RequestId;
	return Node;
}

FString UEGIK_DeploymentStatusAndInfo::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/status/%s"), *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_DeploymentStatusAndInfo::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_DeploymentStatusAndInfo::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	// Use the constructor that takes a JSON object
	FEGIK_DeploymentStatusAndInfoResponse Response(JsonObject);
	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_DeploymentStatusAndInfo::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_DeploymentStatusAndInfoResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
