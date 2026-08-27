// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_AsyncRequestBase.h"

DEFINE_LOG_CATEGORY(LogEdgegap);

namespace EGIK_AsyncRequest
{
	static bool IsUsableHttpUrl(const FString& Url)
	{
		const bool bHttp = Url.StartsWith(TEXT("http://"), ESearchCase::IgnoreCase);
		const bool bHttps = Url.StartsWith(TEXT("https://"), ESearchCase::IgnoreCase);
		if (!bHttp && !bHttps)
		{
			return false;
		}

		const int32 HostStart = bHttps ? 8 : 7;
		const int32 PathStart = Url.Find(TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromStart, HostStart);
		const FString Host = PathStart == INDEX_NONE ? Url.Mid(HostStart) : Url.Mid(HostStart, PathStart - HostStart);
		return !Host.TrimStartAndEnd().IsEmpty();
	}

	static FString JsonScalarToString(const TSharedPtr<FJsonValue>& Value)
	{
		if (!Value.IsValid())
		{
			return TEXT("");
		}

		switch (Value->Type)
		{
		case EJson::String:
			return Value->AsString();
		case EJson::Number:
			return FString::SanitizeFloat(Value->AsNumber());
		case EJson::Boolean:
			return Value->AsBool() ? TEXT("true") : TEXT("false");
		case EJson::Null:
			return TEXT("null");
		default:
			return TEXT("");
		}
	}

	static FString FormatLocation(const TArray<TSharedPtr<FJsonValue>>& Location)
	{
		TArray<FString> Segments;
		for (const TSharedPtr<FJsonValue>& Segment : Location)
		{
			const FString SegmentText = JsonScalarToString(Segment);
			if (!SegmentText.IsEmpty()
				&& !SegmentText.Equals(TEXT("body"), ESearchCase::IgnoreCase)
				&& !SegmentText.Equals(TEXT("query"), ESearchCase::IgnoreCase)
				&& !SegmentText.Equals(TEXT("path"), ESearchCase::IgnoreCase))
			{
				Segments.Add(SegmentText);
			}
		}
		return FString::Join(Segments, TEXT("."));
	}

	static void AddUniqueMessage(TArray<FString>& Messages, const FString& Message)
	{
		FString CleanMessage = Message;
		CleanMessage.TrimStartAndEndInline();
		if (!CleanMessage.IsEmpty())
		{
			Messages.AddUnique(CleanMessage);
		}
	}

	static void CollectErrorMessages(const TSharedPtr<FJsonValue>& Value, TArray<FString>& Messages, int32 Depth = 0)
	{
		if (!Value.IsValid() || Depth > 6)
		{
			return;
		}

		if (Value->Type == EJson::String)
		{
			AddUniqueMessage(Messages, Value->AsString());
			return;
		}

		if (Value->Type == EJson::Array)
		{
			for (const TSharedPtr<FJsonValue>& Item : Value->AsArray())
			{
				CollectErrorMessages(Item, Messages, Depth + 1);
			}
			return;
		}

		if (Value->Type != EJson::Object)
		{
			return;
		}

		const TSharedPtr<FJsonObject> Object = Value->AsObject();
		if (!Object.IsValid())
		{
			return;
		}

		FString Message;
		for (const TCHAR* MessageKey : { TEXT("message"), TEXT("msg"), TEXT("detail"), TEXT("error"), TEXT("title") })
		{
			if (Object->TryGetStringField(MessageKey, Message) && !Message.IsEmpty())
			{
				break;
			}
		}

		FString Location;
		const TArray<TSharedPtr<FJsonValue>>* LocationValues = nullptr;
		if (Object->TryGetArrayField(TEXT("loc"), LocationValues) && LocationValues)
		{
			Location = FormatLocation(*LocationValues);
		}
		else
		{
			Object->TryGetStringField(TEXT("field"), Location);
		}

		if (!Message.IsEmpty())
		{
			AddUniqueMessage(Messages, Location.IsEmpty() ? Message : FString::Printf(TEXT("%s: %s"), *Location, *Message));
		}

		for (const TCHAR* NestedKey : { TEXT("detail"), TEXT("details"), TEXT("error"), TEXT("errors") })
		{
			const TSharedPtr<FJsonValue> NestedValue = Object->TryGetField(NestedKey);
			if (NestedValue.IsValid() && NestedValue->Type != EJson::String)
			{
				CollectErrorMessages(NestedValue, Messages, Depth + 1);
			}
		}
	}

