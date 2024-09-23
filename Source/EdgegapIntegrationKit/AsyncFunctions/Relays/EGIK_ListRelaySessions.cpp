// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIK_ListRelaySessions.h"

UEGIK_ListRelaySessions* UEGIK_ListRelaySessions::ListRelaySessions(int32 PageNumber)
{
	UEGIK_ListRelaySessions* Node = NewObject<UEGIK_ListRelaySessions>();
	Node->Var_PageNumber = PageNumber;
	return Node;
}

void UEGIK_ListRelaySessions::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL("https://api.edgegap.com/v1/relays/sessions");
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	Request->SetHeader("Content-Type", "application/json");
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_ListRelaySessions::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_ListRelaySessionsOutput(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}

void UEGIK_ListRelaySessions::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	if (HttpResponse.IsValid())
	{
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(HttpResponse->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				FEGIK_ListRelaySessionsOutput Output;
				Output.RelaySessions.Empty();
				TArray<TSharedPtr<FJsonValue>> RelaySessionsArray = JsonObject->GetArrayField(TEXT("sessions"));
				for(auto RelaySessionValue : RelaySessionsArray)
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
		else
		{
			OnFailure.Broadcast(FEGIK_ListRelaySessionsOutput(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_ListRelaySessionsOutput(), FEGIK_ErrorStruct(0, "Failed to process request"));
	}
}
