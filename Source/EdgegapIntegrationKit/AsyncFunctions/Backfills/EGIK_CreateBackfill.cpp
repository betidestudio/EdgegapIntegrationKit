// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateBackfill.h"

UEGIK_CreateBackfill* UEGIK_CreateBackfill::CreateBackFillRequest(const FEGIK_CreateBackFillRequest& Request)
{
	UEGIK_CreateBackfill* Node = NewObject<UEGIK_CreateBackfill>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_CreateBackfill::GetEndpointURL() const
{
	FString BaseURL = Var_Request.MatchmakingURL.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
		: Var_Request.MatchmakingURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return BaseURL + TEXT("/backfills");
}

EEGIK_HttpVerb UEGIK_CreateBackfill::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_CreateBackfill::GetAuthorizationHeader() const
{
	return Var_Request.AuthToken.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
		: Var_Request.AuthToken;
}

TSharedPtr<FJsonObject> UEGIK_CreateBackfill::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("profile"), Var_Request.Profile);

	// Create attributes object with updated structure (attributes.assignment.request_id)
	TSharedPtr<FJsonObject> AttributesObject = MakeShareable(new FJsonObject);
	TSharedPtr<FJsonObject> AssignmentObject = MakeShareable(new FJsonObject);
	AssignmentObject->SetStringField(TEXT("request_id"), Var_Request.DeploymentRequestID);

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
				if (Field.Key != TEXT("request_id")) // Don't overwrite request_id
				{
					AssignmentObject->SetField(Field.Key, Field.Value);
				}
			}
		}
		else
		{
			UE_LOG(LogEdgegap, Error, TEXT("[Backfill] Failed to deserialize assignment details JSON: %s"), *Var_Request.AssignmentDetails);
		}
	}

	AttributesObject->SetObjectField(TEXT("assignment"), AssignmentObject);
	JsonObject->SetObjectField(TEXT("attributes"), AttributesObject);

	// Process tickets
	TSharedPtr<FJsonObject> TicketsObject = MakeShareable(new FJsonObject);
	for (const auto& Ticket : Var_Request.Tickets)
	{
		TSharedPtr<FJsonObject> TicketJson;
		TSharedRef<TJsonReader<>> TicketReader = TJsonReaderFactory<>::Create(Ticket.Value);

		if (FJsonSerializer::Deserialize(TicketReader, TicketJson) && TicketJson.IsValid())
		{
			TicketJson->SetStringField(TEXT("id"), Ticket.Key);
			TicketsObject->SetObjectField(Ticket.Key, TicketJson);
		}
		else
		{
			UE_LOG(LogEdgegap, Error, TEXT("[Backfill] Failed to deserialize ticket JSON: %s"), *Ticket.Value);
		}
	}
	JsonObject->SetObjectField(TEXT("tickets"), TicketsObject);

	return JsonObject;
}

void UEGIK_CreateBackfill::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_CreateBackFillResponse Response;
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

				// Try to parse into structured format
				Response.Assignment = FEGIK_AssignmentStruct(*AssignmentObject);
			}
			else
			{
				Response.Assignment = FEGIK_AssignmentStruct("null");
				Response.AssignmentDetailsJson = TEXT("");
			}
		}
		else
		{
			Response.Assignment = FEGIK_AssignmentStruct("null");
			Response.AssignmentDetailsJson = TEXT("");
		}
	}
	else
	{
		Response.Assignment = FEGIK_AssignmentStruct("null");
		Response.AssignmentDetailsJson = TEXT("");
	}

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_CreateBackfill::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_CreateBackfill::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
