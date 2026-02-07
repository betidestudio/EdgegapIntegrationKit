// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetPublicIp.h"

UEGIK_GetPublicIp* UEGIK_GetPublicIp::GetPublicIp()
{
	UEGIK_GetPublicIp* Node = NewObject<UEGIK_GetPublicIp>();
	return Node;
}

FString UEGIK_GetPublicIp::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/ip");
}

EEGIK_HttpVerb UEGIK_GetPublicIp::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_GetPublicIp::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FString IpAddress = JsonObject->GetStringField(TEXT("public_ip"));
	OnSuccess.Broadcast(IpAddress, FEGIK_ErrorStruct());
}

void UEGIK_GetPublicIp::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(TEXT(""), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
