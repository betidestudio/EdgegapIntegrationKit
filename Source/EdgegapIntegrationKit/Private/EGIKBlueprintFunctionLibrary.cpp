// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIKBlueprintFunctionLibrary.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Runtime/Core/Public/CoreGlobals.h"

// We use GEditorIni / GEngineIni instead of raw file paths.
// See EdgegapSettings.cpp for the full explanation of why raw paths break in UE 5.6+.
// In non-editor builds, GEditorIni is empty so GetString returns false gracefully,
// and the env var fallback (checked first) is the primary mechanism for shipped servers.

namespace
{
	bool GetConfigStringWithLegacySections(const TCHAR* Key, FString& OutValue, const FString& ConfigFile)
	{
		if (!GConfig)
		{
			return false;
		}

		if (GConfig->GetString(TEXT("EdgegapIntegrationKit"), Key, OutValue, ConfigFile))
		{
			return true;
		}

		if (GConfig->GetString(TEXT("UltimateCrossplayIntegrationKit"), Key, OutValue, ConfigFile))
		{
			return true;
		}

		return GConfig->GetString(TEXT("Ultimate Crossplay Integration Kit"), Key, OutValue, ConfigFile);
	}
}

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
	// 1. Environment variable (recommended for servers)
	FString AuthorizationKey = FPlatformMisc::GetEnvironmentVariable(TEXT("EDGEGAP_API_KEY"));
	if (!AuthorizationKey.IsEmpty())
	{
		return AuthorizationKey;
	}

	// 2. Editor config hierarchy (editor-only, never ships with builds)
	if (GConfig)
	{
		GetConfigStringWithLegacySections(TEXT("AuthorizationKey"), AuthorizationKey, GEditorIni);
	}

	return AuthorizationKey;
}

FString UEGIKBlueprintFunctionLibrary::GetServerBrowserURL()
{
	// 1. Environment variable
	FString Value = FPlatformMisc::GetEnvironmentVariable(TEXT("EDGEGAP_SB_URL"));
	if (!Value.IsEmpty())
	{
		return Value;
	}

	// 2. Engine config hierarchy (client-safe, ships with builds)
	if (GConfig)
	{
		GetConfigStringWithLegacySections(TEXT("ServerBrowserURL"), Value, GEngineIni);
	}
	return Value;
}

FString UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken()
{
	// 1. Environment variable (recommended for servers)
	FString Value = FPlatformMisc::GetEnvironmentVariable(TEXT("EDGEGAP_SB_SERVER_TOKEN"));
	if (!Value.IsEmpty())
	{
		return Value;
	}

	// 2. Editor config hierarchy (editor-only, never ships with builds)
	if (GConfig)
	{
		GetConfigStringWithLegacySections(TEXT("ServerBrowserServerToken"), Value, GEditorIni);
	}

	return Value;
}

FString UEGIKBlueprintFunctionLibrary::GetServerBrowserClientToken()
{
	// 1. Environment variable
	FString Value = FPlatformMisc::GetEnvironmentVariable(TEXT("EDGEGAP_SB_CLIENT_TOKEN"));
	if (!Value.IsEmpty())
	{
		return Value;
	}

	// 2. Engine config hierarchy (client-safe, ships with builds)
	if (GConfig)
	{
		GetConfigStringWithLegacySections(TEXT("ServerBrowserClientToken"), Value, GEngineIni);
	}
	return Value;
}

FString UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
{
	// 1. Environment variable
	FString Value = FPlatformMisc::GetEnvironmentVariable(TEXT("EDGEGAP_MM_URL"));
	if (!Value.IsEmpty())
	{
		return Value;
	}

	// 2. Engine config hierarchy (client-safe, ships with builds)
	if (GConfig)
	{
		GetConfigStringWithLegacySections(TEXT("MatchmakingURL"), Value, GEngineIni);
	}

	return Value;
}

FString UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
{
	// 1. Environment variables
	FString Value = FPlatformMisc::GetEnvironmentVariable(TEXT("EDGEGAP_MM_AUTH_TOKEN"));
	if (!Value.IsEmpty())
	{
		return Value;
	}

	Value = FPlatformMisc::GetEnvironmentVariable(TEXT("EDGEGAP_MATCHMAKER_AUTH_TOKEN"));
	if (!Value.IsEmpty())
	{
		return Value;
	}

	// 2. Engine config hierarchy (ships with builds)
	if (GConfig)
	{
		GetConfigStringWithLegacySections(TEXT("MatchmakingAuthToken"), Value, GEngineIni);
	}

	// 3. Editor config hierarchy fallback (editor-only)
	if (Value.IsEmpty() && GConfig)
	{
		GetConfigStringWithLegacySections(TEXT("MatchmakingAuthToken"), Value, GEditorIni);
	}

	return Value;
}

void UEGIKBlueprintFunctionLibrary::GetEnvironmentVariable(FString Key, FString& Value)
{
	Value = FPlatformMisc::GetEnvironmentVariable(*Key);
}

bool UEGIKBlueprintFunctionLibrary::GetGroupPlayerMapping(const FString& GroupID, TArray<FString>& PlayerIDs)
{
	// Look for group-to-player mapping in environment variables
	// Format is expected to be a JSON object mapping group IDs to player ID arrays
	FString GroupMappingJson;
	GetEnvironmentVariable("MM_GROUP_MAPPING", GroupMappingJson);

	if (GroupMappingJson.IsEmpty())
	{
		return false;
	}

	// Parse the JSON
	TSharedPtr<FJsonObject> GroupMapping;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(GroupMappingJson);
	if (!FJsonSerializer::Deserialize(Reader, GroupMapping) || !GroupMapping.IsValid())
	{
		return false;
	}

	// Check if the group ID exists in the mapping
	if (!GroupMapping->HasField(GroupID))
	{
		return false;
	}

	// Get the player IDs array for this group
	const TArray<TSharedPtr<FJsonValue>>* PlayerIDsArray;
	if (!GroupMapping->TryGetArrayField(GroupID, PlayerIDsArray))
	{
		return false;
	}

	// Convert to string array
	PlayerIDs.Empty();
	for (const TSharedPtr<FJsonValue>& Value : *PlayerIDsArray)
	{
		if (Value->Type == EJson::String)
		{
			PlayerIDs.Add(Value->AsString());
		}
	}

	return PlayerIDs.Num() > 0;
}

bool UEGIKBlueprintFunctionLibrary::GetExpansionStage(FString& ExpansionStage)
{
	GetEnvironmentVariable("MM_EXPANSION_STAGE", ExpansionStage);
	return !ExpansionStage.IsEmpty();
}

bool UEGIKBlueprintFunctionLibrary::GetMatchProfileName(FString& ProfileName)
{
	GetEnvironmentVariable("MM_MATCH_PROFILE", ProfileName);
	return !ProfileName.IsEmpty();
}

TArray<FString> UEGIKBlueprintFunctionLibrary::ConvertJsonArrayToStringArray(const FString& JsonArray)
{
	TArray<FString> StringArray;
	TSharedPtr<FJsonValue> JsonValue;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonArray);
	if (FJsonSerializer::Deserialize(Reader, JsonValue))
	{
		if (JsonValue->Type == EJson::Array)
		{
			for (TSharedPtr<FJsonValue> Value : JsonValue->AsArray())
			{
				StringArray.Add(Value->AsString());
			}
		}
	}
	return StringArray;
}
