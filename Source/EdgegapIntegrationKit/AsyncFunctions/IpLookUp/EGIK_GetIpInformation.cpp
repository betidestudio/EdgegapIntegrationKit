// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetIpInformation.h"

UEGIK_GetIpInformation* UEGIK_GetIpInformation::GetIpInformation()
{
	UEGIK_GetIpInformation* Node = NewObject<UEGIK_GetIpInformation>();
	return Node;
}

FString UEGIK_GetIpInformation::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/ip");
}

EEGIK_HttpVerb UEGIK_GetIpInformation::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_GetIpInformation::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_IpLookUpAddress IpAddress;
	IpAddress.IP = JsonObject->GetStringField(TEXT("ip_address"));
	IpAddress.Type = JsonObject->GetStringField(TEXT("type"));

	if (JsonObject->HasField(TEXT("location")))
	{
		TSharedPtr<FJsonObject> LocationObj = JsonObject->GetObjectField(TEXT("location"));
		if (LocationObj->HasField(TEXT("continent")))
		{
			TSharedPtr<FJsonObject> ContinentObj = LocationObj->GetObjectField(TEXT("continent"));
			IpAddress.Location.Continent.Code = ContinentObj->GetStringField(TEXT("code"));
			IpAddress.Location.Continent.Name = ContinentObj->GetStringField(TEXT("name"));
		}
		if (LocationObj->HasField(TEXT("country")))
		{
			TSharedPtr<FJsonObject> CountryObj = LocationObj->GetObjectField(TEXT("country"));
			IpAddress.Location.Country.Code = CountryObj->GetStringField(TEXT("code"));
			IpAddress.Location.Country.Name = CountryObj->GetStringField(TEXT("name"));
		}
		IpAddress.Location.LatitudeLongitude.Latitude = LocationObj->GetNumberField(TEXT("latitude"));
		IpAddress.Location.LatitudeLongitude.Longitude = LocationObj->GetNumberField(TEXT("longitude"));
	}

	OnSuccess.Broadcast(IpAddress, FEGIK_ErrorStruct());
}

void UEGIK_GetIpInformation::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_IpLookUpAddress(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
