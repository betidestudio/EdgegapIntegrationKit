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
