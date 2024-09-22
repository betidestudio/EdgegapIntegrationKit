// Fill out your copyright notice in the Description page of Project Settings.


#include "EGIK_DeleteDeployment.h"

UEGIK_DeleteDeployment* UEGIK_DeleteDeployment::DeleteDeployment(FString bRequestId, FString ContainerLogStorage)
{
	UEGIK_DeleteDeployment* UEGIK_DeleteDeploymentObject = NewObject<UEGIK_DeleteDeployment>();
	UEGIK_DeleteDeploymentObject->Var_bRequestId = bRequestId;
	UEGIK_DeleteDeploymentObject->Var_ContainerLogStorage = ContainerLogStorage;
	return UEGIK_DeleteDeploymentObject;
}

void UEGIK_DeleteDeployment::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
                                                TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	if (HttpResponse.IsValid())
	{
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if(FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				FEGIK_DeleteDeploymentResponse Response;
				Response.Message = JsonObject->GetStringField(TEXT("message"));
				if(JsonObject->HasField(TEXT("deployment_summary")))
				{
					Response.DeploymentStatusAndInfo = JsonObject->GetObjectField(TEXT("deployment_summary"));
				}
				OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_DeleteDeploymentResponse(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_DeleteDeploymentResponse(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_DeleteDeploymentResponse(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UEGIK_DeleteDeployment::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("DELETE");
	Request->SetURL("https://api.edgegap.com/v1/stop/" + Var_bRequestId);
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", "token 3d6e71e0-5717-4e8b-959d-b374d004be73");
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_DeleteDeployment::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_DeleteDeploymentResponse(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}