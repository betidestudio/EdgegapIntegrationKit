// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EdgegapSettings.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Misc/ConfigCacheIni.h"

// We use UE's global config file name variables (GEditorIni, GEngineIni) instead of
// constructing paths to DefaultEditor.ini / DefaultEngine.ini directly.
//
// WHY: In UE 5.6+, calling GConfig->SetString/GetString with a raw file path (e.g.
// "Config/DefaultEditor.ini") causes Find() to create a SEPARATE OtherFiles entry
// marked NoSave=true. Flush() then silently skips saving, so values never persist.
//
// GEditorIni / GEngineIni point to the branches loaded during engine startup by the
// standard config hierarchy. These branches do NOT have NoSave set, so Flush works.
//
// - GEditorIni: Editor config hierarchy (only available WITH_EDITOR). Reads merge
//   DefaultEditor.ini + generated Editor.ini. Writes go to the generated file.
// - GEngineIni: Engine config hierarchy (always available). Same merge behavior.
//
// For shipped builds: env vars are the primary mechanism (checked by BPL functions).
// The GConfig fallback only matters in-editor during development.

UEdgegapSettings::UEdgegapSettings()
{
#if PLATFORM_WINDOWS
	DockerPath = TEXT("C:\\Program Files\\Docker\\Docker\\resources\\bin\\docker.exe");
#else
	DockerPath = TEXT("/usr/local/bin/docker");
#endif

	if(GConfig)
	{
		// Server-only keys: read from Editor config hierarchy (never ships with builds)
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), AuthorizationKey, GEditorIni);
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserServerToken"), ServerBrowserServerToken, GEditorIni);

		// Client-safe keys: read from Engine config hierarchy (ships with builds)
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserURL"), ServerBrowserURL, GEngineIni);
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserClientToken"), ServerBrowserClientToken, GEngineIni);
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("MatchmakingURL"), MatchmakingURL, GEngineIni);
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("MatchmakingAuthToken"), MatchmakingAuthToken, GEngineIni);
	}
}

#if WITH_EDITOR

UEdgegapSettings::FOnUpdateSettings UEdgegapSettings::OnSettingsChange;

void UEdgegapSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	OnSettingsChange.Broadcast(this);

	if (PropertyChangedEvent.Property == nullptr)
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		return;
	}

	const FString PropertyName = PropertyChangedEvent.Property->GetName();

	if (PropertyName == TEXT("APIToken"))
	{
		bIsTokenVerified = false;
		if (!APIToken.APIToken.IsEmpty())
		{
			AuthorizationKey = APIToken.APIToken;
			GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), *AuthorizationKey, GEditorIni);
			GConfig->Flush(false, GEditorIni);
		}
	}

	// Server-only keys → Editor config (never ships with builds)
	if (PropertyName == TEXT("AuthorizationKey"))
	{
		bIsTokenVerified = false;
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), *AuthorizationKey, GEditorIni);
		GConfig->Flush(false, GEditorIni);
	}
	if (PropertyName == TEXT("ServerBrowserServerToken"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserServerToken"), *ServerBrowserServerToken, GEditorIni);
		GConfig->Flush(false, GEditorIni);
	}

	// Client-safe keys → Engine config (ships with builds)
	if (PropertyName == TEXT("ServerBrowserURL"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserURL"), *ServerBrowserURL, GEngineIni);
		GConfig->Flush(false, GEngineIni);
	}
	if (PropertyName == TEXT("ServerBrowserClientToken"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserClientToken"), *ServerBrowserClientToken, GEngineIni);
		GConfig->Flush(false, GEngineIni);
	}
	if (PropertyName == TEXT("MatchmakingURL"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("MatchmakingURL"), *MatchmakingURL, GEngineIni);
		GConfig->Flush(false, GEngineIni);
	}
	if (PropertyName == TEXT("MatchmakingAuthToken"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("MatchmakingAuthToken"), *MatchmakingAuthToken, GEngineIni);
		GConfig->Flush(false, GEngineIni);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif // WITH_EDITOR
