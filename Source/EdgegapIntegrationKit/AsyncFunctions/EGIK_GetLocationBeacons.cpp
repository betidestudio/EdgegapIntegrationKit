// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetLocationBeacons.h"

UEGIK_GetLocationBeacons* UEGIK_GetLocationBeacons::GetLocationBeacons(FString MatchmakingUrl, FString AuthToken)
{
	UEGIK_GetLocationBeacons* BlueprintNode = NewObject<UEGIK_GetLocationBeacons>();
	BlueprintNode->Var_MatchmakingUrl = MatchmakingUrl;
	BlueprintNode->Var_AuthToken = AuthToken;
	return BlueprintNode;
}

FString UEGIK_GetLocationBeacons::GetEndpointURL() const
{
	FString BaseURL = Var_MatchmakingUrl.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
		: Var_MatchmakingUrl;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return BaseURL + TEXT("/locations/beacons");
}

EEGIK_HttpVerb UEGIK_GetLocationBeacons::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_GetLocationBeacons::GetAuthorizationHeader() const
{
	return Var_AuthToken.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
		: Var_AuthToken;
}

void UEGIK_GetLocationBeacons::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	FEGIK_LocationBeaconResponse Response;

	if (JsonObject.IsValid())
	{
		Response.Count = JsonObject->GetIntegerField(TEXT("count"));
		TArray<TSharedPtr<FJsonValue>> BeaconsArray = JsonObject->GetArrayField(TEXT("beacons"));
		for (TSharedPtr<FJsonValue> BeaconValue : BeaconsArray)
		{
			TSharedPtr<FJsonObject> BeaconObject = BeaconValue->AsObject();
			FEGIK_LocationBeaconStruct Beacon;
			Beacon.FQDN = BeaconObject->GetStringField(TEXT("fqdn"));
			Beacon.PublicIP = BeaconObject->GetStringField(TEXT("public_ip"));
			Beacon.TCP_Port = BeaconObject->GetIntegerField(TEXT("tcp_port"));
			Beacon.UDP_Port = BeaconObject->GetIntegerField(TEXT("udp_port"));
			TSharedPtr<FJsonObject> LocationObject = BeaconObject->GetObjectField(TEXT("location"));
			Beacon.Location.City = LocationObject->GetStringField(TEXT("city"));
			Beacon.Location.Country = LocationObject->GetStringField(TEXT("country"));
			Beacon.Location.Continent = LocationObject->GetStringField(TEXT("continent"));
			Beacon.Location.AdministrativeDivision = LocationObject->GetStringField(TEXT("administrative_division"));
			Beacon.Location.Timezone = LocationObject->GetStringField(TEXT("timezone"));
			Response.Beacons.Add(Beacon);
		}
		OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
	}
	else
	{
		OnFailure.Broadcast(FEGIK_LocationBeaconResponse(), FEGIK_ErrorStruct(0, TEXT("Failed to parse JSON")));
	}
}

void UEGIK_GetLocationBeacons::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_LocationBeaconResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
