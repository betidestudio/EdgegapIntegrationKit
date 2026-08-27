// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_MatchmakingAttributes.h"

namespace EGIK_MatchmakingProfileDebug
{
	/**
	 * Best-effort runtime diagnostic. Fetches the active Matchmaker OpenAPI schema,
	 * compares it with StructuredAttributes, and logs the result without delaying
	 * or changing the matchmaking request.
	 */
	void LogProfileAlignment(
		const FString& MatchmakingURL,
		const FString& AuthToken,
		const FString& RequestContext,
		const FString& Profile,
		const FEGIK_MatchmakingAttributes& StructuredAttributes,
		const FString& RawAttributesJson);

	/** Cancel in-flight schema requests and clear runtime-only cached contracts. */
	void Shutdown();
}
