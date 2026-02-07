// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_AsyncRequestBase.h"

DEFINE_LOG_CATEGORY(LogEdgegap);

void UEGIK_AsyncRequestBase::Activate()
{
	Super::Activate();
	ExecuteRequest();
}

FString UEGIK_AsyncRequestBase::GetAuthorizationHeader() const
{
	return UEGIKBlueprintFunctionLibrary::GetAuthorizationKey();
}

void UEGIK_AsyncRequestBase::ExecuteRequest()
{
	FHttpModule* Http = &FHttpModule::Get();
	ActiveRequest = Http->CreateRequest();

	const FString Url = GetEndpointURL();
	const FString Verb = HttpVerbToString(GetHTTPVerb());

	ActiveRequest->SetVerb(Verb);
	ActiveRequest->SetURL(Url);
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
		LogError(TEXT("Failed to process HTTP request"));
		HandleError(0, TEXT("Failed to process HTTP request"));
		FinishRequest();
	}
}

void UEGIK_AsyncRequestBase::OnHttpResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bConnectedSuccessfully)
{
	if (!HttpResponse.IsValid())
	{
		LogError(TEXT("Invalid HTTP response - connection may have failed"));
		HandleError(0, TEXT("Failed to connect to server. The service may be down or the URL may be incorrect."));
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

	// Handle rate limiting
	if (ResponseCode == 429)
	{
		LogWarning(TEXT("Rate limited (429 Too Many Requests)"));
		HandleRateLimited(ResponseContent);
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
	LogError(FString::Printf(TEXT("HTTP Error %d: %s"), ResponseCode, *ResponseContent));
	HandleError(ResponseCode, ResponseContent);
	FinishRequest();
}

void UEGIK_AsyncRequestBase::FinishRequest()
{
	ActiveRequest.Reset();
	SetReadyToDestroy();
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
