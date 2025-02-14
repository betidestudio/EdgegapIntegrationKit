// Copyright (c) Betide Studio. All Rights Reserved.


#include "EGIK_DeleteBackFillTicket.h"

#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_DeleteBackFillTicket* UEGIK_DeleteBackFillTicket::DeleteBackFillTicket(const FString& backfillId,
                                                                             const FString& MatchmakingURL, const FString& AuthToken)
{
	UEGIK_DeleteBackFillTicket* Node = NewObject<UEGIK_DeleteBackFillTicket>();
	Node->Var_BackfillId = backfillId;
	Node->Var_MatchmakingURL = MatchmakingURL;
	Node->Var_AuthToken = AuthToken;
	return Node;
}

void UEGIK_DeleteBackFillTicket::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	FEGIK_ErrorStruct Error;
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
		OnFailure.Broadcast(FEGIK_ErrorStruct(0, "Failed to parse JSON"));
	}
}

void UEGIK_DeleteBackFillTicket::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("DELETE");
	Request->SetURL(Var_MatchmakingURL + "/backfills/" + Var_BackfillId);
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", Var_AuthToken);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_DeleteBackFillTicket::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_ErrorStruct(0, "Failed to connect, likely the Matchmaker is down or the URL is incorrect or is not released"));
	}
}
