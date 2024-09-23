// Fill out your copyright notice in the Description page of Project Settings.


#include "EGIK_DeleteRelaySession.h"

UEGIK_DeleteRelaySession* UEGIK_DeleteRelaySession::DeleteRelaySession(FString SessionId)
{
	UEGIK_DeleteRelaySession* Node = NewObject<UEGIK_DeleteRelaySession>();
	Node->Var_SessionId = SessionId;
	return Node;
}

void UEGIK_DeleteRelaySession::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("DELETE");
	Request->SetURL("https://api.edgegap.com/v1/relays/sessions/" + Var_SessionId);
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	Request->SetHeader("Content-Type", "application/json");
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_DeleteRelaySession::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}

void UEGIK_DeleteRelaySession::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	if (HttpResponse.IsValid())
	{
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			OnSuccess.Broadcast(FEGIK_ErrorStruct());
		}
		else
		{
			OnFailure.Broadcast(FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_ErrorStruct(0, "Failed to process request"));
	}
}
