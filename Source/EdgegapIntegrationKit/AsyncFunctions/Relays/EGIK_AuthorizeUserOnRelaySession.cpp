// Fill out your copyright notice in the Description page of Project Settings.


#include "EGIK_AuthorizeUserOnRelaySession.h"

UEGIK_AuthorizeUserOnRelaySession* UEGIK_AuthorizeUserOnRelaySession::AuthorizeUserOnRelaySession(
	FEGIK_AuthorizeUserOnRelaySessionInput Input)
{
	UEGIK_AuthorizeUserOnRelaySession* Node = NewObject<UEGIK_AuthorizeUserOnRelaySession>();
	Node->Var_Input = Input;
	return Node;
}

void UEGIK_AuthorizeUserOnRelaySession::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL("https://api.edgegap.com/v1/relays/sessions:authorize-user");
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	Request->SetHeader("Content-Type", "application/json");
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("session_id", Var_Input.SessionId);
	JsonObject->SetStringField("user_ip", Var_Input.UserIp);
	FString Content;
	TSharedRef<TJsonWriter<TCHAR>> Writer = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(Content);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_AuthorizeUserOnRelaySession::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_RelaySessionInfo(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}

void UEGIK_AuthorizeUserOnRelaySession::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
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
