// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIK_ListLocations.h"

UEGIK_ListLocations* UEGIK_ListLocations::ListLocations(FEGIK_ListAllLocationsRequest ListLocationsRequest)
{
	UEGIK_ListLocations* BlueprintNode = NewObject<UEGIK_ListLocations>();
	BlueprintNode->Var_ListLocationsRequest = ListLocationsRequest;
	return BlueprintNode;
}

void UEGIK_ListLocations::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest,
	TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	TArray<FEGIK_AvailableLocationStruct> Locations;
	if (HttpResponse.IsValid())
	{
		if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				TArray<TSharedPtr<FJsonValue>> LocationsArray = JsonObject->GetArrayField(TEXT("locations"));
				for (TSharedPtr<FJsonValue> Value : LocationsArray)
				{
					TSharedPtr<FJsonObject> LocationObject = Value->AsObject();
					FEGIK_AvailableLocationStruct Location;
					Location.City = LocationObject->GetStringField(TEXT("city"));
					Location.Country = LocationObject->GetStringField(TEXT("country"));
					Location.Continent = LocationObject->GetStringField(TEXT("continent"));
					Location.Timezone = LocationObject->GetStringField(TEXT("timezone"));
					Location.AdministrativeDivision = LocationObject->GetStringField(TEXT("administrative_division"));
					Location.Latitude = LocationObject->GetNumberField(TEXT("latitude"));
					Location.Longitude = LocationObject->GetNumberField(TEXT("longitude"));
					for(auto Tag : LocationObject->GetArrayField(TEXT("tags")))
					{
						Location.Tags.Add(Tag->AsObject()->GetStringField(TEXT("key")), Tag->AsObject()->GetStringField(TEXT("value")));
					}
					Locations.Add(Location);
				}
				OnSuccess.Broadcast(Locations, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(TArray<FEGIK_AvailableLocationStruct>(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
			}
		}
		else
		{
			OnFailure.Broadcast(TArray<FEGIK_AvailableLocationStruct>(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(TArray<FEGIK_AvailableLocationStruct>(), FEGIK_ErrorStruct(0, "Failed to process request"));
	}
	SetReadyToDestroy();
	MarkAsGarbage();
}

void UEGIK_ListLocations::Activate()
{
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL("https://api.edgegap.com/api/v1/locations");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", UEGIKBlueprintFunctionLibrary::GetAuthorizationKey());
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("app", Var_ListLocationsRequest.App);
	JsonObject->SetStringField("version", Var_ListLocationsRequest.Version);
	JsonObject->SetStringField("type", Var_ListLocationsRequest.Type);
	JsonObject->SetBoolField("tags", Var_ListLocationsRequest.bTags);
	FString Content;
	TSharedRef<TJsonWriter<TCHAR>> Writer = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	Request->SetContentAsString(Content);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_ListLocations::OnResponseReceived);
	if (!Request->ProcessRequest())
	{
		OnFailure.Broadcast(TArray<FEGIK_AvailableLocationStruct>(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}

