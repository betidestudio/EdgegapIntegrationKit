// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "Misc/EngineVersionComparison.h"

// UE 5.8 changed FJsonObject::Values from TMap<FString, ...> to
// TMap<UE::FSharedString, ...>. FSharedString does not implicitly convert to
// FString, so iterating Values yields keys that can no longer be passed straight
// into a TMap<FString, ...>::Add(). Wrap JSON keys with EGIK_JSONKEY() when
// inserting them into an FString-keyed map.
//
// On 5.8+ we build an FString from the shared string's TCHAR data (operator*).
// On 5.7 and older the key is already an FString, so it is passed through as-is.
#if UE_VERSION_OLDER_THAN(5, 8, 0)
	#define EGIK_JSONKEY(KeyExpr) (KeyExpr)
#else
	#define EGIK_JSONKEY(KeyExpr) (FString(*(KeyExpr)))
#endif
