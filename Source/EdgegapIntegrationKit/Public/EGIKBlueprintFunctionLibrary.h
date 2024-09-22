// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Interfaces/IHttpResponse.h"
#include "EGIKBlueprintFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_PortStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
	int32 InternalPort;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
	int32 ExternalPort;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
	FString Protocol;
};


USTRUCT(BlueprintType)
struct FEGIK_LocationStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
	FString City;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
	FString Country;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
	FString Continent;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
	FString AdministrativeDivision;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
	FString Timezone;
};

USTRUCT(BlueprintType)
struct FEGIK_ErrorStruct
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Error")
    int32 ErrorCode;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Error")
    FString ErrorMessage;

    FEGIK_ErrorStruct()
    {
        ErrorCode = 0;
        ErrorMessage = "";
    }
    FEGIK_ErrorStruct(int32 Code, FString Message)
    {
        ErrorCode = Code;
        ErrorMessage = Message;
    }
};


USTRUCT(BlueprintType)
struct FEGIK_AssignmentStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
    bool bIsNullOrEmpty;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
    FString FQDN;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
    FString PublicIP;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
    FEGIK_PortStruct GamePort;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Assignment")
    FEGIK_LocationStruct Location;

    // Default constructor
    FEGIK_AssignmentStruct()
    {
        Reset();
    }

    // Constructor from JSON string
    FEGIK_AssignmentStruct(FString JsonValue)
    {
        if (JsonValue.IsEmpty() || JsonValue == "null")
        {
            Reset();
        }
        else
        {
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonValue);

            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
            {
                bIsNullOrEmpty = false;
                DeserializeFromJson(JsonObject);
            }
            else
            {
                Reset();
            }
        }
    }

    // Constructor from FJsonObject
    FEGIK_AssignmentStruct(TSharedPtr<FJsonObject> JsonObject)
    {
        if (JsonObject.IsValid())
        {
            bIsNullOrEmpty = false;
            DeserializeFromJson(JsonObject);
        }
        else
        {
            Reset();
        }
    }

private:
    // Helper function to reset the struct
    void Reset()
    {
        bIsNullOrEmpty = true;
        FQDN = "";
        PublicIP = "";
        GamePort.InternalPort = 0;
        GamePort.ExternalPort = 0;
        GamePort.Protocol = "";
        Location.City = "";
        Location.Country = "";
        Location.Continent = "";
        Location.AdministrativeDivision = "";
        Location.Timezone = "";
    }

    // Helper function to deserialize JSON into the struct
    void DeserializeFromJson(TSharedPtr<FJsonObject> JsonObject)
    {
        if (!JsonObject.IsValid()) return;

        // Parse the FQDN and Public IP fields
        JsonObject->TryGetStringField(TEXT("fqdn"), FQDN);
        JsonObject->TryGetStringField(TEXT("public_ip"), PublicIP);

        // Navigate to the "ports" object and then "gameport"
        const TSharedPtr<FJsonObject>* PortsObject;
        if (JsonObject->TryGetObjectField(TEXT("ports"), PortsObject))
        {
            const TSharedPtr<FJsonObject>* GamePortObject;
            if ((*PortsObject)->TryGetObjectField(TEXT("gameport"), GamePortObject))
            {
                (*GamePortObject)->TryGetNumberField(TEXT("internal"), GamePort.InternalPort);
                (*GamePortObject)->TryGetNumberField(TEXT("external"), GamePort.ExternalPort);
                (*GamePortObject)->TryGetStringField(TEXT("protocol"), GamePort.Protocol);
            }
        }

        // Navigate to the "location" object
        const TSharedPtr<FJsonObject>* LocationObject;
        if (JsonObject->TryGetObjectField(TEXT("location"), LocationObject))
        {
            (*LocationObject)->TryGetStringField(TEXT("city"), Location.City);
            (*LocationObject)->TryGetStringField(TEXT("country"), Location.Country);
            (*LocationObject)->TryGetStringField(TEXT("continent"), Location.Continent);
            (*LocationObject)->TryGetStringField(TEXT("administrative_division"), Location.AdministrativeDivision);
            (*LocationObject)->TryGetStringField(TEXT("timezone"), Location.Timezone);
        }
    }
};

