// Copyright (c) Betide Studio. All Rights Reserved.


#include "EGIK_CreateBackfill.h"

#include "EGIKBlueprintFunctionLibrary.h"
#include "HttpModule.h"

UEGIK_CreateBackfill* UEGIK_CreateBackfill::CreateBackFillRequest(const FEGIK_CreateBackFillRequest& Request)
{
	UEGIK_CreateBackfill* Node = NewObject<UEGIK_CreateBackfill>();
	Node->Var_Request = Request;
	return Node;
}

void UEGIK_CreateBackfill::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	FEGIK_CreateBackFillResponse Response;
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
						Response.Tickets.Add(TicketPair.Key, TicketPair.Value->AsString());
					}
				}

				if (JsonObject->HasField(TEXT("assigned_ticket")))
				{
					Response.AssignedTicket = JsonObject->GetStringField(TEXT("assigned_ticket"));
				}

				OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UEGIK_CreateBackfill::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL(Var_Request.MatchmakingURL + "/backfills");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", Var_Request.AuthToken);
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("profile", Var_Request.Profile);
	TSharedPtr<FJsonObject> AttributesObject = MakeShareable(new FJsonObject);
	AttributesObject->SetStringField("deployment_request_id", Var_Request.DeploymentRequestID);
	JsonObject->SetObjectField("attributes", AttributesObject);
	TSharedPtr<FJsonObject> TicketsObject = MakeShareable(new FJsonObject);
	for (const auto& Ticket : Var_Request.Tickets)
	{
		TSharedPtr<FJsonObject> TicketJson;
		TSharedRef<TJsonReader<>> TicketReader = TJsonReaderFactory<>::Create(Ticket.Value);
    
		if (FJsonSerializer::Deserialize(TicketReader, TicketJson) && TicketJson.IsValid())
		{
			TicketJson->SetStringField("id", Ticket.Key);
			TicketsObject->SetObjectField(Ticket.Key, TicketJson);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to deserialize ticket JSON: %s"), *Ticket.Value);
		}
	}
	JsonObject->SetObjectField("tickets", TicketsObject);
	
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(JsonString);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_CreateBackfill::OnResponseReceived);
	UE_LOG(LogTemp, Warning, TEXT("Original Request: %s"), *JsonString);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}
