// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_JsonKeyCompat.h"
#include "EGIK_ServerBrowserTypes.generated.h"

// Forward declarations
struct FEGIK_SB_ServerInstanceSlot;

/**
 * Optional overrides for Server Browser nodes.
 * Leave fields empty to auto-read from plugin settings.
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_Overrides
{
	GENERATED_BODY()

	/** Override the Server Browser URL (leave empty to use plugin settings) */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser", meta = (DisplayName = "Server Browser URL"))
	FString ServerBrowserURL;

	/** Override the auth token (leave empty to auto-detect from plugin settings) */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser", meta = (DisplayName = "Auth Token"))
	FString AuthToken;
};

/**
 * Server port information for Server Browser
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_ServerPort
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 Internal = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 External = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString Link;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString Protocol;

	FEGIK_SB_ServerPort() = default;

	FEGIK_SB_ServerPort(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			JsonObject->TryGetNumberField(TEXT("internal"), Internal);
			JsonObject->TryGetNumberField(TEXT("external"), External);
			JsonObject->TryGetStringField(TEXT("link"), Link);
			JsonObject->TryGetStringField(TEXT("protocol"), Protocol);
		}
	}

	TSharedPtr<FJsonObject> ToJsonObject() const
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetNumberField(TEXT("internal"), Internal);
		JsonObject->SetNumberField(TEXT("external"), External);
		JsonObject->SetStringField(TEXT("link"), Link);
		JsonObject->SetStringField(TEXT("protocol"), Protocol);
		return JsonObject;
	}
};

/**
 * Server location information for Server Browser
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_ServerLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString City;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString Country;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString Continent;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString AdministrativeDivision;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString Timezone;

	FEGIK_SB_ServerLocation() = default;

	FEGIK_SB_ServerLocation(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("city"), City);
			JsonObject->TryGetStringField(TEXT("country"), Country);
			JsonObject->TryGetStringField(TEXT("continent"), Continent);
			JsonObject->TryGetStringField(TEXT("administrative_division"), AdministrativeDivision);
			JsonObject->TryGetStringField(TEXT("timezone"), Timezone);
		}
	}

	TSharedPtr<FJsonObject> ToJsonObject() const
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField(TEXT("city"), City);
		JsonObject->SetStringField(TEXT("country"), Country);
		JsonObject->SetStringField(TEXT("continent"), Continent);
		JsonObject->SetStringField(TEXT("administrative_division"), AdministrativeDivision);
		JsonObject->SetStringField(TEXT("timezone"), Timezone);
		return JsonObject;
	}
};

/**
 * Server details for Server Browser instance
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_Server
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString FQDN;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString PublicIP;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TMap<FString, FEGIK_SB_ServerPort> Ports;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FEGIK_SB_ServerLocation Location;

	FEGIK_SB_Server() = default;

	FEGIK_SB_Server(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("fqdn"), FQDN);
			JsonObject->TryGetStringField(TEXT("public_ip"), PublicIP);

			const TSharedPtr<FJsonObject>* PortsObj;
			if (JsonObject->TryGetObjectField(TEXT("ports"), PortsObj))
			{
				for (const auto& Pair : (*PortsObj)->Values)
				{
					Ports.Add(EGIK_JSONKEY(Pair.Key), FEGIK_SB_ServerPort(Pair.Value->AsObject()));
				}
			}

			const TSharedPtr<FJsonObject>* LocationObj;
			if (JsonObject->TryGetObjectField(TEXT("location"), LocationObj))
			{
				Location = FEGIK_SB_ServerLocation(*LocationObj);
			}
		}
	}

	TSharedPtr<FJsonObject> ToJsonObject() const
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField(TEXT("fqdn"), FQDN);
		JsonObject->SetStringField(TEXT("public_ip"), PublicIP);

		TSharedPtr<FJsonObject> PortsObj = MakeShareable(new FJsonObject);
		for (const auto& Pair : Ports)
		{
			PortsObj->SetObjectField(Pair.Key, Pair.Value.ToJsonObject());
		}
		JsonObject->SetObjectField(TEXT("ports"), PortsObj);
		JsonObject->SetObjectField(TEXT("location"), Location.ToJsonObject());

		return JsonObject;
	}
};

/**
 * Server instance slot for Server Browser
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_ServerInstanceSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 AvailableSeats = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 ReservedSeats = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 JoinableSeats = 0;

	/** Custom metadata as JSON string (can contain any key-value pairs) */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString MetadataJson;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FDateTime CreatedAt;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FDateTime UpdatedAt;

	FEGIK_SB_ServerInstanceSlot() = default;

	FEGIK_SB_ServerInstanceSlot(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("name"), Name);
			JsonObject->TryGetNumberField(TEXT("available_seats"), AvailableSeats);
			JsonObject->TryGetNumberField(TEXT("reserved_seats"), ReservedSeats);
			JsonObject->TryGetNumberField(TEXT("joinable_seats"), JoinableSeats);

			const TSharedPtr<FJsonObject>* MetadataObj;
			if (JsonObject->TryGetObjectField(TEXT("metadata"), MetadataObj))
			{
				TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MetadataJson);
				FJsonSerializer::Serialize((*MetadataObj).ToSharedRef(), Writer);
			}

			FString CreatedAtStr, UpdatedAtStr;
			if (JsonObject->TryGetStringField(TEXT("created_at"), CreatedAtStr))
			{
				FDateTime::ParseIso8601(*CreatedAtStr, CreatedAt);
			}
			if (JsonObject->TryGetStringField(TEXT("updated_at"), UpdatedAtStr))
			{
				FDateTime::ParseIso8601(*UpdatedAtStr, UpdatedAt);
			}
		}
	}

	TSharedPtr<FJsonObject> ToJsonObject() const
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField(TEXT("name"), Name);
		JsonObject->SetNumberField(TEXT("available_seats"), AvailableSeats);

		// Parse metadata JSON string back to object
		if (!MetadataJson.IsEmpty())
		{
			TSharedPtr<FJsonObject> MetadataObj;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MetadataJson);
			if (FJsonSerializer::Deserialize(Reader, MetadataObj) && MetadataObj.IsValid())
			{
				JsonObject->SetObjectField(TEXT("metadata"), MetadataObj);
			}
		}
		else
		{
			JsonObject->SetObjectField(TEXT("metadata"), MakeShareable(new FJsonObject));
		}

		return JsonObject;
	}
};

