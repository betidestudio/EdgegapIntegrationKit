// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIK_DeploymentStatusAndInfo.h"

UEGIK_DeploymentStatusAndInfo* UEGIK_DeploymentStatusAndInfo::GetDeploymentStatusAndInfo(const FString& RequestId)
{
	UEGIK_DeploymentStatusAndInfo* Node = NewObject<UEGIK_DeploymentStatusAndInfo>();
	Node->Var_RequestId = RequestId;
	return Node;
}

void UEGIK_DeploymentStatusAndInfo::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL("https://api.edgegap.com/v1/status/" + Var_RequestId);
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_DeploymentStatusAndInfo::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_DeploymentStatusAndInfoResponse(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}

void UEGIK_DeploymentStatusAndInfo::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	if(HttpResponse.IsValid())
	{
		if(EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if(FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				FEGIK_DeploymentStatusAndInfoResponse Response;
				Response.RequestId = JsonObject->GetStringField(TEXT("request_id"));
				Response.FQDN = JsonObject->GetStringField(TEXT("fqdn"));
				Response.AppName = JsonObject->GetStringField(TEXT("app_name"));
				Response.VersionName = JsonObject->GetStringField(TEXT("app_version"));
				Response.CurrentStatus = JsonObject->GetStringField(TEXT("current_status"));
				Response.bRunning = JsonObject->GetBoolField(TEXT("running"));
				Response.bWhitelistingActive = JsonObject->GetBoolField(TEXT("whitelisting_active"));
				FDateTime::Parse(JsonObject->GetStringField(TEXT("start_time")),Response.StartTime);
				FDateTime::Parse(JsonObject->GetStringField(TEXT("removal_time")),Response.RemovalTime);
				Response.ElapsedTime = JsonObject->GetIntegerField(TEXT("elapsed_time"));
				Response.LastStatus = JsonObject->GetStringField(TEXT("last_status"));
				Response.bError = JsonObject->GetBoolField(TEXT("error"));
				if(JsonObject->HasField(TEXT("error_detail")))
				{
					Response.ErrorDetails = JsonObject->GetStringField(TEXT("error_detail"));
				}
				TSharedPtr<FJsonObject> PortsObject = JsonObject->GetObjectField(TEXT("ports"));
				for (auto& PortEntry : PortsObject->Values)
				{
					FEGIK_DeploymentPorts PortData;
					TSharedPtr<FJsonObject> PortObject = PortEntry.Value->AsObject();
					PortData.External = PortObject->GetIntegerField(TEXT("external"));
					PortData.Internal = PortObject->GetIntegerField(TEXT("internal"));
					PortData.Protocol = PortObject->GetStringField(TEXT("protocol"));
					PortData.bTLS_Upgrade = PortObject->GetBoolField(TEXT("tls_upgrade"));
					PortData.Link = PortObject->GetStringField(TEXT("link"));
					PortData.Proxy = PortObject->GetIntegerField(TEXT("proxy"));

					Response.Ports.Add(PortEntry.Key, PortData);
				}

				Response.PublicIP = JsonObject->GetStringField(TEXT("public_ip"));
				if(JsonObject->HasField(TEXT("sessions")))
				{
					TArray<TSharedPtr<FJsonValue>> SessionsArray = JsonObject->GetArrayField(TEXT("sessions"));
					for (auto& SessionValue : SessionsArray)
					{
						TSharedPtr<FJsonObject> SessionObject = SessionValue->AsObject();
						FEGIK_Session SessionData;
						SessionData.SessionId = SessionObject->GetStringField(TEXT("session_id"));
						SessionData.Status = SessionObject->GetStringField(TEXT("status"));
						SessionData.bReady = SessionObject->GetBoolField(TEXT("ready"));
						SessionData.bLinked = SessionObject->GetBoolField(TEXT("linked"));
						SessionData.Kind = SessionObject->GetStringField(TEXT("kind"));
						SessionData.UserCount = SessionObject->GetIntegerField(TEXT("user_count"));

						Response.ActiveSessions.Add(SessionData);
					}
				}
				// Parsing location data
				TSharedPtr<FJsonObject> LocationObject = JsonObject->GetObjectField(TEXT("location"));
				Response.Location.Location.City = LocationObject->GetStringField(TEXT("city"));
				Response.Location.Location.Country = LocationObject->GetStringField(TEXT("country"));
				Response.Location.Location.Continent = LocationObject->GetStringField(TEXT("continent"));
				Response.Location.Location.AdministrativeDivision = LocationObject->GetStringField(TEXT("administrative_division"));
				Response.Location.Location.Timezone = LocationObject->GetStringField(TEXT("timezone"));
				Response.Location.LatitudeLongitude.Latitude = LocationObject->GetNumberField(TEXT("latitude"));
				Response.Location.LatitudeLongitude.Longitude = LocationObject->GetNumberField(TEXT("longitude"));

				// Parsing tags
				if(JsonObject->HasField(TEXT("tags")))
				{
					TArray<TSharedPtr<FJsonValue>> TagsArray = JsonObject->GetArrayField(TEXT("tags"));
					for (auto& TagValue : TagsArray)
					{
						Response.Tags.Add(TagValue->AsString());
					}
				}
				// Sockets information
				if(JsonObject->HasField(TEXT("sockets")))
				{
					Response.Sockets = JsonObject->GetIntegerField(TEXT("sockets"));
					Response.SocketsUsage = JsonObject->GetIntegerField(TEXT("sockets_usage"));
				}
				// Command and Arguments
				Response.Command = JsonObject->GetStringField(TEXT("command"));
				Response.Arguments = JsonObject->GetStringField(TEXT("arguments"));
				OnSuccess.Broadcast(Response, FEGIK_ErrorStruct(0, ""));
			}
			else
			{
				OnFailure.Broadcast(FEGIK_DeploymentStatusAndInfoResponse(), FEGIK_ErrorStruct(0, "Failed to deserialize JSON"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_DeploymentStatusAndInfoResponse(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_DeploymentStatusAndInfoResponse(), FEGIK_ErrorStruct(0, "Failed to connect, likely the EdgeGap Server is down or this URL has been deprecated"));
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}