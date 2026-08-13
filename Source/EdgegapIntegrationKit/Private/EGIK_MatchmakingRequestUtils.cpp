// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_MatchmakingRequestUtils.h"
#include "EGIKBlueprintFunctionLibrary.h"

FString EGIK_MatchmakingRequest::ResolveURL(const FString& OverrideURL)
{
	FString URL = OverrideURL.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
		: OverrideURL;
	URL.TrimStartAndEndInline();
	while (URL.EndsWith(TEXT("/")))
	{
		URL.LeftChopInline(1);
	}
	return URL;
}

FString EGIK_MatchmakingRequest::ResolveAuthToken(const FString& OverrideToken)
{
	FString Token = OverrideToken.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
		: OverrideToken;
	Token.TrimStartAndEndInline();
	return Token;
}

bool EGIK_MatchmakingRequest::ValidateConnection(const FString& OverrideURL, const FString& OverrideToken, FString& OutErrorMessage)
{
	const FString URL = ResolveURL(OverrideURL);
	if (URL.IsEmpty())
	{
		OutErrorMessage = TEXT("Matchmaking URL is empty. Set it in plugin settings/environment values or on this request.");
		return false;
	}
	if (!URL.StartsWith(TEXT("https://"), ESearchCase::IgnoreCase)
		&& !URL.StartsWith(TEXT("http://"), ESearchCase::IgnoreCase))
	{
		OutErrorMessage = FString::Printf(TEXT("Matchmaking URL '%s' must start with http:// or https://."), *URL);
		return false;
	}
	if (ResolveAuthToken(OverrideToken).IsEmpty())
	{
		OutErrorMessage = TEXT("Matchmaking auth token is empty. Set it in plugin settings/environment values or on this request.");
		return false;
	}
	return true;
}

bool EGIK_MatchmakingRequest::ValidateProfile(const FString& Profile, FString& OutErrorMessage)
{
	FString TrimmedProfile = Profile;
	TrimmedProfile.TrimStartAndEndInline();
	if (TrimmedProfile.IsEmpty())
	{
		OutErrorMessage = TEXT("Profile is required and must exactly match a profile name in the running Edgegap matchmaker configuration.");
		return false;
	}
	if (!Profile.Equals(TrimmedProfile, ESearchCase::CaseSensitive))
	{
		OutErrorMessage = TEXT("Profile contains leading or trailing whitespace. Profile names are case-sensitive and must match the matchmaker configuration exactly.");
		return false;
	}
	return true;
}

bool EGIK_MatchmakingRequest::ValidateResourceId(const FString& FieldName, const FString& ResourceId, FString& OutErrorMessage)
{
	FString TrimmedId = ResourceId;
	TrimmedId.TrimStartAndEndInline();
	if (TrimmedId.IsEmpty())
	{
		OutErrorMessage = FString::Printf(TEXT("%s is required."), *FieldName);
		return false;
	}
	if (!ResourceId.Equals(TrimmedId, ESearchCase::CaseSensitive))
	{
		OutErrorMessage = FString::Printf(TEXT("%s contains leading or trailing whitespace."), *FieldName);
		return false;
	}
	if (ResourceId.Contains(TEXT("/")) || ResourceId.Contains(TEXT("?")) || ResourceId.Contains(TEXT("#")))
	{
		OutErrorMessage = FString::Printf(TEXT("%s contains characters that are not valid in an Edgegap resource ID."), *FieldName);
		return false;
	}
	return true;
}
