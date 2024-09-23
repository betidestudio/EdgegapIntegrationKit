// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIKBlueprintFunctionLibrary.h"

FString UEGIKBlueprintFunctionLibrary::Conv_EGIK_ErrorStructToString(FEGIK_ErrorStruct ErrorStruct)
{
	return FString::Printf(TEXT("Error Code: %d, Error Message: %s"), ErrorStruct.ErrorCode, *ErrorStruct.ErrorMessage);
}

FString UEGIKBlueprintFunctionLibrary::Conv_EGIK_AssignmentStructToString(FEGIK_AssignmentStruct AssignmentStruct)
{
	return FString::Printf(TEXT("Internal Port: %d, External Port: %d, Protocol: %s, City: %s, Country: %s, Continent: %s, Administrative Division: %s, Timezone: %s"), AssignmentStruct.GamePort.InternalPort, AssignmentStruct.GamePort.ExternalPort, *AssignmentStruct.GamePort.Protocol, *AssignmentStruct.Location.City, *AssignmentStruct.Location.Country, *AssignmentStruct.Location.Continent, *AssignmentStruct.Location.AdministrativeDivision, *AssignmentStruct.Location.Timezone);
}

FString UEGIKBlueprintFunctionLibrary::Conv_EGIK_MatchmakingResponseToString(FEGIK_MatchmakingResponse ResponseStruct)
{
	return FString::Printf(TEXT("Ticket ID: %s, Game Profile: %s, Created At: %s, Assignment: %s"), *ResponseStruct.TicketId, *ResponseStruct.GameProfile, *ResponseStruct.CreatedAt.ToString(), *Conv_EGIK_AssignmentStructToString(ResponseStruct.Assignment));
}

FString UEGIKBlueprintFunctionLibrary::GetAuthorizationKey()
{
	FString AuthorizationKey;
	FString ProjectEngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	if (GConfig)
	{
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), AuthorizationKey, ProjectEngineIniPath);
	}
	return AuthorizationKey;
}
