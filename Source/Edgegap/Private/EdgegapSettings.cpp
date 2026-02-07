// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EdgegapSettings.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Misc/ConfigCacheIni.h"

// DefaultEngine.ini ships with ALL builds (client + server) — only store client-safe keys here
static FString ProjectEngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");

// DefaultEditor.ini is editor-only and NEVER ships with builds — store server-only keys here
static FString ProjectEditorIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEditor.ini");

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
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), AuthorizationKey, ProjectEditorIniPath);
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserServerToken"), ServerBrowserServerToken, ProjectEditorIniPath);

		// Client-safe keys: read from DefaultEngine.ini (ships with builds)
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserURL"), ServerBrowserURL, ProjectEngineIniPath);
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserClientToken"), ServerBrowserClientToken, ProjectEngineIniPath);
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
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), *AuthorizationKey, ProjectEditorIniPath);
		GConfig->Flush(false, ProjectEditorIniPath);
	}
	if (PropertyName == TEXT("ServerBrowserServerToken"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserServerToken"), *ServerBrowserServerToken, ProjectEditorIniPath);
		GConfig->Flush(false, ProjectEditorIniPath);
	}

	// Client-safe keys → DefaultEngine.ini (ships with builds)
	if (PropertyName == TEXT("ServerBrowserURL"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserURL"), *ServerBrowserURL, ProjectEngineIniPath);
		GConfig->Flush(false, ProjectEngineIniPath);
	}
	if (PropertyName == TEXT("ServerBrowserClientToken"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("ServerBrowserClientToken"), *ServerBrowserClientToken, ProjectEngineIniPath);
		GConfig->Flush(false, ProjectEngineIniPath);
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif // WITH_EDITOR
