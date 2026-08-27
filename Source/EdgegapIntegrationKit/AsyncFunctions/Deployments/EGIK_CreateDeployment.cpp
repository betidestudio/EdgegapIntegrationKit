// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateDeployment.h"

UEGIK_CreateDeployment* UEGIK_CreateDeployment::CreateDeployment(FEGIK_CreateDeploymentStruct DeploymentStruct)
{
	UEGIK_CreateDeployment* BlueprintNode = NewObject<UEGIK_CreateDeployment>();
	BlueprintNode->Var_DeploymentStruct = DeploymentStruct;
	return BlueprintNode;
}

FString UEGIK_CreateDeployment::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/deploy");
}

EEGIK_HttpVerb UEGIK_CreateDeployment::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

TSharedPtr<FJsonObject> UEGIK_CreateDeployment::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	JsonObject->SetStringField(TEXT("app_name"), Var_DeploymentStruct.AppName);
	JsonObject->SetStringField(TEXT("version_name"), Var_DeploymentStruct.VersionName);

	if (Var_DeploymentStruct.UserIPs.Num() > 0)
	{
		JsonObject->SetArrayField(TEXT("ip_list"), StringArrayToJsonArray(Var_DeploymentStruct.UserIPs));
	}

	if (Var_DeploymentStruct.UserGeoIPs.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> UserGeoIPsArray;
		for (const auto& GeoIP : Var_DeploymentStruct.UserGeoIPs)
		{
			TSharedPtr<FJsonObject> GeoIPObject = MakeShareable(new FJsonObject);
			GeoIPObject->SetStringField(TEXT("ip"), GeoIP.IP);
			GeoIPObject->SetNumberField(TEXT("latitude"), GeoIP.Latitude);
			GeoIPObject->SetNumberField(TEXT("longitude"), GeoIP.Longitude);
			UserGeoIPsArray.Add(MakeShareable(new FJsonValueObject(GeoIPObject)));
		}
		JsonObject->SetArrayField(TEXT("geo_ip_list"), UserGeoIPsArray);
	}

	if (Var_DeploymentStruct.DeploymentVariables.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> EnvVarsArray;
		for (const auto& EnvVar : Var_DeploymentStruct.DeploymentVariables)
		{
			TSharedPtr<FJsonObject> EnvVarObject = MakeShareable(new FJsonObject);
			EnvVarObject->SetStringField(TEXT("key"), EnvVar.Key);
			EnvVarObject->SetStringField(TEXT("value"), EnvVar.Value);
			EnvVarObject->SetBoolField(TEXT("is_hidden"), EnvVar.bIsHidden);
			EnvVarsArray.Add(MakeShareable(new FJsonValueObject(EnvVarObject)));
		}
		JsonObject->SetArrayField(TEXT("env_vars"), EnvVarsArray);
	}

	JsonObject->SetBoolField(TEXT("skip_telemetry"), Var_DeploymentStruct.bSkipTelemetry);

	if (Var_DeploymentStruct.Location.Latitude != 0 || Var_DeploymentStruct.Location.Longitude != 0)
	{
		TSharedPtr<FJsonObject> LocationObject = MakeShareable(new FJsonObject);
		LocationObject->SetNumberField(TEXT("latitude"), Var_DeploymentStruct.Location.Latitude);
		LocationObject->SetNumberField(TEXT("longitude"), Var_DeploymentStruct.Location.Longitude);
		JsonObject->SetObjectField(TEXT("location"), LocationObject);
	}

	if (!Var_DeploymentStruct.WebHookURL.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("webhook_url"), Var_DeploymentStruct.WebHookURL);
	}

	if (Var_DeploymentStruct.Tags.Num() > 0)
	{
		JsonObject->SetArrayField(TEXT("tags"), StringArrayToJsonArray(Var_DeploymentStruct.Tags));
	}

	if (Var_DeploymentStruct.ContainerLogStorage.bEnabled)
	{
		TSharedPtr<FJsonObject> ContainerLogStorageObject = MakeShareable(new FJsonObject);
		ContainerLogStorageObject->SetBoolField(TEXT("enabled"), Var_DeploymentStruct.ContainerLogStorage.bEnabled);
		ContainerLogStorageObject->SetStringField(TEXT("endpoint_storage"), Var_DeploymentStruct.ContainerLogStorage.EndpointStorageName);
		JsonObject->SetObjectField(TEXT("container_log_storage"), ContainerLogStorageObject);
	}

	if (Var_DeploymentStruct.Filters.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> FiltersArray;
		for (auto Filter : Var_DeploymentStruct.Filters)
		{
			FiltersArray.Add(MakeShareable(new FJsonValueObject(Filter.ToJsonObject())));
		}
		JsonObject->SetArrayField(TEXT("filters"), FiltersArray);
	}

	if (!Var_DeploymentStruct.ContainerCommand.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("command"), Var_DeploymentStruct.ContainerCommand);
	}

	if (!Var_DeploymentStruct.ContainerArguments.IsEmpty())
	{
		JsonObject->SetStringField(TEXT("arguments"), Var_DeploymentStruct.ContainerArguments);
	}

	return JsonObject;
}

void UEGIK_CreateDeployment::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_CreateDeploymentResponse Response;
	Response.RequestId = JsonObject->GetStringField(TEXT("request_id"));
	Response.RequestDns = JsonObject->GetStringField(TEXT("request_dns"));
	Response.RequestApp = JsonObject->GetStringField(TEXT("request_app"));
	Response.RequestVersion = JsonObject->GetStringField(TEXT("request_version"));
	Response.RequestUserCount = JsonObject->GetIntegerField(TEXT("request_user_count"));

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_CreateDeployment::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_CreateDeploymentResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
