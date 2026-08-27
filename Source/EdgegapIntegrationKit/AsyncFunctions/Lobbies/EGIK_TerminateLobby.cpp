// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_TerminateLobby.h"

UEGIK_TerminateLobby* UEGIK_TerminateLobby::TerminateLobby(FString LobbyName)
{
	UEGIK_TerminateLobby* Node = NewObject<UEGIK_TerminateLobby>();
	Node->Var_LobbyName = LobbyName;
	return Node;
}

FString UEGIK_TerminateLobby::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/lobbies:terminate");
}

EEGIK_HttpVerb UEGIK_TerminateLobby::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

TSharedPtr<FJsonObject> UEGIK_TerminateLobby::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), Var_LobbyName);
	return JsonObject;
}

void UEGIK_TerminateLobby::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
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

void UEGIK_TerminateLobby::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG(LogEdgegap, Error, TEXT("[Lobbies] TerminateLobby failed with error %d: %s"), ErrorCode, *ErrorMessage);
	OnFailure.Broadcast(FEGIK_LobbyInfo(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
