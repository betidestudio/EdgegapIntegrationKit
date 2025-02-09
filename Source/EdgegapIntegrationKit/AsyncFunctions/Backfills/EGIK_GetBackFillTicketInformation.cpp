// Copyright (c) Betide Studio. All Rights Reserved.


#include "EGIK_GetBackFillTicketInformation.h"

#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_GetBackFillTicketInformation* UEGIK_GetBackFillTicketInformation::GetBackFillTicketInformation(
	const FString& backfillId, const FString& MatchmakingURL, const FString& AuthToken)
{
	UEGIK_GetBackFillTicketInformation* Node = NewObject<UEGIK_GetBackFillTicketInformation>();
	Node->Var_BackfillId = backfillId;
	Node->Var_MatchmakingURL = MatchmakingURL;
	Node->Var_AuthToken = AuthToken;
	return Node;
}

void UEGIK_GetBackFillTicketInformation::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	FEGIK_GetBackFillTicketInformationResponse Response;
	if (HttpResponse.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Response: %s"), *HttpResponse->GetContentAsString());
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				Response.Id = JsonObject->GetStringField(TEXT("id"));
				Response.Profile = JsonObject->GetStringField(TEXT("profile"));
				Response.Status = JsonObject->GetStringField(TEXT("status"));

				if (JsonObject->HasField(TEXT("tickets")))
				{
					TSharedPtr<FJsonObject> TicketsObject = JsonObject->GetObjectField(TEXT("tickets"));
					for (const auto& TicketPair : TicketsObject->Values)
					{
						Response.Tickets.Add(TicketPair.Key, TicketPair.Value->AsObject());
					}
				}

				if (JsonObject->HasTypedField<EJson::Object>(TEXT("assigned_ticket")))
				{
					//Check if Assigned Ticket is Null
					Response.AssignedTicket = JsonObject->GetObjectField(TEXT("assigned_ticket")); 
				}

				OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_GetBackFillTicketInformationResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_GetBackFillTicketInformationResponse(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_GetBackFillTicketInformationResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
	}
}

void UEGIK_GetBackFillTicketInformation::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(Var_MatchmakingURL + "/backfills/" + Var_BackfillId);
	Request->SetVerb("GET");
	Request->SetHeader("Authorization", Var_AuthToken);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_GetBackFillTicketInformation::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_GetBackFillTicketInformationResponse(), FEGIK_ErrorStruct(0, "Failed to connect, likely the Matchmaker is down or the URL is incorrect or is not released"));
	}
}