/**
 * Server instance response from Server Browser
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_ServerInstance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString RequestId;

	/** Custom metadata as JSON string */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString MetadataJson;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FEGIK_SB_ServerInstanceSlot> Slots;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FEGIK_SB_Server Server;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FDateTime CreatedAt;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FDateTime UpdatedAt;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 TotalAvailableSeats = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 TotalReservedSeats = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 TotalJoinableSeats = 0;

	FEGIK_SB_ServerInstance() = default;

	FEGIK_SB_ServerInstance(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("request_id"), RequestId);

			const TSharedPtr<FJsonObject>* MetadataObj;
			if (JsonObject->TryGetObjectField(TEXT("metadata"), MetadataObj))
			{
				TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MetadataJson);
				FJsonSerializer::Serialize((*MetadataObj).ToSharedRef(), Writer);
			}

			const TArray<TSharedPtr<FJsonValue>>* SlotsArray;
			if (JsonObject->TryGetArrayField(TEXT("slots"), SlotsArray))
			{
				for (const auto& SlotValue : *SlotsArray)
				{
					Slots.Add(FEGIK_SB_ServerInstanceSlot(SlotValue->AsObject()));
				}
			}

			const TSharedPtr<FJsonObject>* ServerObj;
			if (JsonObject->TryGetObjectField(TEXT("server"), ServerObj))
			{
				Server = FEGIK_SB_Server(*ServerObj);
			}

			FString CreatedAtStr, UpdatedAtStr;
			if (JsonObject->TryGetStringField(TEXT("created_at"), CreatedAtStr))
			{
				FDateTime::ParseIso8601(*CreatedAtStr, CreatedAt);
			}
			if (JsonObject->TryGetStringField(TEXT("updated_at"), UpdatedAtStr))
			{
				FDateTime::ParseIso8601(*UpdatedAtStr, UpdatedAt);
			}

			JsonObject->TryGetNumberField(TEXT("total_available_seats"), TotalAvailableSeats);
			JsonObject->TryGetNumberField(TEXT("total_reserved_seats"), TotalReservedSeats);
			JsonObject->TryGetNumberField(TEXT("total_joinable_seats"), TotalJoinableSeats);
		}
	}
};

