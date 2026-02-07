// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetBackFillTicketInformation.h"

UEGIK_GetBackFillTicketInformation* UEGIK_GetBackFillTicketInformation::GetBackFillTicketInformation(const FString& backfillId, const FString& MatchmakingURL, const FString& AuthToken)
{
	UEGIK_GetBackFillTicketInformation* Node = NewObject<UEGIK_GetBackFillTicketInformation>();
	Node->Var_BackfillId = backfillId;
	Node->Var_MatchmakingURL = MatchmakingURL;
	Node->Var_AuthToken = AuthToken;
	return Node;
}

FString UEGIK_GetBackFillTicketInformation::GetEndpointURL() const
{
	FString BaseURL = Var_MatchmakingURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/backfills/%s"), *BaseURL, *Var_BackfillId);
}

EEGIK_HttpVerb UEGIK_GetBackFillTicketInformation::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_GetBackFillTicketInformation::GetAuthorizationHeader() const
{
	return Var_AuthToken;
}

void UEGIK_GetBackFillTicketInformation::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_GetBackFillTicketInformationResponse Response;
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

void UEGIK_GetBackFillTicketInformation::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_GetBackFillTicketInformationResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_GetBackFillTicketInformation::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_GetBackFillTicketInformationResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
