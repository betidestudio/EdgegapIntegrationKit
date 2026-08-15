// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetBackFillTicketInformation.h"
#include "EGIK_JsonKeyCompat.h"
#include "EGIK_MatchmakingRequestUtils.h"

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
	return FString::Printf(
		TEXT("%s/backfills/%s"),
		*EGIK_MatchmakingRequest::ResolveURL(Var_MatchmakingURL),
		*Var_BackfillId);
}

EEGIK_HttpVerb UEGIK_GetBackFillTicketInformation::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_GetBackFillTicketInformation::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_AuthToken);
}

bool UEGIK_GetBackFillTicketInformation::ValidateRequest(FString& OutErrorMessage) const
{
	return EGIK_MatchmakingRequest::ValidateConnection(Var_MatchmakingURL, Var_AuthToken, OutErrorMessage)
		&& EGIK_MatchmakingRequest::ValidateResourceId(TEXT("BackfillId"), Var_BackfillId, OutErrorMessage);
}

void UEGIK_GetBackFillTicketInformation::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_GetBackFillTicketInformationResponse Response;
	if (!JsonObject->TryGetStringField(TEXT("id"), Response.Id)
		|| !JsonObject->TryGetStringField(TEXT("profile"), Response.Profile)
		|| !JsonObject->TryGetStringField(TEXT("status"), Response.Status))
	{
		HandleError(0, TEXT("Edgegap returned a successful Get Backfill response without required id, profile, or status fields."));
		return;
	}

	const TSharedPtr<FJsonObject>* TicketsObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("tickets"), TicketsObject) && TicketsObject && (*TicketsObject).IsValid())
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& TicketPair : (*TicketsObject)->Values)
		{
			if (!TicketPair.Value.IsValid() || TicketPair.Value->Type != EJson::Object)
			{
				HandleError(0, FString::Printf(TEXT("Get Backfill response ticket '%s' was not a JSON object."), *TicketPair.Key));
				return;
			}
			const TSharedPtr<FJsonObject> TicketObject = TicketPair.Value->AsObject();
			if (!TicketObject.IsValid())
			{
				HandleError(0, FString::Printf(TEXT("Get Backfill response ticket '%s' was not a JSON object."), *TicketPair.Key));
				return;
			}
			Response.Tickets.Add(EGIK_JSONKEY(TicketPair.Key), FEGIK_MatchmakingResponse(TicketObject));
		}
	}

	const TSharedPtr<FJsonObject>* AssignedTicketObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("assigned_ticket"), AssignedTicketObject) && AssignedTicketObject && (*AssignedTicketObject).IsValid())
	{
		Response.AssignedTicket = FEGIK_MatchmakingResponse(*AssignedTicketObject);
	}

	// Parse assignment details from attributes.assignment
	const TSharedPtr<FJsonObject>* AttributesObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("attributes"), AttributesObject) && AttributesObject && (*AttributesObject).IsValid())
	{
		const TSharedPtr<FJsonObject>* AssignmentObject = nullptr;
		if ((*AttributesObject)->HasTypedField<EJson::Object>(TEXT("assignment")))
		{
			if ((*AttributesObject)->TryGetObjectField(TEXT("assignment"), AssignmentObject) && AssignmentObject && (*AssignmentObject).IsValid())
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