/**
 * Cursor pagination response
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_CursorPagination
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 PageSize = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString NextCursor;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString PreviousCursor;

	FEGIK_SB_CursorPagination() = default;

	FEGIK_SB_CursorPagination(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			JsonObject->TryGetNumberField(TEXT("page_size"), PageSize);
			JsonObject->TryGetStringField(TEXT("next_cursor"), NextCursor);
			JsonObject->TryGetStringField(TEXT("previous_cursor"), PreviousCursor);
		}
	}
};

/**
 * List server instances response
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_ListServerInstancesResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FEGIK_SB_ServerInstance> Items;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FEGIK_SB_CursorPagination Pagination;

	FEGIK_SB_ListServerInstancesResponse() = default;

	FEGIK_SB_ListServerInstancesResponse(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* ItemsArray;
			if (JsonObject->TryGetArrayField(TEXT("items"), ItemsArray))
			{
				for (const auto& ItemValue : *ItemsArray)
				{
					Items.Add(FEGIK_SB_ServerInstance(ItemValue->AsObject()));
				}
			}

			const TSharedPtr<FJsonObject>* PaginationObj;
			if (JsonObject->TryGetObjectField(TEXT("pagination"), PaginationObj))
			{
				Pagination = FEGIK_SB_CursorPagination(*PaginationObj);
			}
		}
	}
};

/**
 * List slots response
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_ListSlotsResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FEGIK_SB_ServerInstanceSlot> Items;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FEGIK_SB_CursorPagination Pagination;

	FEGIK_SB_ListSlotsResponse() = default;

	FEGIK_SB_ListSlotsResponse(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* ItemsArray;
			if (JsonObject->TryGetArrayField(TEXT("items"), ItemsArray))
			{
				for (const auto& ItemValue : *ItemsArray)
				{
					Items.Add(FEGIK_SB_ServerInstanceSlot(ItemValue->AsObject()));
				}
			}

			const TSharedPtr<FJsonObject>* PaginationObj;
			if (JsonObject->TryGetObjectField(TEXT("pagination"), PaginationObj))
			{
				Pagination = FEGIK_SB_CursorPagination(*PaginationObj);
			}
		}
	}
};

/**
 * Keep-alive response
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_KeepAliveResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FDateTime ExpiresAt;

	FEGIK_SB_KeepAliveResponse() = default;

	FEGIK_SB_KeepAliveResponse(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			FString ExpiresAtStr;
			if (JsonObject->TryGetStringField(TEXT("expires_at"), ExpiresAtStr))
			{
				FDateTime::ParseIso8601(*ExpiresAtStr, ExpiresAt);
			}
		}
	}
};

/**
 * User for seat reservation
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_ReservationUser
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString UserId;

	FEGIK_SB_ReservationUser() = default;

	FEGIK_SB_ReservationUser(const FString& InUserId) : UserId(InUserId) {}

	TSharedPtr<FJsonObject> ToJsonObject() const
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
		JsonObject->SetStringField(TEXT("user_id"), UserId);
		return JsonObject;
	}
};

/**
 * Slot confirmation response within seat confirmation
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_SeatConfirmationSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString Name;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FString> AcceptedUserIds;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FString> ExpiredUserIds;

	FEGIK_SB_SeatConfirmationSlot() = default;

	FEGIK_SB_SeatConfirmationSlot(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("name"), Name);

			const TArray<TSharedPtr<FJsonValue>>* AcceptedArray;
			if (JsonObject->TryGetArrayField(TEXT("accepted_user_ids"), AcceptedArray))
			{
				for (const auto& Value : *AcceptedArray)
				{
					AcceptedUserIds.Add(Value->AsString());
				}
			}

			const TArray<TSharedPtr<FJsonValue>>* ExpiredArray;
			if (JsonObject->TryGetArrayField(TEXT("expired_user_ids"), ExpiredArray))
			{
				for (const auto& Value : *ExpiredArray)
				{
					ExpiredUserIds.Add(Value->AsString());
				}
			}
		}
	}
};

/**
 * Seat reservation confirmation response
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_SeatConfirmationResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FString> UnknownUserIds;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FEGIK_SB_SeatConfirmationSlot> Slots;

	FEGIK_SB_SeatConfirmationResponse() = default;

	FEGIK_SB_SeatConfirmationResponse(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* UnknownArray;
			if (JsonObject->TryGetArrayField(TEXT("unknown_user_ids"), UnknownArray))
			{
				for (const auto& Value : *UnknownArray)
				{
					UnknownUserIds.Add(Value->AsString());
				}
			}

			const TArray<TSharedPtr<FJsonValue>>* SlotsArray;
			if (JsonObject->TryGetArrayField(TEXT("slots"), SlotsArray))
			{
				for (const auto& SlotValue : *SlotsArray)
				{
					Slots.Add(FEGIK_SB_SeatConfirmationSlot(SlotValue->AsObject()));
				}
			}
		}
	}
};

/**
 * Monitor/health check response
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_MonitorResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString Status;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	int32 UnhealthyServices = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FString> Reasons;

	FEGIK_SB_MonitorResponse() = default;

	FEGIK_SB_MonitorResponse(TSharedPtr<FJsonObject> JsonObject)
	{
		if (JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("status"), Status);
			JsonObject->TryGetNumberField(TEXT("unhealthy_services"), UnhealthyServices);

			const TArray<TSharedPtr<FJsonValue>>* ReasonsArray;
			if (JsonObject->TryGetArrayField(TEXT("reasons"), ReasonsArray))
			{
				for (const auto& Value : *ReasonsArray)
				{
					Reasons.Add(Value->AsString());
				}
			}
		}
	}
};
