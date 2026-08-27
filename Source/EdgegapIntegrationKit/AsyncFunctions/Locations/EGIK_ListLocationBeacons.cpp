// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_ListLocationBeacons.h"

UEGIK_ListLocationBeacons* UEGIK_ListLocationBeacons::ListLocationBeacons()
{
	UEGIK_ListLocationBeacons* BlueprintNode = NewObject<UEGIK_ListLocationBeacons>();
	return BlueprintNode;
}

FString UEGIK_ListLocationBeacons::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/locations/beacons");
}

EEGIK_HttpVerb UEGIK_ListLocationBeacons::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_ListLocationBeacons::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	TArray<FEGIK_ListLocationBeaconsResponse> LocationBeacons;

	if (JsonObject.IsValid())
	{
		TArray<TSharedPtr<FJsonValue>> LocationBeaconsArray = JsonObject->GetArrayField(TEXT("locations"));
		for (TSharedPtr<FJsonValue> Value : LocationBeaconsArray)
		{
			TSharedPtr<FJsonObject> LocationBeaconObject = Value->AsObject();
			FEGIK_ListLocationBeaconsResponse LocationBeacon;
			LocationBeacon.Host = LocationBeaconObject->GetStringField(TEXT("host"));
			LocationBeacon.FQDN = LocationBeaconObject->GetStringField(TEXT("fqdn"));
			LocationBeacon.UdpPort = LocationBeaconObject->GetNumberField(TEXT("udp_port"));
			LocationBeacon.TcpPort = LocationBeaconObject->GetNumberField(TEXT("tcp_port"));
			LocationBeacon.Location.City = LocationBeaconObject->GetObjectField(TEXT("location"))->GetStringField(TEXT("city"));
			LocationBeacon.Location.Country = LocationBeaconObject->GetObjectField(TEXT("location"))->GetStringField(TEXT("country"));
			LocationBeacon.Location.Continent = LocationBeaconObject->GetObjectField(TEXT("location"))->GetStringField(TEXT("continent"));
			LocationBeacon.Location.Timezone = LocationBeaconObject->GetObjectField(TEXT("location"))->GetStringField(TEXT("timezone"));
			LocationBeacon.Location.AdministrativeDivision = LocationBeaconObject->GetObjectField(TEXT("location"))->GetStringField(TEXT("administrative_division"));
			LocationBeacon.LatitudeLongitude.Latitude = LocationBeaconObject->GetObjectField(TEXT("location"))->GetNumberField(TEXT("latitude"));
			LocationBeacon.LatitudeLongitude.Longitude = LocationBeaconObject->GetObjectField(TEXT("location"))->GetNumberField(TEXT("longitude"));
			LocationBeacons.Add(LocationBeacon);
		}
		OnSuccess.Broadcast(LocationBeacons, FEGIK_ErrorStruct());
	}
	else
	{
		OnFailure.Broadcast(TArray<FEGIK_ListLocationBeaconsResponse>(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
	}
}

void UEGIK_ListLocationBeacons::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(TArray<FEGIK_ListLocationBeaconsResponse>(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
