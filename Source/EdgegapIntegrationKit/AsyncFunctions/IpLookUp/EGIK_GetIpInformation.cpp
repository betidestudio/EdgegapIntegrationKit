// Fill out your copyright notice in the Description page of Project Settings.


#include "EGIK_GetIpInformation.h"

UEGIK_GetIpInformation* UEGIK_GetIpInformation::GetIpInformation()
{
	UEGIK_GetIpInformation* Node = NewObject<UEGIK_GetIpInformation>();
	return Node;
}


void UEGIK_GetIpInformation::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	if (HttpResponse.IsValid())
	{
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				FEGIK_IpLookUpAddress IpAddress;
				IpAddress.IP = JsonObject->GetStringField(TEXT("ip_address"));
				IpAddress.Type = JsonObject->GetStringField(TEXT("type"));
				IpAddress.Location.Continent.Code = JsonObject->GetObjectField(TEXT("location"))->GetObjectField(TEXT("continent"))->GetStringField(TEXT("code"));
				IpAddress.Location.Continent.Name = JsonObject->GetObjectField(TEXT("location"))->GetObjectField(TEXT("continent"))->GetStringField(TEXT("name"));
				IpAddress.Location.Country.Code = JsonObject->GetObjectField(TEXT("location"))->GetObjectField(TEXT("country"))->GetStringField(TEXT("code"));
				IpAddress.Location.Country.Name = JsonObject->GetObjectField(TEXT("location"))->GetObjectField(TEXT("country"))->GetStringField(TEXT("name"));
				IpAddress.Location.LatitudeLongitude.Latitude = JsonObject->GetObjectField(TEXT("location"))->GetNumberField(TEXT("latitude"));
				IpAddress.Location.LatitudeLongitude.Longitude = JsonObject->GetObjectField(TEXT("location"))->GetNumberField(TEXT("longitude"));
				OnSuccess.Broadcast(IpAddress, FEGIK_ErrorStruct());					
			}
			else
			{
				OnFailure.Broadcast(FEGIK_IpLookUpAddress(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_IpLookUpAddress(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), "Failed to get ip information"));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_IpLookUpAddress(), FEGIK_ErrorStruct(0, "Failed to deserialize response"));
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UEGIK_GetIpInformation::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL("https://api.edgegap.com/v1/ip");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", "token 3d6e71e0-5717-4e8b-959d-b374d004be73");
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_GetIpInformation::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_IpLookUpAddress(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}
