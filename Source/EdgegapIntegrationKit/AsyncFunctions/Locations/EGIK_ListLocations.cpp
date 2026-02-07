// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_ListLocations.h"

UEGIK_ListLocations* UEGIK_ListLocations::ListLocations(FEGIK_ListAllLocationsRequest ListLocationsRequest)
{
	UEGIK_ListLocations* BlueprintNode = NewObject<UEGIK_ListLocations>();
	BlueprintNode->Var_ListLocationsRequest = ListLocationsRequest;
	return BlueprintNode;
}

FString UEGIK_ListLocations::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/locations");
}

EEGIK_HttpVerb UEGIK_ListLocations::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

TSharedPtr<FJsonObject> UEGIK_ListLocations::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("app", Var_ListLocationsRequest.App);
	JsonObject->SetStringField("version", Var_ListLocationsRequest.Version);
	JsonObject->SetStringField("type", Var_ListLocationsRequest.Type);
	JsonObject->SetBoolField("tags", Var_ListLocationsRequest.bTags);
	return JsonObject;
}

void UEGIK_ListLocations::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	TArray<FEGIK_AvailableLocationStruct> Locations;

	if (JsonObject.IsValid())
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

void UEGIK_ListLocations::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(TArray<FEGIK_AvailableLocationStruct>(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
