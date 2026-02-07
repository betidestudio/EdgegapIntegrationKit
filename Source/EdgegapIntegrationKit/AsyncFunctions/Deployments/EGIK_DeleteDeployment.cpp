// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_DeleteDeployment.h"

UEGIK_DeleteDeployment* UEGIK_DeleteDeployment::DeleteDeployment(FString RequestId, FString ContainerLogStorage)
{
	UEGIK_DeleteDeployment* Node = NewObject<UEGIK_DeleteDeployment>();
	Node->Var_RequestId = RequestId;
	Node->Var_ContainerLogStorage = ContainerLogStorage;
	return Node;
}

FString UEGIK_DeleteDeployment::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/stop/%s"), *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_DeleteDeployment::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

void UEGIK_DeleteDeployment::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	FEGIK_DeleteDeploymentResponse Response;

	if (JsonObject.IsValid())
	{
		Response.Message = JsonObject->GetStringField(TEXT("message"));
		if (JsonObject->HasField(TEXT("deployment_summary")))
		{
			Response.DeploymentStatusAndInfo = FEGIK_DeploymentStatusAndInfoResponse(JsonObject->GetObjectField(TEXT("deployment_summary")));
		}
	}

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_DeleteDeployment::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_DeleteDeploymentResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
