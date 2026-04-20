// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SelfStopDeployment.h"
#include "GenericPlatform/GenericPlatformMisc.h"

UEGIK_SelfStopDeployment* UEGIK_SelfStopDeployment::SelfStopDeployment()
{
	return NewObject<UEGIK_SelfStopDeployment>();
}

void UEGIK_SelfStopDeployment::Activate()
{
	const FString DeleteURL = FPlatformMisc::GetEnvironmentVariable(TEXT("ARBITRIUM_DELETE_URL"));
	const FString DeleteToken = FPlatformMisc::GetEnvironmentVariable(TEXT("ARBITRIUM_DELETE_TOKEN"));

	if (DeleteURL.IsEmpty() || DeleteToken.IsEmpty())
	{
		TArray<FString> MissingVariables;
		if (DeleteURL.IsEmpty())
		{
			MissingVariables.Add(TEXT("ARBITRIUM_DELETE_URL"));
		}
		if (DeleteToken.IsEmpty())
		{
			MissingVariables.Add(TEXT("ARBITRIUM_DELETE_TOKEN"));
		}

		const FString Message = FString::Printf(
			TEXT("Cannot self-stop deployment: missing %s. SelfStopDeployment must run inside an Edgegap deployment runtime with self-stop environment variables injected."),
			*FString::Join(MissingVariables, TEXT(", "))
		);

		LogWarning(Message);
		OnFailure.Broadcast(false, FEGIK_ErrorStruct(0, Message));
		FinishRequest();
		return;
	}

	Super::Activate();
}

FString UEGIK_SelfStopDeployment::GetEndpointURL() const
{
	return FPlatformMisc::GetEnvironmentVariable(TEXT("ARBITRIUM_DELETE_URL"));
}

EEGIK_HttpVerb UEGIK_SelfStopDeployment::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::DELETE;
}

FString UEGIK_SelfStopDeployment::GetAuthorizationHeader() const
{
	return FPlatformMisc::GetEnvironmentVariable(TEXT("ARBITRIUM_DELETE_TOKEN"));
}

void UEGIK_SelfStopDeployment::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	OnSuccess.Broadcast(true, FEGIK_ErrorStruct());
}

void UEGIK_SelfStopDeployment::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(false, FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
