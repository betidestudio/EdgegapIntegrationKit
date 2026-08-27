// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_UpdateDeploymentProperties.h"

UEGIK_UpdateDeploymentProperties* UEGIK_UpdateDeploymentProperties::UpdateDeploymentProperties(FEGIK_UpdateDeploymentPropertiesRequest Request)
{
	UEGIK_UpdateDeploymentProperties* Node = NewObject<UEGIK_UpdateDeploymentProperties>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_UpdateDeploymentProperties::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/deployments/%s"), *Var_Request.RequestId);
}

EEGIK_HttpVerb UEGIK_UpdateDeploymentProperties::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::PATCH;
}

TSharedPtr<FJsonObject> UEGIK_UpdateDeploymentProperties::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetBoolField(TEXT("is_joinable_by_session"), Var_Request.bIsJoinableBySession);
	return JsonObject;
}

void UEGIK_UpdateDeploymentProperties::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	OnSuccess.Broadcast(Var_Request.bIsJoinableBySession, FEGIK_ErrorStruct());
}

void UEGIK_UpdateDeploymentProperties::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(false, FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