	static FString GetStatusDescription(int32 StatusCode)
	{
		switch (StatusCode)
		{
		case 400: return TEXT("Bad Request");
		case 401: return TEXT("Unauthorized - check the authorization credential");
		case 403: return TEXT("Forbidden - the credential does not permit this request");
		case 404: return TEXT("Not Found - check the URL and resource IDs");
		case 409: return TEXT("Conflict - the resource may have advanced to a state that no longer allows this operation");
		case 422: return TEXT("Validation Failed");
		case 429: return TEXT("Too Many Requests - retry with backoff");
		case 500: return TEXT("Edgegap Internal Server Error");
		case 502: return TEXT("Bad Gateway");
		case 503: return TEXT("Service Unavailable");
		case 504: return TEXT("Gateway Timeout");
		default: return TEXT("HTTP request failed");
		}
	}
}

void UEGIK_AsyncRequestBase::Activate()
{
	Super::Activate();
	if (!IsRooted())
	{
		AddToRoot();
		bAddedToRootForRequest = true;
	}
	ExecuteRequest();
}

FString UEGIK_AsyncRequestBase::GetAuthorizationHeader() const
{
	return UEGIKBlueprintFunctionLibrary::GetAuthorizationKey();
}

void UEGIK_AsyncRequestBase::ExecuteRequest()
{
	FString ValidationError;
	if (!ValidateRequest(ValidationError))
	{
		const FString ErrorMessage = FString::Printf(TEXT("Request was not sent: %s"), *ValidationError);
		LogError(ErrorMessage);
		HandleError(0, ErrorMessage);
		FinishRequest();
		return;
	}

	FString Url = GetEndpointURL();
	Url.TrimStartAndEndInline();
	if (!EGIK_AsyncRequest::IsUsableHttpUrl(Url))
	{
		const FString ErrorMessage = FString::Printf(
			TEXT("Request was not sent: endpoint URL '%s' is invalid or has no host."),
			*Url);
		LogError(ErrorMessage);
		HandleError(0, ErrorMessage);
		FinishRequest();
		return;
	}

	FHttpModule* Http = &FHttpModule::Get();
	ActiveRequest = Http->CreateRequest();

	const FString Verb = HttpVerbToString(GetHTTPVerb());

	ActiveRequest->SetVerb(Verb);
	ActiveRequest->SetURL(Url);
	ActiveRequest->SetTimeout(GetRequestTimeoutSeconds());
	ActiveRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	ActiveRequest->SetHeader(TEXT("Authorization"), GetAuthorizationHeader());

	// Add any additional headers
	TMap<FString, FString> AdditionalHeaders = GetAdditionalHeaders();
	for (const auto& Header : AdditionalHeaders)
	{
		ActiveRequest->SetHeader(Header.Key, Header.Value);
	}

	// Build request body if applicable
	TSharedPtr<FJsonObject> RequestBody = BuildRequestBody();
	if (RequestBody.IsValid())
	{
		FString BodyString = JsonObjectToString(RequestBody);
		ActiveRequest->SetContentAsString(BodyString);

		if (bEnableVerboseLogging)
		{
			LogMessage(FString::Printf(TEXT("Request Body: %s"), *BodyString));
		}
	}

	if (bEnableVerboseLogging)
	{
		LogMessage(FString::Printf(TEXT("Executing %s request to: %s"), *Verb, *Url));
	}

	ActiveRequest->OnProcessRequestComplete().BindUObject(this, &UEGIK_AsyncRequestBase::OnHttpResponseReceived);

	if (!ActiveRequest->ProcessRequest())
	{
		const FString ErrorMessage = TEXT("Failed to start HTTP request");
		if (TryScheduleRetry(0, ErrorMessage, nullptr, false))
		{
			return;
		}
		LogError(ErrorMessage);
		HandleError(0, ErrorMessage);
		FinishRequest();
	}
}

