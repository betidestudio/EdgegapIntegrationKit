// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIK_CreateDeployment.h"

UEGIK_CreateDeployment* UEGIK_CreateDeployment::CreateDeployment(FEGIK_CreateDeploymentStruct DeploymentStruct)
{
	UEGIK_CreateDeployment* BlueprintNode = NewObject<UEGIK_CreateDeployment>();
	BlueprintNode->Var_DeploymentStruct = DeploymentStruct;
	return BlueprintNode;
}

void UEGIK_CreateDeployment::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	if(HttpResponse.IsValid())
	{
		if(EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if(FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				FEGIK_CreateDeploymentResponse Response;
				Response.RequestId = JsonObject->GetStringField(TEXT("request_id"));
				Response.RequestDns = JsonObject->GetStringField(TEXT("request_dns"));
				Response.RequestApp = JsonObject->GetStringField(TEXT("request_app"));
				Response.RequestVersion = JsonObject->GetStringField(TEXT("request_version"));
				Response.RequestUserCount = JsonObject->GetIntegerField(TEXT("request_user_count"));
				Response.ApSortStrategy = JsonObject->GetStringField(TEXT("ap_sort_strategy"));
				OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_CreateDeploymentResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_CreateDeploymentResponse(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_CreateDeploymentResponse(), FEGIK_ErrorStruct(0, "Failed to connect, likely the EdgeGap Server is down or this URL has been deprecated"));
	}
}

void UEGIK_CreateDeployment::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL("https://api.edgegap.com/v1/deploy");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("app_name", Var_DeploymentStruct.AppName);
	JsonObject->SetStringField("version_name", Var_DeploymentStruct.VersionName);
	JsonObject->SetBoolField("is_public_app", Var_DeploymentStruct.IsPublicApp);
	if(Var_DeploymentStruct.UserIPs.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> UserIPsArray;
		for(auto val: Var_DeploymentStruct.UserIPs)
		{
			UserIPsArray.Add(MakeShareable(new FJsonValueString(val)));
		}
		JsonObject->SetArrayField("ip_list", UserIPsArray);
	}
	if(Var_DeploymentStruct.UserGeoIPs.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> UserGeoIPsArray;
		for(auto val: Var_DeploymentStruct.UserGeoIPs)
		{
			TSharedPtr<FJsonObject> UserGeoIPsValue = MakeShareable(new FJsonObject);
			UserGeoIPsValue->SetStringField("ip", val.IP);
			UserGeoIPsValue->SetNumberField("latitude", val.Latitude);
			UserGeoIPsValue->SetNumberField("longitude", val.Longitude);
			UserGeoIPsArray.Add(MakeShareable(new FJsonValueObject(UserGeoIPsValue)));
		}
		JsonObject->SetArrayField("geo_ip_list", UserGeoIPsArray);
	}
	if(Var_DeploymentStruct.telemetry_profile_uuid_list.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> telemetry_profile_uuid_listArray;
		for(auto val: Var_DeploymentStruct.telemetry_profile_uuid_list)
		{
			telemetry_profile_uuid_listArray.Add(MakeShareable(new FJsonValueString(val)));
		}
		JsonObject->SetArrayField("telemetry_profile_uuid_list", telemetry_profile_uuid_listArray);
	}
	if(Var_DeploymentStruct.DeploymentVariables.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> DeploymentVariablesArray;
		for(auto val: Var_DeploymentStruct.DeploymentVariables)
		{
			TSharedPtr<FJsonObject> DeploymentVariablesValue = MakeShareable(new FJsonObject);
			DeploymentVariablesValue->SetStringField("key", val.Key);
			DeploymentVariablesValue->SetStringField("value", val.Value);
			DeploymentVariablesValue->SetBoolField("is_hidden", val.bIsHidden);
			DeploymentVariablesArray.Add(MakeShareable(new FJsonValueObject(DeploymentVariablesValue)));
		}
		JsonObject->SetArrayField("env_vars", DeploymentVariablesArray);
	}
	JsonObject->SetBoolField("skip_telemetry", Var_DeploymentStruct.bSkipTelemetry);
	if(Var_DeploymentStruct.Location.Latitude != 0 || Var_DeploymentStruct.Location.Longitude != 0)
	{
		TSharedPtr<FJsonObject> LocationObject = MakeShareable(new FJsonObject);
		LocationObject->SetNumberField("latitude", Var_DeploymentStruct.Location.Latitude);
		LocationObject->SetNumberField("longitude", Var_DeploymentStruct.Location.Longitude);
		JsonObject->SetObjectField("location", LocationObject);
	}
	if(Var_DeploymentStruct.WebHookURL.Len() > 0)
	{
		JsonObject->SetStringField("webhook_url", Var_DeploymentStruct.WebHookURL);
	}
	if(Var_DeploymentStruct.Tags.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> TagsArray;
		for(auto val: Var_DeploymentStruct.Tags)
		{
			TagsArray.Add(MakeShareable(new FJsonValueString(val)));
		}
		JsonObject->SetArrayField("tags", TagsArray);
	}
	if(Var_DeploymentStruct.ContainerLogStorage.bEnabled)
	{
		TSharedPtr<FJsonObject> ContainerLogStorageObject = MakeShareable(new FJsonObject);
		ContainerLogStorageObject->SetBoolField("enabled", Var_DeploymentStruct.ContainerLogStorage.bEnabled);
		ContainerLogStorageObject->SetStringField("endpoint_storage", Var_DeploymentStruct.ContainerLogStorage.EndpointStorageName);
		JsonObject->SetObjectField("container_log_storage", ContainerLogStorageObject);
	}
	if(Var_DeploymentStruct.Filters.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> FiltersArray;
		for(auto val: Var_DeploymentStruct.Filters)
		{
			FiltersArray.Add(MakeShareable(new FJsonValueObject(val.ToJsonObject())));
		}
		JsonObject->SetArrayField("filters", FiltersArray);
	}
	switch (Var_DeploymentStruct.SortStrategy)
	{
	case EGIK_Basic:
		JsonObject->SetStringField("ap_sort_strategy", "basic");
		break;
	case EGIK_Weighted:
		JsonObject->SetStringField("ap_sort_strategy", "weighted");
		break;
	default: break;
	}
	if(Var_DeploymentStruct.ContainerCommand.Len() > 0)
	{
		JsonObject->SetStringField("command", Var_DeploymentStruct.ContainerCommand);
	}
	if(Var_DeploymentStruct.ContainerArguments.Len() > 0)
	{
		JsonObject->SetStringField("arguments", Var_DeploymentStruct.ContainerArguments);
	}
	FString Content;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(Content);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_CreateDeployment::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_CreateDeploymentResponse(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}
