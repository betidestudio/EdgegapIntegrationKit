// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_MatchmakingAttributes.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

TSharedPtr<FJsonValue> FEGIK_MatchmakingAttribute::ToJsonValue() const
{
	switch (Type)
	{
	case EEGIK_MatchmakingAttributeType::String:
		return MakeShared<FJsonValueString>(StringValue);
	case EEGIK_MatchmakingAttributeType::Number:
		return MakeShared<FJsonValueNumber>(NumberValue);
	case EEGIK_MatchmakingAttributeType::Boolean:
		return MakeShared<FJsonValueBoolean>(bBooleanValue);
	case EEGIK_MatchmakingAttributeType::StringArray:
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		JsonValues.Reserve(StringArrayValue.Num());
		for (const FString& Value : StringArrayValue)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		return MakeShared<FJsonValueArray>(JsonValues);
	}
	case EEGIK_MatchmakingAttributeType::NumberMap:
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		for (const TPair<FString, double>& Pair : NumberMapValue)
		{
			JsonObject->SetNumberField(Pair.Key, Pair.Value);
		}
		return MakeShared<FJsonValueObject>(JsonObject);
	}
	default:
		return nullptr;
	}
}

bool FEGIK_MatchmakingAttributes::ToJsonObject(TSharedPtr<FJsonObject>& OutObject, FString& OutErrorMessage) const
{
	OutObject = MakeShared<FJsonObject>();
	TSet<FString> AttributeNames;

	for (int32 Index = 0; Index < Values.Num(); ++Index)
	{
		const FEGIK_MatchmakingAttribute& Attribute = Values[Index];
		FString AttributeName = Attribute.Name;
		AttributeName.TrimStartAndEndInline();

		if (AttributeName.IsEmpty())
		{
			OutErrorMessage = FString::Printf(TEXT("Structured Attributes entry %d has an empty Name."), Index);
			return false;
		}
		if (!Attribute.Name.Equals(AttributeName, ESearchCase::CaseSensitive))
		{
			OutErrorMessage = FString::Printf(TEXT("Structured attribute '%s' has leading or trailing whitespace in its Name."), *Attribute.Name);
			return false;
		}
		if (AttributeNames.Contains(AttributeName))
		{
			OutErrorMessage = FString::Printf(TEXT("Structured Attributes contains the duplicate key '%s'."), *AttributeName);
			return false;
		}
		if (Attribute.Type == EEGIK_MatchmakingAttributeType::Number && !FMath::IsFinite(Attribute.NumberValue))
		{
			OutErrorMessage = FString::Printf(TEXT("Structured number attribute '%s' must be finite (not NaN or Infinity)."), *AttributeName);
			return false;
		}
		if (Attribute.Type == EEGIK_MatchmakingAttributeType::StringArray)
		{
			if (Attribute.StringArrayValue.IsEmpty())
			{
				OutErrorMessage = FString::Printf(TEXT("Structured string-array attribute '%s' must contain at least one value."), *AttributeName);
				return false;
			}
			for (const FString& Value : Attribute.StringArrayValue)
			{
				if (Value.IsEmpty())
				{
					OutErrorMessage = FString::Printf(TEXT("Structured string-array attribute '%s' contains an empty value."), *AttributeName);
					return false;
				}
			}
		}
		if (Attribute.Type == EEGIK_MatchmakingAttributeType::NumberMap)
		{
			if (Attribute.NumberMapValue.IsEmpty())
			{
				OutErrorMessage = FString::Printf(TEXT("Structured beacon-latencies attribute '%s' must contain at least one beacon."), *AttributeName);
				return false;
			}
			for (const TPair<FString, double>& Pair : Attribute.NumberMapValue)
			{
				FString BeaconName = Pair.Key;
				BeaconName.TrimStartAndEndInline();
				if (BeaconName.IsEmpty() || !Pair.Key.Equals(BeaconName, ESearchCase::CaseSensitive))
				{
					OutErrorMessage = FString::Printf(TEXT("Structured beacon-latencies attribute '%s' contains an empty or whitespace-padded beacon name."), *AttributeName);
					return false;
				}
				if (!FMath::IsFinite(Pair.Value) || Pair.Value < 0.0)
				{
					OutErrorMessage = FString::Printf(TEXT("Latency for beacon '%s' must be a finite, non-negative number."), *Pair.Key);
					return false;
				}
			}
		}

		const TSharedPtr<FJsonValue> JsonValue = Attribute.ToJsonValue();
		if (!JsonValue.IsValid())
		{
			OutErrorMessage = FString::Printf(TEXT("Structured attribute '%s' has an unsupported value type."), *AttributeName);
			return false;
		}

		AttributeNames.Add(AttributeName);
		OutObject->SetField(AttributeName, JsonValue);
	}

	return true;
}

