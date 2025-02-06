// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EGIK_ListAllDeployments.h"

#include "GenericPlatform/GenericPlatformHttp.h"

UEGIK_ListAllDeployments* UEGIK_ListAllDeployments::ListAllDeployments(const FString& Query)
{
	UEGIK_ListAllDeployments* Node = NewObject<UEGIK_ListAllDeployments>();
	Node->Var_Query = Query;
	return Node;
}

void UEGIK_ListAllDeployments::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
    TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
    if (!HttpResponse.IsValid())
    {
        OnFailure.Broadcast(FEGIK_ListAllDeploymentsResponse(), FEGIK_ErrorStruct(0, "Invalid HTTP response"));
        return;
    }

    if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
    {
        FString ErrorMessage = FString::Printf(TEXT("HTTP Error: %d - %s"),
            HttpResponse->GetResponseCode(),
            *HttpResponse->GetContentAsString());
        OnFailure.Broadcast(FEGIK_ListAllDeploymentsResponse(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), ErrorMessage));
        return;
    }

    // Deserialize JSON response
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        OnFailure.Broadcast(FEGIK_ListAllDeploymentsResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
        return;
    }

    FEGIK_ListAllDeploymentsResponse Response;

    // Parse Pagination
	//Check if the pagination object exists
	if (JsonObject->HasField(TEXT("pagination")))
	{
		Response.Pagination.Number = JsonObject->GetObjectField(TEXT("pagination"))->GetIntegerField(TEXT("number"));
		Response.Pagination.NextPageNumber = JsonObject->GetObjectField(TEXT("pagination"))->GetIntegerField(TEXT("next_page_number"));
		Response.Pagination.PreviousPageNumber = JsonObject->GetObjectField(TEXT("pagination"))->GetIntegerField(TEXT("previous_page_number"));
		if(JsonObject->GetObjectField(TEXT("pagination"))->HasField(TEXT("total_pages")))
		{
			Response.Pagination.TotalPages = JsonObject->GetObjectField(TEXT("pagination"))->GetIntegerField(TEXT("total_pages"));
		}
		else
		{
			Response.Pagination.TotalPages = 0;
		}
		Response.Pagination.bHasNextPage = JsonObject->GetObjectField(TEXT("pagination"))->GetBoolField(TEXT("has_next"));
		Response.Pagination.bHasPreviousPage = JsonObject->GetObjectField(TEXT("pagination"))->GetBoolField(TEXT("has_previous"));
	}

    // Parse Total Count
    if (JsonObject->HasField(TEXT("total_count")))
    {
        Response.TotalCount = JsonObject->GetIntegerField(TEXT("total_count"));
    }

    // Parse Data Array
    const TArray<TSharedPtr<FJsonValue>>* DataArrayPtr;
    if (JsonObject->TryGetArrayField(TEXT("data"), DataArrayPtr) && DataArrayPtr)
    {
        for (const TSharedPtr<FJsonValue>& val : *DataArrayPtr)
        {
            if (!val.IsValid()) continue;

            FEGIK_DeploymentListData DeploymentData;
            TSharedPtr<FJsonObject> DeploymentObject = val->AsObject();
            if (!DeploymentObject.IsValid()) continue;

            DeploymentData.RequestId = DeploymentObject->GetStringField(TEXT("request_id"));
            DeploymentData.FQDN = DeploymentObject->GetStringField(TEXT("fqdn"));
            DeploymentData.StartTime = DeploymentObject->GetStringField(TEXT("start_time"));
            DeploymentData.bReady = DeploymentObject->GetBoolField(TEXT("ready"));
            DeploymentData.PublicIP = DeploymentObject->GetStringField(TEXT("public_ip"));
        	if(DeploymentObject->HasField(TEXT("sockets")))
        	{
        		DeploymentData.Sockets = DeploymentObject->GetIntegerField(TEXT("sockets"));
        	}
        	if(DeploymentObject->HasField(TEXT("sockets_usage")))
        	{
        		DeploymentData.SocketsUsage = DeploymentObject->GetIntegerField(TEXT("sockets_usage"));
        	}
            DeploymentData.bIsJoinableBySession = DeploymentObject->GetBoolField(TEXT("is_joinable_by_session"));

            // Parse Tags
            const TArray<TSharedPtr<FJsonValue>>* TagsArrayPtr;
            if (DeploymentObject->TryGetArrayField(TEXT("tags"), TagsArrayPtr))
            {
                DeploymentData.Tags = JsonArrayToStringArray(TagsArrayPtr);
            }

            // Parse Ports
        	TArray<FEGIK_DeploymentPorts> Ports;
        	const TSharedPtr<FJsonObject>* PortsObjectPtr;
        	if (DeploymentObject->TryGetObjectField(TEXT("ports"), PortsObjectPtr) && PortsObjectPtr)
        	{
        		TSharedPtr<FJsonObject> PortsObject = *PortsObjectPtr;
        		for (const auto& PortEntry : PortsObject->Values)
        		{
        			FEGIK_DeploymentPorts PortData;
        
        			// Ensure the entry is a JSON object
        			TSharedPtr<FJsonObject> PortObject = PortEntry.Value->AsObject();
        			if (!PortObject.IsValid())
        			{
        				continue;
        			}

        			// Safely extract values using TryGet methods
        			PortObject->TryGetNumberField(TEXT("external"), PortData.External);
        			PortObject->TryGetNumberField(TEXT("internal"), PortData.Internal);
        			PortObject->TryGetStringField(TEXT("protocol"), PortData.Protocol);
        			PortObject->TryGetBoolField(TEXT("tls_upgrade"), PortData.bTLS_Upgrade);
        			PortObject->TryGetStringField(TEXT("link"), PortData.Link);
        			PortObject->TryGetNumberField(TEXT("proxy"), PortData.Proxy);

        			// Add to Ports map
        			DeploymentData.Ports.Add(PortEntry.Key, PortData);
        		}
        	}
            Response.Deployments.Add(DeploymentData);
        }
    }

    // Broadcast success
    OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}


void UEGIK_ListAllDeployments::Activate()
{
	Super::Activate();
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "application/json");
	FString URL = "https://api.edgegap.com/v1/deployments";
	if (Var_Query.Len() > 0)
	{
		URL += "?query=" + FGenericPlatformHttp::UrlEncode(Var_Query);
	}
	Request->SetURL(URL);
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_ListAllDeployments::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_ListAllDeploymentsResponse(), FEGIK_ErrorStruct(0, "Failed to connect, likely the EdgeGap Server is down or this URL has been deprecated"));
	}
}