USTRUCT(BlueprintType)
struct FEGIK_CreateMatchmakingStruct
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
    FString Profile;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
    FString Attributes;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
    FString RealIp = "1.1.1.1";

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
    FString MatchmakingURL;
};

USTRUCT(BlueprintType)
struct FEGIK_MatchmakingResponse
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
    FString TicketId;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
    FString GameProfile;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
    FEGIK_AssignmentStruct Assignment;

    UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
    FDateTime CreatedAt;
};

USTRUCT(BlueprintType)
struct FEGIK_GeoIpStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString IP;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Latitude;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Longitude;
};

USTRUCT(BlueprintType)
struct FEGIK_LatitudeLongitudeStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Latitude;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Longitude;
};

USTRUCT(BlueprintType)
struct FEGIK_ContainerLogStorageStruct
{
	GENERATED_BODY()

	//Will override the app version container log storage for this deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bEnabled = false;

	//The name of your endpoint storage. If container log storage is enabled without this parameter, we will try to take the app version endpoint storage. If there is no endpoint storage in your app version, the container logs will not be stored. If we don't find any endpoint storage associated with this name, the container logs will not be stored.
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString EndpointStorageName;
};

USTRUCT(BlueprintType)
struct FEGIK_EnvironmentVariableStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Value;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bIsHidden = false;
};

USTRUCT(BlueprintType)
struct FEGIK_SelectorStruct
{
	GENERATED_BODY()

	//The Tag to filter potential Deployment with this Selector
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Tag;

	//If True, will not try to filter Deployment and only tag the Session
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool IsTagOnly = false;

	//Environment Variable to inject in new Deployment created by App Version with auto-deploy
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_EnvironmentVariableStruct EnvironmentVariable;
};

UENUM(BlueprintType)
enum EEGIK_FilterType
{
	EGIK_Any UMETA(DisplayName = "Any"),
	EGIK_All UMETA(DisplayName = "All"),
	EGIK_Not UMETA(DisplayName = "Not")
};

UENUM(BlueprintType)
enum EEGIK_ApSortStrategy
{
	EGIK_Basic UMETA(DisplayName = "Basic"),
	EGIK_Weighted UMETA(DisplayName = "Weighted")
};

USTRUCT(BlueprintType)
struct FEGIK_FiltersStruct
{
	GENERATED_BODY()

	//The field to filter
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Field;

	//The values to filter
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Values;

	//The type of filter
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TEnumAsByte<EEGIK_FilterType> FilterType;

	TSharedPtr<FJsonObject> ToJsonObject()
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField("field", Field);

		TArray<TSharedPtr<FJsonValue>> ValuesArray;
		for (auto val : Values)
		{
			ValuesArray.Add(MakeShareable(new FJsonValueString(val)));
		}
		JsonObject->SetArrayField("values", ValuesArray);
		switch(FilterType)
		{
		case EGIK_Any:
			JsonObject->SetStringField("filter_type", "any");
			break;
		case EGIK_All:
			JsonObject->SetStringField("filter_type", "all");
			break;
		case EGIK_Not:
			JsonObject->SetStringField("filter_type", "not");
			break;
		default:
			JsonObject->SetStringField("filter_type", "any");
			break;
		}

		return JsonObject;
	}
	
};


USTRUCT(BlueprintType)
struct FEGIK_Session
{
	GENERATED_BODY()

	//Unique UUID
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString SessionId;

	//Current status of the session
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Status;

	//If the session is linked to a Ready deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bReady;

	//If the session is linked to a deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bLinked;

	//Type of session created
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Kind;

	//Count of user this session currently have
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 UserCount;
	
};

USTRUCT(BlueprintType)
struct FEGIK_DeploymentPorts
{
	GENERATED_BODY()

	//The Port to Connect from Internet
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 External;

	//The internal Port of the Container
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Internal;
	
	//The Protocol (i.e. 'TCP')
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Protocol;

