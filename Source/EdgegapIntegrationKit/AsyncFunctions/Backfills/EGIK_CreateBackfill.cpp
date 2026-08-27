// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateBackfill.h"
#include "EGIK_JsonKeyCompat.h"
#include "EGIK_MatchmakingRequestUtils.h"

namespace EGIK_CreateBackfillRequest
{
	static bool ParseJsonObject(
		const FString& Source,
		const FString& FieldName,
		TSharedPtr<FJsonObject>& OutObject,
		FString& OutErrorMessage)
	{
		FString Json = Source;
		Json.TrimStartAndEndInline();
		if (Json.IsEmpty())
		{
			OutErrorMessage = FString::Printf(TEXT("%s must be a non-empty JSON object."), *FieldName);
			return false;
		}

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		if (!FJsonSerializer::Deserialize(Reader, OutObject) || !OutObject.IsValid())
		{
			OutErrorMessage = FString::Printf(
				TEXT("%s must be a valid JSON object. Parser error: %s"),
				*FieldName,
				*Reader->GetErrorMessage());
			return false;
		}
		return true;
	}
}

UEGIK_CreateBackfill* UEGIK_CreateBackfill::CreateBackFillRequest(const FEGIK_CreateBackFillRequest& Request)
{
	UEGIK_CreateBackfill* Node = NewObject<UEGIK_CreateBackfill>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_CreateBackfill::GetEndpointURL() const
{
	return EGIK_MatchmakingRequest::ResolveURL(Var_Request.MatchmakingURL) + TEXT("/backfills");
}

EEGIK_HttpVerb UEGIK_CreateBackfill::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_CreateBackfill::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_Request.AuthToken);
}

bool UEGIK_CreateBackfill::ValidateRequest(FString& OutErrorMessage) const
{
	if (!EGIK_MatchmakingRequest::ValidateConnection(Var_Request.MatchmakingURL, Var_Request.AuthToken, OutErrorMessage)
		|| !EGIK_MatchmakingRequest::ValidateProfile(Var_Request.Profile, OutErrorMessage)
		|| !EGIK_MatchmakingRequest::ValidateResourceId(TEXT("DeploymentRequestID"), Var_Request.DeploymentRequestID, OutErrorMessage))
	{
		return false;
	}

	TSharedPtr<FJsonObject> AssignmentObject;
	if (!EGIK_CreateBackfillRequest::ParseJsonObject(
		Var_Request.AssignmentDetails,
		TEXT("AssignmentDetails"),
		AssignmentObject,
		OutErrorMessage))
	{
		return false;
	}

	FString FQDN;
	FString PublicIP;
	const TSharedPtr<FJsonObject>* PortsObject = nullptr;
	const TSharedPtr<FJsonObject>* LocationObject = nullptr;
	if (!AssignmentObject->TryGetStringField(TEXT("fqdn"), FQDN) || FQDN.IsEmpty()
		|| !AssignmentObject->TryGetStringField(TEXT("public_ip"), PublicIP) || PublicIP.IsEmpty()
		|| !AssignmentObject->TryGetObjectField(TEXT("ports"), PortsObject) || !PortsObject || !(*PortsObject).IsValid() || (*PortsObject)->Values.IsEmpty()
		|| !AssignmentObject->TryGetObjectField(TEXT("location"), LocationObject) || !LocationObject || !(*LocationObject).IsValid())
	{
		OutErrorMessage = TEXT("AssignmentDetails must contain non-empty fqdn, public_ip, ports, and location fields from the active Edgegap deployment assignment.");
		return false;
	}

	FString AttributesJson = Var_Request.AttributesOverride;
	AttributesJson.TrimStartAndEndInline();
	if (!AttributesJson.IsEmpty())
	{
		TSharedPtr<FJsonObject> AttributesObject;
		if (!EGIK_CreateBackfillRequest::ParseJsonObject(
			AttributesJson,
			TEXT("AttributesOverride"),
			AttributesObject,
			OutErrorMessage))
		{
			return false;
		}
		if (AttributesObject->HasField(TEXT("assignment")))
		{
			OutErrorMessage = TEXT("AttributesOverride cannot contain the reserved 'assignment' key. Use AssignmentDetails and DeploymentRequestID instead.");
			return false;
		}
	}

	for (const TPair<FString, FString>& Ticket : Var_Request.Tickets)
	{
		FString TicketError;
		if (!EGIK_MatchmakingRequest::ValidateResourceId(TEXT("Ticket ID"), Ticket.Key, TicketError))
		{
			OutErrorMessage = FString::Printf(TEXT("Tickets[%s]: %s"), *Ticket.Key, *TicketError);
			return false;
		}

		TSharedPtr<FJsonObject> TicketObject;
		if (!EGIK_CreateBackfillRequest::ParseJsonObject(
			Ticket.Value,
			FString::Printf(TEXT("Tickets[%s]"), *Ticket.Key),
			TicketObject,
			OutErrorMessage))
		{
			return false;
		}
	}

	return true;
}

