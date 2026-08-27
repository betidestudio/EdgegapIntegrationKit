// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateDeploymentV2.h"

UEGIK_CreateDeploymentV2* UEGIK_CreateDeploymentV2::CreateDeploymentV2(FEGIK_CreateDeploymentV2Struct DeploymentStruct)
{
	UEGIK_CreateDeploymentV2* Node = NewObject<UEGIK_CreateDeploymentV2>();
	Node->Var_DeploymentStruct = DeploymentStruct;
	return Node;
}

FString UEGIK_CreateDeploymentV2::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v2/deployments");
}

EEGIK_HttpVerb UEGIK_CreateDeploymentV2::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

TSharedPtr<FJsonObject> UEGIK_CreateDeploymentV2::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// Required fields
	JsonObject->SetStringField(TEXT("application"), Var_DeploymentStruct.Application);
	JsonObject->SetStringField(TEXT("version"), Var_DeploymentStruct.Version);

	// Users array (required, min 1)
	TArray<TSharedPtr<FJsonValue>> UsersArray;
	for (const auto& User : Var_DeploymentStruct.Users)
	{
		UsersArray.Add(MakeShareable(new FJsonValueObject(User.ToJsonObject())));
	}
	JsonObject->SetArrayField(TEXT("users"), UsersArray);

	// Optional: require_cached_locations
	if (Var_DeploymentStruct.bRequireCachedLocations)
	{
		JsonObject->SetBoolField(TEXT("require_cached_locations"), true);
	}

	// Optional: environment_variables
	if (Var_DeploymentStruct.EnvironmentVariables.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> EnvVarsArray;
		for (const auto& EnvVar : Var_DeploymentStruct.EnvironmentVariables)
		{
			TSharedPtr<FJsonObject> EnvVarObject = MakeShareable(new FJsonObject);
			EnvVarObject->SetStringField(TEXT("key"), EnvVar.Key);
			EnvVarObject->SetStringField(TEXT("value"), EnvVar.Value);
			EnvVarObject->SetBoolField(TEXT("is_hidden"), EnvVar.bIsHidden);
			EnvVarsArray.Add(MakeShareable(new FJsonValueObject(EnvVarObject)));
		}
		JsonObject->SetArrayField(TEXT("environment_variables"), EnvVarsArray);
	}

	// Optional: tags
	if (Var_DeploymentStruct.Tags.Num() > 0)
	{
		JsonObject->SetArrayField(TEXT("tags"), StringArrayToJsonArray(Var_DeploymentStruct.Tags));
	}

	// Optional: webhooks
	if (!Var_DeploymentStruct.WebhookOnReady.URL.IsEmpty())
	{
		TSharedPtr<FJsonObject> WebhookObject = MakeShareable(new FJsonObject);
		WebhookObject->SetStringField(TEXT("url"), Var_DeploymentStruct.WebhookOnReady.URL);
		JsonObject->SetObjectField(TEXT("webhook_on_ready"), WebhookObject);
	}

	if (!Var_DeploymentStruct.WebhookOnError.URL.IsEmpty())
	{
		TSharedPtr<FJsonObject> WebhookObject = MakeShareable(new FJsonObject);
		WebhookObject->SetStringField(TEXT("url"), Var_DeploymentStruct.WebhookOnError.URL);
		JsonObject->SetObjectField(TEXT("webhook_on_error"), WebhookObject);
	}

	if (!Var_DeploymentStruct.WebhookOnTerminated.URL.IsEmpty())
	{
		TSharedPtr<FJsonObject> WebhookObject = MakeShareable(new FJsonObject);
		WebhookObject->SetStringField(TEXT("url"), Var_DeploymentStruct.WebhookOnTerminated.URL);
		JsonObject->SetObjectField(TEXT("webhook_on_terminated"), WebhookObject);
	}

	return JsonObject;
}

void UEGIK_CreateDeploymentV2::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_CreateDeploymentV2Response Response;
	Response.RequestId = JsonObject->GetStringField(TEXT("request_id"));

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_CreateDeploymentV2::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_CreateDeploymentV2Response(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
