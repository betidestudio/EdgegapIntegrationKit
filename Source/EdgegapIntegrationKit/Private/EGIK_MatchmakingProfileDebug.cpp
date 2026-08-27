// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_MatchmakingProfileDebug.h"

#include "EGIK_AsyncRequestBase.h"
#include "EGIK_JsonKeyCompat.h"
#include "EGIK_MatchmakingRequestUtils.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace EGIK_MatchmakingProfileDebug
{
	enum class EAttributeKind : uint8
	{
		Unknown,
		String,
		Number,
		Boolean,
		StringArray,
		NumberMap
	};

	struct FAttributeRequirement
	{
		FString Name;
		EAttributeKind Kind = EAttributeKind::Unknown;
		bool bRequired = false;
	};

	struct FProfileRequirement
	{
		FString Name;
		TMap<FString, FAttributeRequirement> Attributes;
		bool bAllowsUnknownAttributes = true;
	};

	struct FSchemaSnapshot
	{
		TMap<FString, FProfileRequirement> Profiles;
	};

	struct FPendingDiagnostic
	{
		FString RequestContext;
		FString Profile;
		FEGIK_MatchmakingAttributes StructuredAttributes;
		FString RawAttributesJson;
	};

	struct FCacheEntry
	{
		FSchemaSnapshot Snapshot;
		TArray<FPendingDiagnostic> PendingDiagnostics;
		TSharedPtr<IHttpRequest> ActiveRequest;
		double CachedAtSeconds = 0.0;
		double LastFailureAtSeconds = 0.0;
		FString LastFailure;
		bool bHasSnapshot = false;
		bool bRequestInFlight = false;
	};

	static constexpr double SchemaCacheLifetimeSeconds = 60.0;
	static constexpr double FailureCooldownSeconds = 10.0;
	static TMap<FString, TSharedPtr<FCacheEntry>> SchemaCache;

	static const TSharedPtr<FJsonObject>* TryGetObject(const TSharedPtr<FJsonObject>& Object, const FString& FieldName)
	{
		if (!Object.IsValid())
		{
			return nullptr;
		}

		const TSharedPtr<FJsonObject>* Result = nullptr;
		return Object->TryGetObjectField(FieldName, Result) && Result && (*Result).IsValid()
			? Result
			: nullptr;
	}

	static TSharedPtr<FJsonObject> ResolveSchemaReference(
		const TSharedPtr<FJsonObject>& RootObject,
		const TSharedPtr<FJsonObject>& SchemaObject)
	{
		if (!SchemaObject.IsValid())
		{
			return nullptr;
		}

		FString Reference;
		if (!SchemaObject->TryGetStringField(TEXT("$ref"), Reference))
		{
			return SchemaObject;
		}

		const FString Prefix = TEXT("#/components/schemas/");
		if (!Reference.StartsWith(Prefix))
		{
			return nullptr;
		}

		const TSharedPtr<FJsonObject>* Components = TryGetObject(RootObject, TEXT("components"));
		const TSharedPtr<FJsonObject>* Schemas = Components ? TryGetObject(*Components, TEXT("schemas")) : nullptr;
		const TSharedPtr<FJsonObject>* Resolved = Schemas
			? TryGetObject(*Schemas, Reference.RightChop(Prefix.Len()))
			: nullptr;
		return Resolved ? *Resolved : nullptr;
	}

	static EAttributeKind GetExpectedKind(const FString& AttributeName, const TSharedPtr<FJsonObject>& PropertyObject)
	{
		FString Type;
		PropertyObject->TryGetStringField(TEXT("type"), Type);

		if (Type.Equals(TEXT("string"), ESearchCase::IgnoreCase))
		{
			return EAttributeKind::String;
		}
		if (Type.Equals(TEXT("number"), ESearchCase::IgnoreCase)
			|| Type.Equals(TEXT("integer"), ESearchCase::IgnoreCase))
		{
			return EAttributeKind::Number;
		}
		if (Type.Equals(TEXT("boolean"), ESearchCase::IgnoreCase))
		{
			return EAttributeKind::Boolean;
		}
		if (Type.Equals(TEXT("array"), ESearchCase::IgnoreCase))
		{
			const TSharedPtr<FJsonObject>* Items = TryGetObject(PropertyObject, TEXT("items"));
			FString ItemType;
			if (Items && (*Items)->TryGetStringField(TEXT("type"), ItemType)
				&& ItemType.Equals(TEXT("string"), ESearchCase::IgnoreCase))
			{
				return EAttributeKind::StringArray;
			}
			return EAttributeKind::Unknown;
		}
		if (Type.Equals(TEXT("object"), ESearchCase::IgnoreCase))
		{
			FString Description;
			PropertyObject->TryGetStringField(TEXT("description"), Description);
			if (AttributeName.Equals(TEXT("beacons"), ESearchCase::IgnoreCase)
				|| Description.Contains(TEXT("latenc"), ESearchCase::IgnoreCase))
			{
				// Edgegap's generated 3.2.x OpenAPI examples describe numeric beacon
				// map values as strings while showing and accepting numeric examples.
				return EAttributeKind::NumberMap;
			}
		}

		return EAttributeKind::Unknown;
	}

	static EAttributeKind GetActualKind(const FEGIK_MatchmakingAttribute& Attribute)
	{
		switch (Attribute.Type)
		{
		case EEGIK_MatchmakingAttributeType::String:
			return EAttributeKind::String;
		case EEGIK_MatchmakingAttributeType::Number:
			return EAttributeKind::Number;
		case EEGIK_MatchmakingAttributeType::Boolean:
			return EAttributeKind::Boolean;
		case EEGIK_MatchmakingAttributeType::StringArray:
			return EAttributeKind::StringArray;
		case EEGIK_MatchmakingAttributeType::NumberMap:
			return EAttributeKind::NumberMap;
		default:
			return EAttributeKind::Unknown;
		}
	}

	static FString KindToString(EAttributeKind Kind)
	{
		switch (Kind)
		{
		case EAttributeKind::String: return TEXT("String");
		case EAttributeKind::Number: return TEXT("Number");
		case EAttributeKind::Boolean: return TEXT("Boolean");
		case EAttributeKind::StringArray: return TEXT("String Array");
		case EAttributeKind::NumberMap: return TEXT("Number Map / Beacon Latencies");
		default: return TEXT("Unknown");
		}
	}

	static bool ParseVariant(
		const TSharedPtr<FJsonObject>& RootObject,
		const TSharedPtr<FJsonObject>& UnresolvedVariant,
		FProfileRequirement& OutProfile)
	{
		const TSharedPtr<FJsonObject> Variant = ResolveSchemaReference(RootObject, UnresolvedVariant);
		const TSharedPtr<FJsonObject>* Properties = TryGetObject(Variant, TEXT("properties"));
		const TSharedPtr<FJsonObject>* ProfileProperty = Properties ? TryGetObject(*Properties, TEXT("profile")) : nullptr;
		if (!Properties || !ProfileProperty)
		{
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* ProfileEnum = nullptr;
		if (!(*ProfileProperty)->TryGetArrayField(TEXT("enum"), ProfileEnum) || !ProfileEnum || ProfileEnum->IsEmpty())
		{
			return false;
		}

		OutProfile.Name = (*ProfileEnum)[0]->AsString();
		if (OutProfile.Name.IsEmpty())
		{
			return false;
		}

		const TSharedPtr<FJsonObject>* Attributes = TryGetObject(*Properties, TEXT("attributes"));
		if (!Attributes)
		{
			return true;
		}

		bool bAdditionalProperties = true;
		if ((*Attributes)->TryGetBoolField(TEXT("additionalProperties"), bAdditionalProperties))
		{
			OutProfile.bAllowsUnknownAttributes = bAdditionalProperties;
		}

		TSet<FString> RequiredNames;
		const TArray<TSharedPtr<FJsonValue>>* RequiredValues = nullptr;
		if ((*Attributes)->TryGetArrayField(TEXT("required"), RequiredValues) && RequiredValues)
		{
			for (const TSharedPtr<FJsonValue>& RequiredValue : *RequiredValues)
			{
				if (RequiredValue.IsValid() && RequiredValue->Type == EJson::String)
				{
					RequiredNames.Add(RequiredValue->AsString());
				}
			}
		}

		const TSharedPtr<FJsonObject>* AttributeProperties = TryGetObject(*Attributes, TEXT("properties"));
		if (!AttributeProperties)
		{
			return true;
		}

		for (const auto& PropertyPair : (*AttributeProperties)->Values)
		{
			if (!PropertyPair.Value.IsValid() || PropertyPair.Value->Type != EJson::Object)
			{
				continue;
			}

			const TSharedPtr<FJsonObject> PropertyObject = PropertyPair.Value->AsObject();
			const FString PropertyName = EGIK_JSONKEY(PropertyPair.Key);
			FAttributeRequirement Requirement;
			Requirement.Name = PropertyName;
			Requirement.Kind = GetExpectedKind(PropertyName, PropertyObject);
			Requirement.bRequired = RequiredNames.Contains(PropertyName);
			OutProfile.Attributes.Add(PropertyName, Requirement);
		}

		return true;
	}

	static bool ParseOpenApiSchema(const FString& JsonString, FSchemaSnapshot& OutSnapshot, FString& OutError)
	{
		TSharedPtr<FJsonObject> RootObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
		{
			OutError = FString::Printf(TEXT("Swagger response was not valid JSON: %s"), *Reader->GetErrorMessage());
			return false;
		}

		const TSharedPtr<FJsonObject>* Paths = TryGetObject(RootObject, TEXT("paths"));
		const TSharedPtr<FJsonObject>* TicketsPath = Paths ? TryGetObject(*Paths, TEXT("/tickets")) : nullptr;
		const TSharedPtr<FJsonObject>* Post = TicketsPath ? TryGetObject(*TicketsPath, TEXT("post")) : nullptr;
		const TSharedPtr<FJsonObject>* RequestBody = Post ? TryGetObject(*Post, TEXT("requestBody")) : nullptr;
		const TSharedPtr<FJsonObject>* Content = RequestBody ? TryGetObject(*RequestBody, TEXT("content")) : nullptr;
		const TSharedPtr<FJsonObject>* JsonContent = Content ? TryGetObject(*Content, TEXT("application/json")) : nullptr;
		const TSharedPtr<FJsonObject>* SchemaField = JsonContent ? TryGetObject(*JsonContent, TEXT("schema")) : nullptr;
		const TSharedPtr<FJsonObject> Schema = SchemaField ? ResolveSchemaReference(RootObject, *SchemaField) : nullptr;
		if (!Schema.IsValid())
		{
			OutError = TEXT("Swagger did not contain a POST /tickets application/json request schema.");
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* Variants = nullptr;
		if (Schema->TryGetArrayField(TEXT("oneOf"), Variants) && Variants)
		{
			for (const TSharedPtr<FJsonValue>& VariantValue : *Variants)
			{
				if (!VariantValue.IsValid() || VariantValue->Type != EJson::Object)
				{
					continue;
				}
				FProfileRequirement Profile;
				if (ParseVariant(RootObject, VariantValue->AsObject(), Profile))
				{
					const FString ProfileName = Profile.Name;
					OutSnapshot.Profiles.Add(ProfileName, MoveTemp(Profile));
				}
			}
		}
		else
		{
			FProfileRequirement Profile;
			if (ParseVariant(RootObject, Schema, Profile))
			{
				const FString ProfileName = Profile.Name;
				OutSnapshot.Profiles.Add(ProfileName, MoveTemp(Profile));
			}
		}

		if (OutSnapshot.Profiles.IsEmpty())
		{
			OutError = TEXT("Swagger ticket schema did not expose any profile variants.");
			return false;
		}
		return true;
	}

	static FString BuildReport(
		const FSchemaSnapshot& Snapshot,
		const FPendingDiagnostic& Diagnostic,
		bool& bOutHasMisalignment)
	{
		bOutHasMisalignment = false;
		TArray<FString> Lines;
		Lines.Add(FString::Printf(
			TEXT("[ProfileDebug][%s] Profile '%s'"),
			*Diagnostic.RequestContext,
			*Diagnostic.Profile));

		const FProfileRequirement* Profile = Snapshot.Profiles.Find(Diagnostic.Profile);
		if (!Profile)
		{
			bOutHasMisalignment = true;
			TArray<FString> AvailableProfiles;
			Snapshot.Profiles.GetKeys(AvailableProfiles);
			AvailableProfiles.Sort();
			Lines.Add(FString::Printf(
				TEXT("[PROFILE] Not found. Available profiles: %s"),
				AvailableProfiles.IsEmpty() ? TEXT("<none>") : *FString::Join(AvailableProfiles, TEXT(", "))));
			return FString::Join(Lines, TEXT("\n"));
		}

		FString RawJson = Diagnostic.RawAttributesJson;
		RawJson.TrimStartAndEndInline();
		if (!RawJson.IsEmpty())
		{
			Lines.Add(TEXT("[SKIPPED] Raw Attributes JSON is in use; this diagnostic compares StructuredAttributes only."));
			return FString::Join(Lines, TEXT("\n"));
		}

		TMap<FString, const FEGIK_MatchmakingAttribute*> ActualAttributes;
		TSet<FString> DuplicateNames;
		for (const FEGIK_MatchmakingAttribute& Attribute : Diagnostic.StructuredAttributes.Values)
		{
			if (ActualAttributes.Contains(Attribute.Name))
			{
				DuplicateNames.Add(Attribute.Name);
			}
			else
			{
				ActualAttributes.Add(Attribute.Name, &Attribute);
			}
		}

		TArray<FString> ExpectedNames;
		Profile->Attributes.GetKeys(ExpectedNames);
		ExpectedNames.Sort();
		for (const FString& ExpectedName : ExpectedNames)
		{
			const FAttributeRequirement& Requirement = Profile->Attributes.FindChecked(ExpectedName);
			const FEGIK_MatchmakingAttribute* const* Actual = ActualAttributes.Find(ExpectedName);
			if (!Actual)
			{
				if (Requirement.bRequired)
				{
					bOutHasMisalignment = true;
					Lines.Add(FString::Printf(
						TEXT("[MISSING] %s: required %s"),
						*ExpectedName,
						*KindToString(Requirement.Kind)));
				}
				continue;
			}

			const EAttributeKind ActualKind = GetActualKind(**Actual);
			if (Requirement.Kind != EAttributeKind::Unknown && Requirement.Kind != ActualKind)
			{
				bOutHasMisalignment = true;
				Lines.Add(FString::Printf(
					TEXT("[TYPE] %s: expected %s, received %s"),
					*ExpectedName,
					*KindToString(Requirement.Kind),
					*KindToString(ActualKind)));
			}
			else
			{
				Lines.Add(FString::Printf(
					TEXT("[OK] %s: %s"),
					*ExpectedName,
					*KindToString(ActualKind)));
			}
		}

		TArray<FString> ActualNames;
		ActualAttributes.GetKeys(ActualNames);
		ActualNames.Sort();
		for (const FString& ActualName : ActualNames)
		{
			if (!Profile->Attributes.Contains(ActualName) && !Profile->bAllowsUnknownAttributes)
			{
				bOutHasMisalignment = true;
				Lines.Add(FString::Printf(TEXT("[UNKNOWN] %s: not expected by this profile"), *ActualName));
			}
		}

		TArray<FString> SortedDuplicateNames = DuplicateNames.Array();
		SortedDuplicateNames.Sort();
		for (const FString& DuplicateName : SortedDuplicateNames)
		{
			bOutHasMisalignment = true;
			Lines.Add(FString::Printf(TEXT("[DUPLICATE] %s: supplied more than once"), *DuplicateName));
		}

		if (Lines.Num() == 1)
		{
			Lines.Add(TEXT("[OK] This profile expects no structured ticket attributes."));
		}

		return FString::Join(Lines, TEXT("\n"));
	}

	static void LogReport(const FSchemaSnapshot& Snapshot, const FPendingDiagnostic& Diagnostic)
	{
		bool bHasMisalignment = false;
		const FString Report = BuildReport(Snapshot, Diagnostic, bHasMisalignment);
		if (bHasMisalignment)
		{
			UE_LOG(LogEdgegap, Warning, TEXT("%s\n[ProfileDebug] Matchmaking request is still being submitted."), *Report);
		}
		else
		{
			UE_LOG(LogEdgegap, Log, TEXT("%s"), *Report);
		}
	}

	static void LogUnavailable(const FPendingDiagnostic& Diagnostic, const FString& Error)
	{
		UE_LOG(
			LogEdgegap,
			Warning,
			TEXT("[ProfileDebug][%s] Profile debug unavailable: %s. Matchmaking request was not delayed or blocked."),
			*Diagnostic.RequestContext,
			*Error);
	}

	static void CompleteSchemaRequest(
		const FString& CacheKey,
		const TSharedPtr<IHttpResponse>& Response,
		bool bConnectedSuccessfully)
	{
		const TSharedPtr<FCacheEntry>* FoundEntry = SchemaCache.Find(CacheKey);
		if (!FoundEntry || !FoundEntry->IsValid())
		{
			return;
		}

		const TSharedPtr<FCacheEntry> Entry = *FoundEntry;
		Entry->bRequestInFlight = false;
		Entry->ActiveRequest.Reset();
		TArray<FPendingDiagnostic> Pending = MoveTemp(Entry->PendingDiagnostics);
		Entry->PendingDiagnostics.Reset();

		FString Error;
		FSchemaSnapshot ParsedSnapshot;
		const int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : 0;
		const bool bHttpSuccess = bConnectedSuccessfully
			&& Response.IsValid()
			&& EHttpResponseCodes::IsOk(ResponseCode);
		if (!bHttpSuccess)
		{
			Error = Response.IsValid()
				? FString::Printf(TEXT("Swagger request returned HTTP %d"), ResponseCode)
				: TEXT("Swagger request could not connect");
		}
		else if (!ParseOpenApiSchema(Response->GetContentAsString(), ParsedSnapshot, Error))
		{
			// ParseOpenApiSchema supplies a useful error.
		}
		else
		{
			Entry->Snapshot = MoveTemp(ParsedSnapshot);
			Entry->bHasSnapshot = true;
			Entry->CachedAtSeconds = FPlatformTime::Seconds();
			Entry->LastFailure.Empty();
			for (const FPendingDiagnostic& Diagnostic : Pending)
			{
				LogReport(Entry->Snapshot, Diagnostic);
			}
			return;
		}

		Entry->LastFailure = Error;
		Entry->LastFailureAtSeconds = FPlatformTime::Seconds();
		for (const FPendingDiagnostic& Diagnostic : Pending)
		{
			LogUnavailable(Diagnostic, Error);
		}
	}

	void LogProfileAlignment(
		const FString& MatchmakingURL,
		const FString& AuthToken,
		const FString& RequestContext,
		const FString& Profile,
		const FEGIK_MatchmakingAttributes& StructuredAttributes,
		const FString& RawAttributesJson)
	{
		const FString ResolvedURL = EGIK_MatchmakingRequest::ResolveURL(MatchmakingURL);
		FPendingDiagnostic Diagnostic;
		Diagnostic.RequestContext = RequestContext;
		Diagnostic.Profile = Profile;
		Diagnostic.StructuredAttributes = StructuredAttributes;
		Diagnostic.RawAttributesJson = RawAttributesJson;

		if (ResolvedURL.IsEmpty())
		{
			LogUnavailable(Diagnostic, TEXT("Matchmaking URL is empty"));
			return;
		}

		TSharedPtr<FCacheEntry>& Entry = SchemaCache.FindOrAdd(ResolvedURL);
		if (!Entry.IsValid())
		{
			Entry = MakeShared<FCacheEntry>();
		}

		const double NowSeconds = FPlatformTime::Seconds();
		if (Entry->bHasSnapshot && NowSeconds - Entry->CachedAtSeconds < SchemaCacheLifetimeSeconds)
		{
			LogReport(Entry->Snapshot, Diagnostic);
			return;
		}

		if (!Entry->bRequestInFlight
			&& !Entry->LastFailure.IsEmpty()
			&& NowSeconds - Entry->LastFailureAtSeconds < FailureCooldownSeconds)
		{
			LogUnavailable(Diagnostic, Entry->LastFailure);
			return;
		}

		Entry->PendingDiagnostics.Add(MoveTemp(Diagnostic));
		if (Entry->bRequestInFlight)
		{
			return;
		}

		Entry->bRequestInFlight = true;
		Entry->ActiveRequest = FHttpModule::Get().CreateRequest();
		Entry->ActiveRequest->SetVerb(TEXT("GET"));
		Entry->ActiveRequest->SetURL(ResolvedURL + TEXT("/swagger/v1/swagger.json"));
		Entry->ActiveRequest->SetHeader(TEXT("Accept"), TEXT("application/json"));
		Entry->ActiveRequest->SetTimeout(10.0f);

		const FString ResolvedToken = EGIK_MatchmakingRequest::ResolveAuthToken(AuthToken);
		if (!ResolvedToken.IsEmpty())
		{
			Entry->ActiveRequest->SetHeader(TEXT("Authorization"), ResolvedToken);
		}

		Entry->ActiveRequest->OnProcessRequestComplete().BindLambda(
			[CacheKey = ResolvedURL](TSharedPtr<IHttpRequest>, TSharedPtr<IHttpResponse> Response, bool bConnectedSuccessfully)
			{
				CompleteSchemaRequest(CacheKey, Response, bConnectedSuccessfully);
			});

		if (!Entry->ActiveRequest->ProcessRequest())
		{
			CompleteSchemaRequest(ResolvedURL, nullptr, false);
		}
	}

	void Shutdown()
	{
		for (TPair<FString, TSharedPtr<FCacheEntry>>& Pair : SchemaCache)
		{
			if (Pair.Value.IsValid() && Pair.Value->ActiveRequest.IsValid())
			{
				Pair.Value->ActiveRequest->OnProcessRequestComplete().Unbind();
				Pair.Value->ActiveRequest->CancelRequest();
				Pair.Value->ActiveRequest.Reset();
			}
		}
		SchemaCache.Reset();
	}
}

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEGIKMatchmakingProfileDebugTest,
	"Edgegap.Matchmaking.ProfileDebug.SchemaAndAlignment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEGIKMatchmakingProfileDebugTest::RunTest(const FString& Parameters)
{
	using namespace EGIK_MatchmakingProfileDebug;

	const FString SchemaJson = TEXT(R"JSON(
	{
		"openapi":"3.0.1",
		"paths":{
			"/tickets":{
				"post":{
					"requestBody":{
						"content":{
							"application/json":{
								"schema":{
									"oneOf":[{
										"type":"object",
										"properties":{
											"profile":{"type":"string","enum":["advanced-example"]},
											"attributes":{
												"type":"object",
												"required":["beacons","elo_rating","selected_map"],
												"additionalProperties":false,
												"properties":{
													"beacons":{"type":"object","description":"Latencies object for the beacons rule.","additionalProperties":{"type":"string"}},
													"elo_rating":{"type":"number"},
													"selected_map":{"type":"array","items":{"type":"string"}}
												}
											}
										}
									}]
								}
							}
						}
					}
				}
			}
		}
	}
	)JSON");

	FSchemaSnapshot Snapshot;
	FString ParseError;
	TestTrue(TEXT("Parses profile-specific ticket requirements"), ParseOpenApiSchema(SchemaJson, Snapshot, ParseError));
	TestEqual(TEXT("Finds one profile"), Snapshot.Profiles.Num(), 1);

	FPendingDiagnostic ValidDiagnostic;
	ValidDiagnostic.RequestContext = TEXT("TestValid");
	ValidDiagnostic.Profile = TEXT("advanced-example");
	ValidDiagnostic.StructuredAttributes.Values.Add(
		UEGIK_MatchmakingAttributeLibrary::MakeBeaconLatenciesAttribute({{TEXT("Montreal"), 12.3}}));
	ValidDiagnostic.StructuredAttributes.Values.Add(
		UEGIK_MatchmakingAttributeLibrary::MakeNumberAttribute(TEXT("elo_rating"), 1337.0));
	ValidDiagnostic.StructuredAttributes.Values.Add(
		UEGIK_MatchmakingAttributeLibrary::MakeStringArrayAttribute(TEXT("selected_map"), {TEXT("Airport")}));

	bool bHasMisalignment = true;
	const FString ValidReport = BuildReport(Snapshot, ValidDiagnostic, bHasMisalignment);
	TestFalse(TEXT("Matching structured attributes have no misalignment"), bHasMisalignment);
	TestTrue(TEXT("Normalizes Edgegap beacon schema to a number map"), ValidReport.Contains(TEXT("[OK] beacons: Number Map")));

	FPendingDiagnostic InvalidDiagnostic;
	InvalidDiagnostic.RequestContext = TEXT("TestInvalid");
	InvalidDiagnostic.Profile = TEXT("advanced-example");
	InvalidDiagnostic.StructuredAttributes.Values.Add(
		UEGIK_MatchmakingAttributeLibrary::MakeStringAttribute(TEXT("elo_rating"), TEXT("1337")));
	InvalidDiagnostic.StructuredAttributes.Values.Add(
		UEGIK_MatchmakingAttributeLibrary::MakeStringAttribute(TEXT("difficulty"), TEXT("hard")));

	const FString InvalidReport = BuildReport(Snapshot, InvalidDiagnostic, bHasMisalignment);
	TestTrue(TEXT("Missing, mistyped, and unknown attributes are misaligned"), bHasMisalignment);
	TestTrue(TEXT("Reports missing required attributes"), InvalidReport.Contains(TEXT("[MISSING] selected_map")));
	TestTrue(TEXT("Reports incorrect types"), InvalidReport.Contains(TEXT("[TYPE] elo_rating")));
	TestTrue(TEXT("Reports unknown attributes"), InvalidReport.Contains(TEXT("[UNKNOWN] difficulty")));

	return true;
}

#endif