TSharedPtr<FJsonObject> UEGIK_CreateBackfill::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("profile"), Var_Request.Profile);

	TSharedPtr<FJsonObject> AttributesObject = MakeShareable(new FJsonObject);
	FString AttributesJson = Var_Request.AttributesOverride;
	AttributesJson.TrimStartAndEndInline();
	if (!AttributesJson.IsEmpty())
	{
		FString IgnoredError;
		EGIK_CreateBackfillRequest::ParseJsonObject(
			AttributesJson,
			TEXT("AttributesOverride"),
			AttributesObject,
			IgnoredError);
	}

	TSharedPtr<FJsonObject> AssignmentObject = MakeShareable(new FJsonObject);
	FString IgnoredAssignmentError;
	TSharedPtr<FJsonObject> ParsedAssignmentObject;
	if (EGIK_CreateBackfillRequest::ParseJsonObject(
		Var_Request.AssignmentDetails,
		TEXT("AssignmentDetails"),
		ParsedAssignmentObject,
		IgnoredAssignmentError))
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Field : ParsedAssignmentObject->Values)
		{
			if (Field.Key != TEXT("request_id"))
			{
				AssignmentObject->SetField(Field.Key, Field.Value);
			}
		}
	}
	AssignmentObject->SetStringField(TEXT("request_id"), Var_Request.DeploymentRequestID);

	AttributesObject->SetObjectField(TEXT("assignment"), AssignmentObject);
	JsonObject->SetObjectField(TEXT("attributes"), AttributesObject);

	TSharedPtr<FJsonObject> TicketsObject = MakeShareable(new FJsonObject);
	for (const TPair<FString, FString>& Ticket : Var_Request.Tickets)
	{
		TSharedPtr<FJsonObject> TicketJson;
		FString IgnoredTicketError;
		if (EGIK_CreateBackfillRequest::ParseJsonObject(
			Ticket.Value,
			FString::Printf(TEXT("Tickets[%s]"), *Ticket.Key),
			TicketJson,
			IgnoredTicketError))
		{
			TicketJson->SetStringField(TEXT("id"), Ticket.Key);
			TicketsObject->SetObjectField(Ticket.Key, TicketJson);
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
	if (!JsonObject->TryGetStringField(TEXT("id"), Response.Id)
		|| !JsonObject->TryGetStringField(TEXT("profile"), Response.Profile)
		|| !JsonObject->TryGetStringField(TEXT("status"), Response.Status))
	{
		HandleError(0, TEXT("Edgegap returned a successful Create Backfill response without required id, profile, or status fields."));
		return;
	}

	const TSharedPtr<FJsonObject>* TicketsObject = nullptr;
	if (JsonObject->TryGetObjectField(TEXT("tickets"), TicketsObject) && TicketsObject && (*TicketsObject).IsValid())
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& TicketPair : (*TicketsObject)->Values)
		{
			if (!TicketPair.Value.IsValid() || TicketPair.Value->Type != EJson::Object)
			{
				HandleError(0, FString::Printf(TEXT("Create Backfill response ticket '%s' was not a JSON object."), *TicketPair.Key));
				return;
			}
			const TSharedPtr<FJsonObject> TicketObject = TicketPair.Value->AsObject();
			if (!TicketObject.IsValid())
			{
				HandleError(0, FString::Printf(TEXT("Create Backfill response ticket '%s' was not a JSON object."), *TicketPair.Key));
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

void UEGIK_CreateBackfill::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_CreateBackfill::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(FEGIK_CreateBackFillResponse(), FEGIK_ErrorStruct(429, ResponseContent));
}
