// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EdgegapSettings.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Misc/ConfigCacheIni.h"

FString ProjectEngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");

UEdgegapSettings::UEdgegapSettings()
{
	if(GConfig)
	{
		GConfig->GetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), AuthorizationKey, ProjectEngineIniPath);
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

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetName() == TEXT("APIToken"))
	{
		bIsTokenVerified = false;
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetName() == TEXT("AuthorizationKey"))
	{
		GConfig->SetString(TEXT("EdgegapIntegrationKit"), TEXT("AuthorizationKey"), *AuthorizationKey, ProjectEngineIniPath);
		GConfig->Flush(false, ProjectEngineIniPath);
	}

}

#endif // WITH_EDITOR
