// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace EGIK_MatchmakingRequest
{
	EDGEGAPINTEGRATIONKIT_API FString ResolveURL(const FString& OverrideURL);
	EDGEGAPINTEGRATIONKIT_API FString ResolveAuthToken(const FString& OverrideToken);
	EDGEGAPINTEGRATIONKIT_API bool ValidateConnection(const FString& OverrideURL, const FString& OverrideToken, FString& OutErrorMessage);
	EDGEGAPINTEGRATIONKIT_API bool ValidateProfile(const FString& Profile, FString& OutErrorMessage);
	EDGEGAPINTEGRATIONKIT_API bool ValidateResourceId(const FString& FieldName, const FString& ResourceId, FString& OutErrorMessage);
}
