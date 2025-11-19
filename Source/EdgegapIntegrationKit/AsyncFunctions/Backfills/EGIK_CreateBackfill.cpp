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
		const int32 ResponseCode = HttpResponse->GetResponseCode();
		if (EHttpResponseCodes::IsOk(ResponseCode))
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

				if (JsonObject->HasField(TEXT("assigned_ticket")))
				{
					Response.AssignedTicket = JsonObject->GetObjectField(TEXT("assigned_ticket"));
				}

				// Parse assignment details from attributes.assignment
				const TSharedPtr<FJsonObject>* AttributesObject;
				if (JsonObject->TryGetObjectField(TEXT("attributes"), AttributesObject))
				{
					const TSharedPtr<FJsonObject>* AssignmentObject;
					if ((*AttributesObject)->HasTypedField<EJson::Object>(TEXT("assignment")))
					{
						if ((*AttributesObject)->TryGetObjectField(TEXT("assignment"), AssignmentObject))
						{
							// Store raw JSON string for flexible handling of variable structures
							FString AssignmentJsonString;
							TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&AssignmentJsonString);
							FJsonSerializer::Serialize((*AssignmentObject).ToSharedRef(), Writer);
							Response.AssignmentDetailsJson = AssignmentJsonString;
							
							// Try to parse into structured format (will gracefully handle missing fields)
							Response.Assignment = FEGIK_AssignmentStruct(*AssignmentObject);
						}
						else
						{
							Response.Assignment = FEGIK_AssignmentStruct("null");
							Response.AssignmentDetailsJson = "";
						}
					}
					else
					{
						Response.Assignment = FEGIK_AssignmentStruct("null");
						Response.AssignmentDetailsJson = "";
					}
				}
				else
				{
					Response.Assignment = FEGIK_AssignmentStruct("null");
					Response.AssignmentDetailsJson = "";
				}

				OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
			}
		}
		else if(ResponseCode == 429)
		{
			// 429 Too Many Requests - Rate limiting response
			OnRateLimited.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(429, HttpResponse->GetContentAsString()));
		}
		else
		{
			OnFailure.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(ResponseCode, HttpResponse->GetContentAsString()));
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
	
	// Create attributes object with updated structure (attributes.assignment.request_id)
	TSharedPtr<FJsonObject> AttributesObject = MakeShareable(new FJsonObject);
	TSharedPtr<FJsonObject> AssignmentObject = MakeShareable(new FJsonObject);
	AssignmentObject->SetStringField("request_id", Var_Request.DeploymentRequestID);
	
	// Add assignment details if provided
	if (!Var_Request.AssignmentDetails.IsEmpty())
	{
		TSharedPtr<FJsonObject> FullAssignmentJson;
		TSharedRef<TJsonReader<>> AssignmentReader = TJsonReaderFactory<>::Create(Var_Request.AssignmentDetails);
		if (FJsonSerializer::Deserialize(AssignmentReader, FullAssignmentJson) && FullAssignmentJson.IsValid())
		{
			// Merge additional assignment details with request_id
			for (const auto& Field : FullAssignmentJson->Values)
			{
				if (Field.Key != "request_id") // Don't overwrite request_id
				{
					AssignmentObject->SetField(Field.Key, Field.Value);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to deserialize assignment details JSON: %s"), *Var_Request.AssignmentDetails);
		}
	}
	
	AttributesObject->SetObjectField("assignment", AssignmentObject);
	JsonObject->SetObjectField("attributes", AttributesObject);
	
	// Process tickets
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
