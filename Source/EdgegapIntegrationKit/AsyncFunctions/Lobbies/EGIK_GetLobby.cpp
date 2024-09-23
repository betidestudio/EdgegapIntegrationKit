// Fill out your copyright notice in the Description page of Project Settings.


#include "EGIK_GetLobby.h"

UEGIK_GetLobby* UEGIK_GetLobby::GetLobby(FString LobbyName)
{
	UEGIK_GetLobby* Node = NewObject<UEGIK_GetLobby>();
	Node->Var_LobbyName = LobbyName;
	return Node;
}

void UEGIK_GetLobby::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL("https://api.edgegap.com/v1/lobbies/" + Var_LobbyName);
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	Request->SetHeader("Content-Type", "application/json");
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_GetLobby::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_LobbyInfo(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}

void UEGIK_GetLobby::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse,
	bool bArg)
{
	if (HttpResponse.IsValid())
	{
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(HttpResponse->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				FEGIK_LobbyInfo Lobby;
				Lobby.Name = JsonObject->GetStringField(TEXT("name"));
				Lobby.Url = JsonObject->GetStringField(TEXT("url"));
				Lobby.Status = JsonObject->GetStringField(TEXT("status"));
				OnSuccess.Broadcast(Lobby, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_LobbyInfo(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_LobbyInfo(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_LobbyInfo(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}
