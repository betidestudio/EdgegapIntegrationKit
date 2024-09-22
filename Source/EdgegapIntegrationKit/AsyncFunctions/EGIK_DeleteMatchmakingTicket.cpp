// Fill out your copyright notice in the Description page of Project Settings.


#include "EGIK_DeleteMatchmakingTicket.h"

UEGIK_DeleteMatchmakingTicket* UEGIK_DeleteMatchmakingTicket::DeleteMatchmakingTicket(
	FEGIK_DeleteMatchmakingRequest Request)
{
	UEGIK_DeleteMatchmakingTicket* BlueprintNode = NewObject<UEGIK_DeleteMatchmakingTicket>();
	BlueprintNode->Var_Request = Request;
	return BlueprintNode;
}

void UEGIK_DeleteMatchmakingTicket::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	if(HttpResponse.IsValid())
	{
		if(EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
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
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UEGIK_DeleteMatchmakingTicket::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("DELETE");
	Request->SetURL(Var_Request.MatchmakingURL + "/tickets/" + Var_Request.TicketId);
	Request->SetHeader("Content-Type", "application/json");
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_DeleteMatchmakingTicket::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}
