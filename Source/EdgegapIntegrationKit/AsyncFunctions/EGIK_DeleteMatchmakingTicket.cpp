// Copyright (c) 2024 Betide Studio. All Rights Reserved.


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
		const int32 ResponseCode = HttpResponse->GetResponseCode();
		
		if(EHttpResponseCodes::IsOk(ResponseCode))
		{
			// 204 No Content indicates successful deletion
			OnSuccess.Broadcast(FEGIK_ErrorStruct());
		}
		else if(ResponseCode == 429)
		{
			// 429 Too Many Requests - Rate limiting response
			OnRateLimited.Broadcast(FEGIK_ErrorStruct(429, HttpResponse->GetContentAsString()));
		}
		else if(ResponseCode == 409)
		{
			// 409 Conflict indicates the ticket cannot be deleted because deployment is starting
			// This is expected behavior in the updated API
			FString ErrorMessage = "Cannot delete ticket as deployment is starting. To replace leavers, use a Backfill issued by the server.";
			OnFailure.Broadcast(FEGIK_ErrorStruct(409, ErrorMessage));
		}
		else
		{
			// Any other error
			OnFailure.Broadcast(FEGIK_ErrorStruct(ResponseCode, HttpResponse->GetContentAsString()));
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
	Request->SetHeader("Authorization", Var_Request.AuthToken);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_DeleteMatchmakingTicket::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}
