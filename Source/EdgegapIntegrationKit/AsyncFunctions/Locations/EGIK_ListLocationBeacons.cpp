// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EGIK_ListLocationBeacons.h"

UEGIK_ListLocationBeacons* UEGIK_ListLocationBeacons::ListLocationBeacons()
{
	UEGIK_ListLocationBeacons* BlueprintNode = NewObject<UEGIK_ListLocationBeacons>();
	return BlueprintNode;
}

void UEGIK_ListLocationBeacons::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	TArray<FEGIK_ListLocationBeaconsResponse> LocationBeacons;
	if (HttpResponse.IsValid())
	{
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
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
		else
		{
			OnFailure.Broadcast(TArray<FEGIK_ListLocationBeaconsResponse>(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(TArray<FEGIK_ListLocationBeaconsResponse>(), FEGIK_ErrorStruct(0, "Failed to process request"));
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UEGIK_ListLocationBeacons::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL("https://api.edgegap.com/v1/locations/beacons");
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_ListLocationBeacons::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(TArray<FEGIK_ListLocationBeaconsResponse>(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}