	//If the port require TLS Upgrade
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bTLS_Upgrade;
	
	//link of the port with scheme depending of the protocol
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Link;
	
	//Internal Proxy Mapping
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Proxy;


};

USTRUCT(BlueprintType)
struct FEGIK_DeploymentLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_LocationStruct Location;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_LatitudeLongitudeStruct LatitudeLongitude;
};

USTRUCT(BlueprintType)
struct FEGIK_DeploymentStatusAndInfoResponse
{
	GENERATED_BODY()

	//The Unique ID of the Deployment's request
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString RequestId;

	//The FQDN that allow to connect to your Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString FQDN;

	//The name of the deployed App
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString AppName;

	//The version of the deployed App
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString VersionName;

	//The current status of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString CurrentStatus;

	//True if the current Deployment is ready to be connected and running
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bRunning;

	//True if the current Deployment is ACL protected
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bWhitelistingActive;

	//Timestamp of the Deployment when it is up and running
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FDateTime StartTime;

	//Timestamp of the end of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FDateTime RemovalTime;

	//Time since the Deployment is up and running in seconds
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 ElapsedTime;

	//The last status of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString LastStatus;

	//True if there is an error with the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	bool bError;

	//The error details of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString ErrorDetails;

	//Ports
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TMap<FString, FEGIK_DeploymentPorts> Ports;

	//The public IP
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString PublicIP;

	//List of Active Sessions if Deployment App is Session Based
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FEGIK_Session> ActiveSessions;

	//Location
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FEGIK_DeploymentLocation Location;

	//List of tags associated with the deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	TArray<FString> Tags;

	//The Capacity of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 Sockets;

	//The Capacity Usage of the Deployment
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	int32 SocketsUsage;

	//The command to use in the container, null mean it will take the default of the container
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Command;

	//The arguments to use in the container, null mean it will take the default of the container
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Deployment")
	FString Arguments;

	FEGIK_DeploymentStatusAndInfoResponse()
	{
		RequestId = "";
		FQDN = "";
		AppName = "";
		VersionName = "";
		CurrentStatus = "";
		bRunning = false;
		bWhitelistingActive = false;
		StartTime = FDateTime();
		RemovalTime = FDateTime();
		ElapsedTime = 0;
		LastStatus = "";
		bError = false;
		ErrorDetails = "";
		PublicIP = "";
		Sockets = 0;
		SocketsUsage = 0;
		Command = "";
		Arguments = "";
	}

