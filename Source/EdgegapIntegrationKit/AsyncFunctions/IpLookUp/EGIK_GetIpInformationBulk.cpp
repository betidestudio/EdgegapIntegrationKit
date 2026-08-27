// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetIpInformationBulk.h"

UEGIK_GetIpInformationBulk* UEGIK_GetIpInformationBulk::GetIpInformationBulk(TArray<FString> IpAddresses)
{
	UEGIK_GetIpInformationBulk* Node = NewObject<UEGIK_GetIpInformationBulk>();
	Node->Var_IpAddresses = IpAddresses;
	return Node;
}

FString UEGIK_GetIpInformationBulk::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/ips/lookup");
}

EEGIK_HttpVerb UEGIK_GetIpInformationBulk::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

TSharedPtr<FJsonObject> UEGIK_GetIpInformationBulk::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetArrayField(TEXT("addresses"), StringArrayToJsonArray(Var_IpAddresses));
	return JsonObject;
}

void UEGIK_GetIpInformationBulk::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	TArray<FEGIK_IpLookUpAddress> IpAddresses;

	if (JsonObject->HasField(TEXT("addresses")))
	{
		TArray<TSharedPtr<FJsonValue>> JsonArray = JsonObject->GetArrayField(TEXT("addresses"));
		for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
		{
			TSharedPtr<FJsonObject> JsonObj = JsonValue->AsObject();
			if (!JsonObj.IsValid()) continue;

			FEGIK_IpLookUpAddress IpAddress;
			IpAddress.IP = JsonObj->GetStringField(TEXT("ip_address"));
			IpAddress.Type = JsonObj->GetStringField(TEXT("type"));

			if (JsonObj->HasField(TEXT("location")))
			{
				TSharedPtr<FJsonObject> LocationObj = JsonObj->GetObjectField(TEXT("location"));
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

			IpAddresses.Add(IpAddress);
		}
	}

	OnSuccess.Broadcast(IpAddresses, FEGIK_ErrorStruct());
}

void UEGIK_GetIpInformationBulk::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(TArray<FEGIK_IpLookUpAddress>(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
