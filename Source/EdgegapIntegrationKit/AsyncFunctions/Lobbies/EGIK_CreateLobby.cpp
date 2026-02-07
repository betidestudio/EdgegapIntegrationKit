// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateLobby.h"

UEGIK_CreateLobby* UEGIK_CreateLobby::CreateLobby(FString LobbyName)
{
	UEGIK_CreateLobby* Node = NewObject<UEGIK_CreateLobby>();
	Node->Var_LobbyName = LobbyName;
	return Node;
}

FString UEGIK_CreateLobby::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/lobbies");
}

EEGIK_HttpVerb UEGIK_CreateLobby::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

TSharedPtr<FJsonObject> UEGIK_CreateLobby::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), Var_LobbyName);
	return JsonObject;
}

void UEGIK_CreateLobby::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (JsonObject.IsValid())
	{
		FEGIK_LobbyInfo Lobby;
		Lobby.Name = JsonObject->GetStringField(TEXT("name"));
		Lobby.Url = JsonObject->GetStringField(TEXT("url"));
		Lobby.Status = JsonObject->GetStringField(TEXT("status"));
		OnSuccess.Broadcast(Lobby, FEGIK_ErrorStruct());
	}
	else
	{
		UE_LOG(LogEdgegap, Error, TEXT("[Lobbies] Failed to deserialize response"));
		OnFailure.Broadcast(FEGIK_LobbyInfo(), FEGIK_ErrorStruct(0, TEXT("Failed to deserialize response")));
	}
}

void UEGIK_CreateLobby::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG(LogEdgegap, Error, TEXT("[Lobbies] CreateLobby failed with error %d: %s"), ErrorCode, *ErrorMessage);
	OnFailure.Broadcast(FEGIK_LobbyInfo(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