	FEGIK_DeploymentStatusAndInfoResponse(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			RequestId = JsonObject->GetStringField(TEXT("request_id"));
			FQDN = JsonObject->GetStringField(TEXT("fqdn"));
			AppName = JsonObject->GetStringField(TEXT("app_name"));
			VersionName = JsonObject->GetStringField(TEXT("app_version"));
			CurrentStatus = JsonObject->GetStringField(TEXT("current_status"));
			bRunning = JsonObject->GetBoolField(TEXT("running"));
			bWhitelistingActive = JsonObject->GetBoolField(TEXT("whitelisting_active"));
			FDateTime::Parse(JsonObject->GetStringField(TEXT("start_time")),StartTime);
			FDateTime::Parse(JsonObject->GetStringField(TEXT("removal_time")),RemovalTime);
			ElapsedTime = JsonObject->GetIntegerField(TEXT("elapsed_time"));
			LastStatus = JsonObject->GetStringField(TEXT("last_status"));
			bError = JsonObject->GetBoolField(TEXT("error"));
			if(JsonObject->HasField(TEXT("error_detail")))
			{
				ErrorDetails = JsonObject->GetStringField(TEXT("error_detail"));
			}
			TSharedPtr<FJsonObject> PortsObject = JsonObject->GetObjectField(TEXT("ports"));
			for (auto& PortEntry : PortsObject->Values)
			{
				FEGIK_DeploymentPorts PortData;
				TSharedPtr<FJsonObject> PortObject = PortEntry.Value->AsObject();
				PortData.External = PortObject->GetIntegerField(TEXT("external"));
				PortData.Internal = PortObject->GetIntegerField(TEXT("internal"));
				PortData.Protocol = PortObject->GetStringField(TEXT("protocol"));
				PortData.bTLS_Upgrade = PortObject->GetBoolField(TEXT("tls_upgrade"));
				PortData.Link = PortObject->GetStringField(TEXT("link"));
				PortData.Proxy = PortObject->GetIntegerField(TEXT("proxy"));
				Ports.Add(PortEntry.Key, PortData);
			}

			PublicIP = JsonObject->GetStringField(TEXT("public_ip"));
			if(JsonObject->HasField(TEXT("sessions")))
			{
				TArray<TSharedPtr<FJsonValue>> SessionsArray = JsonObject->GetArrayField(TEXT("sessions"));
				for (auto& SessionValue : SessionsArray)
				{
					TSharedPtr<FJsonObject> SessionObject = SessionValue->AsObject();
					FEGIK_Session SessionData;
					SessionData.SessionId = SessionObject->GetStringField(TEXT("session_id"));
					SessionData.Status = SessionObject->GetStringField(TEXT("status"));
					SessionData.bReady = SessionObject->GetBoolField(TEXT("ready"));
					SessionData.bLinked = SessionObject->GetBoolField(TEXT("linked"));
					SessionData.Kind = SessionObject->GetStringField(TEXT("kind"));
					SessionData.UserCount = SessionObject->GetIntegerField(TEXT("user_count"));
					ActiveSessions.Add(SessionData);
				}
			}
			// Parsing location data
			TSharedPtr<FJsonObject> LocationObject = JsonObject->GetObjectField(TEXT("location"));
			Location.Location.City = LocationObject->GetStringField(TEXT("city"));
			Location.Location.Country = LocationObject->GetStringField(TEXT("country"));
			Location.Location.Continent = LocationObject->GetStringField(TEXT("continent"));
			Location.Location.AdministrativeDivision = LocationObject->GetStringField(TEXT("administrative_division"));
			Location.Location.Timezone = LocationObject->GetStringField(TEXT("timezone"));
			Location.LatitudeLongitude.Latitude = LocationObject->GetNumberField(TEXT("latitude"));
			Location.LatitudeLongitude.Longitude = LocationObject->GetNumberField(TEXT("longitude"));

			// Parsing tags
			if(JsonObject->HasField(TEXT("tags")))
			{
				TArray<TSharedPtr<FJsonValue>> TagsArray = JsonObject->GetArrayField(TEXT("tags"));
				for (auto& TagValue : TagsArray)
				{
					Tags.Add(TagValue->AsString());
				}
			}
			// Sockets information
			if(JsonObject->HasField(TEXT("sockets")))
			{
				Sockets = JsonObject->GetIntegerField(TEXT("sockets"));
				SocketsUsage = JsonObject->GetIntegerField(TEXT("sockets_usage"));
			}
			// Command and Arguments
			Command = JsonObject->GetStringField(TEXT("command"));
			Arguments = JsonObject->GetStringField(TEXT("arguments"));
		}
	}
	
};

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIKBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

    // Convert the Error struct to a string
    UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = "Edgegap Integration Kit | Conversion")
    static FString Conv_EGIK_ErrorStructToString(FEGIK_ErrorStruct ErrorStruct);

    // Convert the Assignment struct to a string
    UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = "Edgegap Integration Kit | Conversion")
    static FString Conv_EGIK_AssignmentStructToString(FEGIK_AssignmentStruct AssignmentStruct);

    // Convert the Response struct to a string
    UFUNCTION(BlueprintPure, meta=(CompactNodeTitle = "->", BlueprintAutocast), Category = "Edgegap Integration Kit | Conversion")
    static FString Conv_EGIK_MatchmakingResponseToString(FEGIK_MatchmakingResponse ResponseStruct);

	//UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", FriendlyName = "Create Item Data", BlueprintInternalUseOnly = "true"), Category = "Game Inventory System")
	//static class UGISItemData* Create(UObject* WorldContextObject, TSubclassOf ItemType, APlayerController* OwningPlayer);
};
