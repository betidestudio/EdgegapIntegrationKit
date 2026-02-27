// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EdgegapSettings.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Misc/ConfigCacheIni.h"

// Lazy-initialized normalized paths to avoid non-normalized path warnings in UE 5.6+
// (FPaths::ProjectConfigDir() returns a relative path that GConfig may fail to match)

// DefaultEngine.ini ships with ALL builds (client + server) — only store client-safe keys here
static const FString& GetProjectEngineIniPath()
{
	static FString Path = FConfigCacheIni::NormalizeConfigIniPath(FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini"));
	return Path;
}

// DefaultEditor.ini is editor-only and NEVER ships with builds — store server-only keys here
static const FString& GetProjectEditorIniPath()
{
	static FString Path = FConfigCacheIni::NormalizeConfigIniPath(FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini"));
	return Path;
}

UEdgegapSettings::UEdgegapSettings()
{
#if PLATFORM_WINDOWS
	DockerPath = TEXT("C:\\Program Files\\Docker\\Docker\\resources\\bin\\docker.exe");
#else
	DockerPath = TEXT("/usr/local/bin/docker");
#endif

	if(GConfig)
	{
		// Server-only keys: read from DefaultEditor.ini (never ships with builds)
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), AuthorizationKey, GetProjectEditorIniPath());
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserServerToken"), ServerBrowserServerToken, GetProjectEditorIniPath());

		// Client-safe keys: read from DefaultEngine.ini (ships with builds)
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserURL"), ServerBrowserURL, GetProjectEngineIniPath());
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserClientToken"), ServerBrowserClientToken, GetProjectEngineIniPath());
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("MatchmakingURL"), MatchmakingURL, GetProjectEngineIniPath());
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("MatchmakingAuthToken"), MatchmakingAuthToken, GetProjectEngineIniPath());
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
	}

	// Server-only keys → DefaultEditor.ini (never ships with builds)
	if (PropertyName == TEXT("AuthorizationKey"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), *AuthorizationKey, GetProjectEditorIniPath());
		GConfig->Flush(false, GetProjectEditorIniPath());
	}
	if (PropertyName == TEXT("ServerBrowserServerToken"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserServerToken"), *ServerBrowserServerToken, GetProjectEditorIniPath());
		GConfig->Flush(false, GetProjectEditorIniPath());
	}

	// Client-safe keys → DefaultEngine.ini (ships with builds)
	if (PropertyName == TEXT("ServerBrowserURL"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserURL"), *ServerBrowserURL, GetProjectEngineIniPath());
		GConfig->Flush(false, GetProjectEngineIniPath());
	}
	if (PropertyName == TEXT("ServerBrowserClientToken"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserClientToken"), *ServerBrowserClientToken, GetProjectEngineIniPath());
		GConfig->Flush(false, GetProjectEngineIniPath());
	}
	if (PropertyName == TEXT("MatchmakingURL"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("MatchmakingURL"), *MatchmakingURL, GetProjectEngineIniPath());
		GConfig->Flush(false, GetProjectEngineIniPath());
	}
	if (PropertyName == TEXT("MatchmakingAuthToken"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("MatchmakingAuthToken"), *MatchmakingAuthToken, GetProjectEngineIniPath());
		GConfig->Flush(false, GetProjectEngineIniPath());
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif // WITH_EDITOR