FEGIK_MatchmakingAttribute UEGIK_MatchmakingAttributeLibrary::MakeStringAttribute(const FString& Name, const FString& Value)
{
	FEGIK_MatchmakingAttribute Attribute;
	Attribute.Name = Name;
	Attribute.Type = EEGIK_MatchmakingAttributeType::String;
	Attribute.StringValue = Value;
	return Attribute;
}

FEGIK_MatchmakingAttribute UEGIK_MatchmakingAttributeLibrary::MakeNumberAttribute(const FString& Name, double Value)
{
	FEGIK_MatchmakingAttribute Attribute;
	Attribute.Name = Name;
	Attribute.Type = EEGIK_MatchmakingAttributeType::Number;
	Attribute.NumberValue = Value;
	return Attribute;
}

FEGIK_MatchmakingAttribute UEGIK_MatchmakingAttributeLibrary::MakeBooleanAttribute(const FString& Name, bool Value)
{
	FEGIK_MatchmakingAttribute Attribute;
	Attribute.Name = Name;
	Attribute.Type = EEGIK_MatchmakingAttributeType::Boolean;
	Attribute.bBooleanValue = Value;
	return Attribute;
}

FEGIK_MatchmakingAttribute UEGIK_MatchmakingAttributeLibrary::MakeStringArrayAttribute(const FString& Name, const TArray<FString>& Values)
{
	FEGIK_MatchmakingAttribute Attribute;
	Attribute.Name = Name;
	Attribute.Type = EEGIK_MatchmakingAttributeType::StringArray;
	Attribute.StringArrayValue = Values;
	return Attribute;
}

FEGIK_MatchmakingAttribute UEGIK_MatchmakingAttributeLibrary::MakeBeaconLatenciesAttribute(const TMap<FString, double>& BeaconLatencies, const FString& Name)
{
	FEGIK_MatchmakingAttribute Attribute;
	Attribute.Name = Name;
	Attribute.Type = EEGIK_MatchmakingAttributeType::NumberMap;
	Attribute.NumberMapValue = BeaconLatencies;
	return Attribute;
}

FString UEGIK_MatchmakingAttributeLibrary::ToJsonString(const FEGIK_MatchmakingAttributes& Attributes, bool& bSuccess, FString& ErrorMessage)
{
	TSharedPtr<FJsonObject> JsonObject;
	bSuccess = Attributes.ToJsonObject(JsonObject, ErrorMessage);
	if (!bSuccess)
	{
		return TEXT("");
	}

	FString JsonString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	bSuccess = FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	if (!bSuccess)
	{
		ErrorMessage = TEXT("Failed to serialize matchmaking attributes.");
		return TEXT("");
	}

	ErrorMessage.Empty();
	return JsonString;
}

bool EGIK_MatchmakingAttributeUtils::Build(
	const FEGIK_MatchmakingAttributes& StructuredAttributes,
	const FString& RawAttributesJson,
	TSharedPtr<FJsonObject>& OutObject,
	FString& OutErrorMessage)
{
	FString RawJson = RawAttributesJson;
	RawJson.TrimStartAndEndInline();

	if (!StructuredAttributes.IsEmpty() && !RawJson.IsEmpty())
	{
		OutErrorMessage = TEXT("Set either Structured Attributes or raw Attributes JSON, not both.");
		return false;
	}

	if (!StructuredAttributes.IsEmpty())
	{
		return StructuredAttributes.ToJsonObject(OutObject, OutErrorMessage);
	}

	if (RawJson.IsEmpty())
	{
		OutObject = MakeShared<FJsonObject>();
		return true;
	}

	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawJson);
	if (!FJsonSerializer::Deserialize(Reader, OutObject) || !OutObject.IsValid())
	{
		OutErrorMessage = FString::Printf(
			TEXT("Attributes must be a valid JSON object. Parser error: %s"),
			*Reader->GetErrorMessage());
		return false;
	}

	return true;
}
