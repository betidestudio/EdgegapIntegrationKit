// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SelfStopDeployment.h"

UEGIK_SelfStopDeployment* UEGIK_SelfStopDeployment::SelfStopDeployment(const FString& RequestId, const FString& AccessPointId)
{
	UEGIK_SelfStopDeployment* Node = NewObject<UEGIK_SelfStopDeployment>();
	Node->Var_RequestId = RequestId;
	Node->Var_AccessPointId = AccessPointId;
	return Node;
}

FString UEGIK_SelfStopDeployment::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/self/stop/%s/%s"), *Var_RequestId, *Var_AccessPointId);
}

EEGIK_HttpVerb UEGIK_SelfStopDeployment::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

void UEGIK_SelfStopDeployment::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	OnSuccess.Broadcast(true, FEGIK_ErrorStruct());
}

void UEGIK_SelfStopDeployment::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(false, FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
