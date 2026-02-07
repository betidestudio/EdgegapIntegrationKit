// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_ListRelaySessions.h"

UEGIK_ListRelaySessions* UEGIK_ListRelaySessions::ListRelaySessions(int32 PageNumber)
{
	UEGIK_ListRelaySessions* Node = NewObject<UEGIK_ListRelaySessions>();
	Node->Var_PageNumber = PageNumber;
	return Node;
}

FString UEGIK_ListRelaySessions::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/relays/sessions");
}

EEGIK_HttpVerb UEGIK_ListRelaySessions::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_ListRelaySessions::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (JsonObject.IsValid())
	{
		FEGIK_ListRelaySessionsOutput Output;
		Output.RelaySessions.Empty();
		TArray<TSharedPtr<FJsonValue>> RelaySessionsArray = JsonObject->GetArrayField(TEXT("sessions"));
		for (auto RelaySessionValue : RelaySessionsArray)
		{
			Output.RelaySessions.Add(RelaySessionValue->AsObject());
		}
		TSharedPtr<FJsonObject> PaginationObject = JsonObject->GetObjectField(TEXT("pagination"));
		Output.Pagination.Number = PaginationObject->GetIntegerField(TEXT("number"));
		Output.Pagination.NextPageNumber = PaginationObject->GetIntegerField(TEXT("next_page_number"));
		Output.Pagination.PreviousPageNumber = PaginationObject->GetIntegerField(TEXT("previous_page_number"));
		Output.Pagination.TotalPages = PaginationObject->GetObjectField(TEXT("paginator"))->GetIntegerField(TEXT("num_pages"));
		Output.Pagination.bHasNextPage = PaginationObject->GetBoolField(TEXT("has_next"));
		Output.Pagination.bHasPreviousPage = PaginationObject->GetBoolField(TEXT("has_previous"));
		OnSuccess.Broadcast(Output, FEGIK_ErrorStruct());
	}
	else
	{
		OnFailure.Broadcast(FEGIK_ListRelaySessionsOutput(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
	}
}

void UEGIK_ListRelaySessions::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ListRelaySessionsOutput(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
