// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Dom/JsonObject.h"
#include "EGIK_MatchmakingAttributes.generated.h"

/** Value types accepted by Edgegap matchmaking profile rules. */
UENUM(BlueprintType)
enum class EEGIK_MatchmakingAttributeType : uint8
{
	String UMETA(DisplayName = "String"),
	Number UMETA(DisplayName = "Number"),
	Boolean UMETA(DisplayName = "Boolean"),
	StringArray UMETA(DisplayName = "String Array / Intersection"),
	NumberMap UMETA(DisplayName = "Number Map / Beacon Latencies")
};

/**
 * One type-safe matchmaking attribute. Use the Make Matchmaking ... Attribute
 * Blueprint nodes so no JSON syntax is required.
 */
USTRUCT(BlueprintType)
struct EDGEGAPINTEGRATIONKIT_API FEGIK_MatchmakingAttribute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Edgegap Integration Kit | Matchmaking")
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Edgegap Integration Kit | Matchmaking")
	EEGIK_MatchmakingAttributeType Type = EEGIK_MatchmakingAttributeType::String;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Edgegap Integration Kit | Matchmaking", meta = (EditCondition = "Type == EEGIK_MatchmakingAttributeType::String", EditConditionHides))
	FString StringValue;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Edgegap Integration Kit | Matchmaking", meta = (EditCondition = "Type == EEGIK_MatchmakingAttributeType::Number", EditConditionHides))
	double NumberValue = 0.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Edgegap Integration Kit | Matchmaking", meta = (EditCondition = "Type == EEGIK_MatchmakingAttributeType::Boolean", EditConditionHides))
	bool bBooleanValue = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Edgegap Integration Kit | Matchmaking", meta = (EditCondition = "Type == EEGIK_MatchmakingAttributeType::StringArray", EditConditionHides))
	TArray<FString> StringArrayValue;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Edgegap Integration Kit | Matchmaking", meta = (EditCondition = "Type == EEGIK_MatchmakingAttributeType::NumberMap", EditConditionHides))
	TMap<FString, double> NumberMapValue;

	TSharedPtr<FJsonValue> ToJsonValue() const;
};

/** Type-safe attributes to use instead of manually authored JSON. */
USTRUCT(BlueprintType)
struct EDGEGAPINTEGRATIONKIT_API FEGIK_MatchmakingAttributes
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Edgegap Integration Kit | Matchmaking")
	TArray<FEGIK_MatchmakingAttribute> Values;

	bool IsEmpty() const { return Values.IsEmpty(); }
	bool ToJsonObject(TSharedPtr<FJsonObject>& OutObject, FString& OutErrorMessage) const;
};

/** Convenient Blueprint constructors for type-safe matchmaking attributes. */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_MatchmakingAttributeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Edgegap Integration Kit | Matchmaking | Attributes", meta = (DisplayName = "Make Matchmaking String Attribute"))
	static FEGIK_MatchmakingAttribute MakeStringAttribute(const FString& Name, const FString& Value);

	UFUNCTION(BlueprintPure, Category = "Edgegap Integration Kit | Matchmaking | Attributes", meta = (DisplayName = "Make Matchmaking Number Attribute"))
	static FEGIK_MatchmakingAttribute MakeNumberAttribute(const FString& Name, double Value);

	UFUNCTION(BlueprintPure, Category = "Edgegap Integration Kit | Matchmaking | Attributes", meta = (DisplayName = "Make Matchmaking Boolean Attribute"))
	static FEGIK_MatchmakingAttribute MakeBooleanAttribute(const FString& Name, bool Value);

	UFUNCTION(BlueprintPure, Category = "Edgegap Integration Kit | Matchmaking | Attributes", meta = (DisplayName = "Make Matchmaking String Array Attribute"))
	static FEGIK_MatchmakingAttribute MakeStringArrayAttribute(const FString& Name, const TArray<FString>& Values);

	UFUNCTION(BlueprintPure, Category = "Edgegap Integration Kit | Matchmaking | Attributes", meta = (DisplayName = "Make Matchmaking Beacon Latencies Attribute", AutoCreateRefTerm = "BeaconLatencies"))
	static FEGIK_MatchmakingAttribute MakeBeaconLatenciesAttribute(const TMap<FString, double>& BeaconLatencies, const FString& Name = TEXT("beacons"));

	/** Useful when calling an older API that still accepts only an Attributes JSON string. */
	UFUNCTION(BlueprintPure, Category = "Edgegap Integration Kit | Matchmaking | Attributes", meta = (DisplayName = "Matchmaking Attributes To JSON"))
	static FString ToJsonString(const FEGIK_MatchmakingAttributes& Attributes, bool& bSuccess, FString& ErrorMessage);
};

namespace EGIK_MatchmakingAttributeUtils
{
	/** Build either typed or raw attributes. Supplying both is rejected to avoid hidden precedence. */
	EDGEGAPINTEGRATIONKIT_API bool Build(
		const FEGIK_MatchmakingAttributes& StructuredAttributes,
		const FString& RawAttributesJson,
		TSharedPtr<FJsonObject>& OutObject,
		FString& OutErrorMessage);
}
