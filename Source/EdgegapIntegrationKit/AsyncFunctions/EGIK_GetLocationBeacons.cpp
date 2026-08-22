// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetLocationBeacons.h"

#include "EGIK_MatchmakingRequestUtils.h"

UEGIK_GetLocationBeacons* UEGIK_GetLocationBeacons::GetLocationBeacons(FString MatchmakingUrl, FString AuthToken)
{
	UEGIK_GetLocationBeacons* BlueprintNode = NewObject<UEGIK_GetLocationBeacons>();
	BlueprintNode->Var_MatchmakingUrl = MatchmakingUrl;
	BlueprintNode->Var_AuthToken = AuthToken;
	return BlueprintNode;
}

FString UEGIK_GetLocationBeacons::GetEndpointURL() const
{
	return EGIK_MatchmakingRequest::ResolveURL(Var_MatchmakingUrl) + TEXT("/locations/beacons");
}

EEGIK_HttpVerb UEGIK_GetLocationBeacons::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

FString UEGIK_GetLocationBeacons::GetAuthorizationHeader() const
{
	return EGIK_MatchmakingRequest::ResolveAuthToken(Var_AuthToken);
}

bool UEGIK_GetLocationBeacons::ValidateRequest(FString& OutErrorMessage) const
{
	return EGIK_MatchmakingRequest::ValidateConnection(Var_MatchmakingUrl, Var_AuthToken, OutErrorMessage);
}

void UEGIK_GetLocationBeacons::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	FEGIK_LocationBeaconResponse Response;

	const TArray<TSharedPtr<FJsonValue>>* BeaconsArray = nullptr;
	if (JsonObject.IsValid() && JsonObject->TryGetArrayField(TEXT("beacons"), BeaconsArray) && BeaconsArray)
	{
		double Count = 0.0;
		if (JsonObject->TryGetNumberField(TEXT("count"), Count))
		{
			Response.Count = static_cast<int32>(Count);
		}

		for (const TSharedPtr<FJsonValue>& BeaconValue : *BeaconsArray)
		{
			if (!BeaconValue.IsValid() || BeaconValue->Type != EJson::Object)
			{
				continue;
			}

			const TSharedPtr<FJsonObject> BeaconObject = BeaconValue->AsObject();
			if (!BeaconObject.IsValid())
			{
				continue;
			}

			FEGIK_LocationBeaconStruct Beacon;
			BeaconObject->TryGetStringField(TEXT("fqdn"), Beacon.FQDN);
			BeaconObject->TryGetStringField(TEXT("public_ip"), Beacon.PublicIP);

			double Port = 0.0;
			if (BeaconObject->TryGetNumberField(TEXT("tcp_port"), Port))
			{
				Beacon.TCP_Port = static_cast<int32>(Port);
			}
			if (BeaconObject->TryGetNumberField(TEXT("udp_port"), Port))
			{
				Beacon.UDP_Port = static_cast<int32>(Port);
			}

			if (BeaconObject->HasTypedField<EJson::Object>(TEXT("location")))
			{
				const TSharedPtr<FJsonObject> LocationObject = BeaconObject->GetObjectField(TEXT("location"));
				if (LocationObject.IsValid())
				{
					LocationObject->TryGetStringField(TEXT("city"), Beacon.Location.City);
					LocationObject->TryGetStringField(TEXT("country"), Beacon.Location.Country);
					LocationObject->TryGetStringField(TEXT("continent"), Beacon.Location.Continent);
					LocationObject->TryGetStringField(TEXT("administrative_division"), Beacon.Location.AdministrativeDivision);
					LocationObject->TryGetStringField(TEXT("timezone"), Beacon.Location.Timezone);
				}
			}
			Response.Beacons.Add(Beacon);
		}
		if (Response.Count == 0)
		{
			Response.Count = Response.Beacons.Num();
		}
		OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
	}
	else
	{
		OnFailure.Broadcast(FEGIK_LocationBeaconResponse(), FEGIK_ErrorStruct(0, TEXT("Location-beacons response did not contain a valid 'beacons' array.")));
	}
}

void UEGIK_GetLocationBeacons::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_LocationBeaconResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
