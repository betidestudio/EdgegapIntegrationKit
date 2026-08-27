// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetLobby.h"

UEGIK_GetLobby* UEGIK_GetLobby::GetLobby(FString LobbyName)
{
	UEGIK_GetLobby* Node = NewObject<UEGIK_GetLobby>();
	Node->Var_LobbyName = LobbyName;
	return Node;
}

FString UEGIK_GetLobby::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/lobbies/%s"), *Var_LobbyName);
}

EEGIK_HttpVerb UEGIK_GetLobby::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_GetLobby::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
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

void UEGIK_GetLobby::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG(LogEdgegap, Error, TEXT("[Lobbies] GetLobby failed with error %d: %s"), ErrorCode, *ErrorMessage);
	OnFailure.Broadcast(FEGIK_LobbyInfo(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