void UEGIK_AsyncRequestBase::OnHttpResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bConnectedSuccessfully)
{
	if (!bConnectedSuccessfully || !HttpResponse.IsValid())
	{
		const FString ErrorMessage = TEXT("Failed to connect to server. The service may be down, the request may have timed out, or the URL may be incorrect.");
		if (TryScheduleRetry(0, ErrorMessage, HttpResponse, false))
		{
			return;
		}
		LogError(ErrorMessage);
		HandleError(0, ErrorMessage);
		FinishRequest();
		return;
	}

	const int32 ResponseCode = HttpResponse->GetResponseCode();
	const FString ResponseContent = HttpResponse->GetContentAsString();

	if (bEnableVerboseLogging)
	{
		LogMessage(FString::Printf(TEXT("Response Code: %d"), ResponseCode));
		LogMessage(FString::Printf(TEXT("Response Content: %s"), *ResponseContent));
	}

	if (IsRetryableResponse(ResponseCode, true))
	{
		const FString ErrorMessage = FormatErrorResponse(ResponseCode, ResponseContent);
		if (TryScheduleRetry(ResponseCode, ErrorMessage, HttpResponse, true))
		{
			return;
		}

		if (ResponseCode == 429)
		{
			LogWarning(FString::Printf(TEXT("HTTP 429 after retries: %s"), *ErrorMessage));
			HandleRateLimited(ErrorMessage);
		}
		else
		{
			LogError(FString::Printf(TEXT("HTTP %d after retries: %s"), ResponseCode, *ErrorMessage));
			HandleError(ResponseCode, ErrorMessage);
		}
		FinishRequest();
		return;
	}

	// Handle success responses (2xx)
	if (EHttpResponseCodes::IsOk(ResponseCode))
	{
		TSharedPtr<FJsonObject> JsonObject = nullptr;

		// Only parse JSON if there's content
		if (!ResponseContent.IsEmpty())
		{
			JsonObject = ParseJsonResponse(ResponseContent);
			if (!JsonObject.IsValid() && !ResponseContent.IsEmpty())
			{
				LogWarning(TEXT("Failed to parse JSON response, passing raw content"));
			}
		}

		ProcessResponse(ResponseCode, JsonObject);
		FinishRequest();
		return;
	}

	// Handle error responses
	const FString ErrorMessage = FormatErrorResponse(ResponseCode, ResponseContent);
	LogError(FString::Printf(TEXT("HTTP %d: %s"), ResponseCode, *ErrorMessage));
	HandleError(ResponseCode, ErrorMessage);
	FinishRequest();
}

bool UEGIK_AsyncRequestBase::IsRetryableResponse(int32 ResponseCode, bool bConnectedSuccessfully) const
{
	return !bConnectedSuccessfully || ResponseCode == 429 || (ResponseCode >= 500 && ResponseCode <= 599);
}

bool UEGIK_AsyncRequestBase::TryScheduleRetry(
	int32 ResponseCode,
	const FString& ErrorMessage,
	const TSharedPtr<IHttpResponse>& HttpResponse,
	bool bConnectedSuccessfully)
{
	if (bRequestFinished
		|| !IsRetryableResponse(ResponseCode, bConnectedSuccessfully)
		|| RetryAttemptCount >= GetMaxRetryAttempts())
	{
		return false;
	}

	const float DelaySeconds = CalculateRetryDelaySeconds(HttpResponse);
	++RetryAttemptCount;
	ActiveRequest.Reset();

	LogWarning(FString::Printf(
		TEXT("Retrying request %d/%d in %.2f seconds after: %s"),
		RetryAttemptCount,
		GetMaxRetryAttempts(),
		DelaySeconds,
		*ErrorMessage));

	if (RetryTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RetryTickerHandle);
		RetryTickerHandle.Reset();
	}

	RetryTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UEGIK_AsyncRequestBase::HandleRetryTicker),
		DelaySeconds);
	return true;
}

bool UEGIK_AsyncRequestBase::HandleRetryTicker(float DeltaTime)
{
	RetryTickerHandle.Reset();
	if (!bRequestFinished)
	{
		ExecuteRequest();
	}
	return false;
}

