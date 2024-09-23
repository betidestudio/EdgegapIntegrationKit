// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIK_CreateRelaySession.h"

UEGIK_CreateRelaySession* UEGIK_CreateRelaySession::CreateRelaySession(FEGIK_CreateRelaySessionInput Input)
{
	UEGIK_CreateRelaySession* Node = NewObject<UEGIK_CreateRelaySession>();
	Node->Var_Input = Input;
	return Node;
}

void UEGIK_CreateRelaySession::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL("https://api.edgegap.com/v1/relays/sessions");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> UsersArray;
	for (int i = 0; i < Var_Input.Users.Num(); i++)
	{
		TSharedPtr<FJsonObject> UserObject = MakeShareable(new FJsonObject);
		UserObject->SetStringField("ip", Var_Input.Users[i]);
		UsersArray.Add(MakeShareable(new FJsonValueObject(UserObject)));
	}
	JsonObject->SetArrayField("users", UsersArray);
	TArray<TSharedPtr<FJsonValue>> FiltersArray;
	for (int i = 0; i < Var_Input.Filters.Num(); i++)
	{
		TSharedPtr<FJsonObject> FilterObject = MakeShareable(new FJsonObject);
		FilterObject->SetStringField("field", Var_Input.Filters[i].Field);
		TArray<TSharedPtr<FJsonValue>> ValuesArray;
		for (int j = 0; j < Var_Input.Filters[i].Values.Num(); j++)
		{
			TSharedPtr<FJsonValue> Value = MakeShareable(new FJsonValueString(Var_Input.Filters[i].Values[j]));
			ValuesArray.Add(Value);
		}
		FilterObject->SetArrayField("values", ValuesArray);
		switch (Var_Input.Filters[i].FilterType)
		{
		case EGIK_Any:
			FilterObject->SetStringField("filter_type", "any");
			break;
		case EGIK_All:
			FilterObject->SetStringField("filter_type", "all");
			break;
		case EGIK_Not:
			FilterObject->SetStringField("filter_type", "not");
			break;
		default: ;
		}
		
		TSharedPtr<FJsonValueObject> FilterValue = MakeShareable(new FJsonValueObject(FilterObject));
		FiltersArray.Add(FilterValue);
	}
	JsonObject->SetArrayField("filters", FiltersArray);
	JsonObject->SetStringField("webhook_url", Var_Input.WebhookUrl);
	FString Content;
	TSharedRef<TJsonWriter<TCHAR>> Writer = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(Content);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_CreateRelaySession::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}

void UEGIK_CreateRelaySession::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
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
				OnSuccess.Broadcast(JsonObject, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}