float UEGIK_AsyncRequestBase::CalculateRetryDelaySeconds(const TSharedPtr<IHttpResponse>& HttpResponse) const
{
	if (HttpResponse.IsValid())
	{
		FString RetryAfter = HttpResponse->GetHeader(TEXT("Retry-After"));
		RetryAfter.TrimStartAndEndInline();
		if (RetryAfter.IsNumeric())
		{
			const float HeaderDelay = FCString::Atof(*RetryAfter);
			if (HeaderDelay >= 0.0f)
			{
				return HeaderDelay + FMath::FRandRange(0.0f, GetRetryJitterSeconds());
			}
		}
	}

	const float ExponentialDelay = GetInitialRetryDelaySeconds() * FMath::Pow(2.0f, static_cast<float>(RetryAttemptCount));
	return FMath::Min(ExponentialDelay, GetMaxRetryDelaySeconds())
		+ FMath::FRandRange(0.0f, GetRetryJitterSeconds());
}

void UEGIK_AsyncRequestBase::FinishRequest()
{
	if (bRequestFinished)
	{
		return;
	}
	bRequestFinished = true;

	if (RetryTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RetryTickerHandle);
		RetryTickerHandle.Reset();
	}
	ActiveRequest.Reset();
	SetReadyToDestroy();
	if (bAddedToRootForRequest && IsRooted())
	{
		RemoveFromRoot();
		bAddedToRootForRequest = false;
	}
	MarkAsGarbage();
}

FString UEGIK_AsyncRequestBase::HttpVerbToString(EEGIK_HttpVerb Verb)
{
	switch (Verb)
	{
	case EEGIK_HttpVerb::GET:
		return TEXT("GET");
	case EEGIK_HttpVerb::POST:
		return TEXT("POST");
	case EEGIK_HttpVerb::PUT:
		return TEXT("PUT");
	case EEGIK_HttpVerb::PATCH:
		return TEXT("PATCH");
	case EEGIK_HttpVerb::DELETE:
		return TEXT("DELETE");
	default:
		return TEXT("GET");
	}
}

FString UEGIK_AsyncRequestBase::JsonObjectToString(TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		return TEXT("");
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	return OutputString;
}

TSharedPtr<FJsonObject> UEGIK_AsyncRequestBase::ParseJsonResponse(const FString& JsonString)
{
	if (JsonString.IsEmpty())
	{
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		return JsonObject;
	}

	return nullptr;
}

FString UEGIK_AsyncRequestBase::FormatErrorResponse(int32 HttpStatusCode, const FString& ResponseContent)
{
	const FString StatusDescription = EGIK_AsyncRequest::GetStatusDescription(HttpStatusCode);
	FString TrimmedContent = ResponseContent;
	TrimmedContent.TrimStartAndEndInline();

	if (TrimmedContent.IsEmpty())
	{
		return StatusDescription;
	}

	TSharedPtr<FJsonValue> RootValue;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(TrimmedContent);
	if (!FJsonSerializer::Deserialize(Reader, RootValue) || !RootValue.IsValid())
	{
		return FString::Printf(TEXT("%s: %s"), *StatusDescription, *TrimmedContent);
	}

	TArray<FString> Messages;
	EGIK_AsyncRequest::CollectErrorMessages(RootValue, Messages);
	if (Messages.IsEmpty())
	{
		return FString::Printf(TEXT("%s: %s"), *StatusDescription, *TrimmedContent);
	}

	return FString::Printf(TEXT("%s: %s"), *StatusDescription, *FString::Join(Messages, TEXT("; ")));
}

TArray<TSharedPtr<FJsonValue>> UEGIK_AsyncRequestBase::StringArrayToJsonArray(const TArray<FString>& StringArray)
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	for (const FString& Value : StringArray)
	{
		JsonArray.Add(MakeShareable(new FJsonValueString(Value)));
	}
	return JsonArray;
}

void UEGIK_AsyncRequestBase::LogMessage(const FString& Message) const
{
	UE_LOG(LogEdgegap, Log, TEXT("[%s] %s"), *GetLogCategory(), *Message);
}

void UEGIK_AsyncRequestBase::LogWarning(const FString& Message) const
{
	UE_LOG(LogEdgegap, Warning, TEXT("[%s] %s"), *GetLogCategory(), *Message);
}

void UEGIK_AsyncRequestBase::LogError(const FString& Message) const
{
	UE_LOG(LogEdgegap, Error, TEXT("[%s] %s"), *GetLogCategory(), *Message);
}
