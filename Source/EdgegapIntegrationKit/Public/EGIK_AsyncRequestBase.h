// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_AsyncRequestBase.generated.h"

EDGEGAPINTEGRATIONKIT_API DECLARE_LOG_CATEGORY_EXTERN(LogEdgegap, Log, All);

UENUM(BlueprintType)
enum class EEGIK_HttpVerb : uint8
{
	GET,
	POST,
	PUT,
	PATCH,
	DELETE
};

/**
 * Base class for all Edgegap async HTTP requests.
 * Provides common functionality for HTTP handling, error processing, and cleanup.
 *
 * Subclasses must implement:
 * - GetEndpointURL(): Return the full URL for the request
 * - GetHTTPVerb(): Return the HTTP method to use
 * - ProcessResponse(): Handle successful JSON responses
 * - HandleError(): Handle error cases
 *
 * Subclasses may override:
 * - GetAuthorizationHeader(): Custom auth (defaults to main API key)
 * - BuildRequestBody(): For POST/PUT/PATCH requests
 * - HandleRateLimited(): For endpoints that may return 429
 * - Retry policy getters: To customize timeout/backoff for a specific endpoint
 */
UCLASS(Abstract)
class EDGEGAPINTEGRATIONKIT_API UEGIK_AsyncRequestBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void Activate() override;

protected:
	//~ Begin Required Overrides

	/** Return the full endpoint URL for this request */
	virtual FString GetEndpointURL() const PURE_VIRTUAL(UEGIK_AsyncRequestBase::GetEndpointURL, return TEXT(""););

	/** Return the HTTP verb for this request */
	virtual EEGIK_HttpVerb GetHTTPVerb() const PURE_VIRTUAL(UEGIK_AsyncRequestBase::GetHTTPVerb, return EEGIK_HttpVerb::GET;);

	/** Process a successful HTTP response. JsonObject may be null for empty responses. */
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) PURE_VIRTUAL(UEGIK_AsyncRequestBase::ProcessResponse, );

	/** Handle an error response or connection failure */
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) PURE_VIRTUAL(UEGIK_AsyncRequestBase::HandleError, );

	//~ End Required Overrides

	//~ Begin Optional Overrides

	/** Return the authorization header value. Override for custom auth (e.g., matchmaking tokens) */
	virtual FString GetAuthorizationHeader() const;

	/** Build the request body JSON. Return nullptr for requests without a body */
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const { return nullptr; }

	/**
	 * Validate request inputs before creating or sending an HTTP request.
	 * Return false and populate OutErrorMessage to fail locally through HandleError.
	 */
	virtual bool ValidateRequest(FString& OutErrorMessage) const { return true; }

	/** Handle rate limited (429) responses. Override if your endpoint may return 429 */
	virtual void HandleRateLimited(const FString& ResponseContent) { HandleError(429, ResponseContent); }

	/** Return additional headers to include in the request */
	virtual TMap<FString, FString> GetAdditionalHeaders() const { return TMap<FString, FString>(); }

	/** Maximum number of retries after the initial request. */
	virtual int32 GetMaxRetryAttempts() const { return 3; }

	/** Timeout for each individual HTTP attempt. */
	virtual float GetRequestTimeoutSeconds() const { return 10.0f; }

	/** Initial delay used for exponential retry backoff. */
	virtual float GetInitialRetryDelaySeconds() const { return 1.0f; }

	/** Upper bound for the exponential portion of retry backoff. */
	virtual float GetMaxRetryDelaySeconds() const { return 10.0f; }

	/** Maximum random jitter added to a retry delay. */
	virtual float GetRetryJitterSeconds() const { return 0.25f; }

	/** Network failures, 429 responses, and transient 5xx responses are retried by default. */
	virtual bool IsRetryableResponse(int32 ResponseCode, bool bConnectedSuccessfully) const;

	//~ End Optional Overrides

	//~ Begin Helper Methods

	/** Execute the HTTP request with current configuration */
	void ExecuteRequest();

	/** Called when HTTP response is received */
	void OnHttpResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bConnectedSuccessfully);

	/** Mark this async action as complete and ready for garbage collection */
	void FinishRequest();

	/** Schedule another attempt when the retry policy allows it. */
	bool TryScheduleRetry(int32 ResponseCode, const FString& ErrorMessage, const TSharedPtr<IHttpResponse>& HttpResponse, bool bConnectedSuccessfully);

	/** Core ticker callback used to execute a delayed retry without requiring a World. */
	bool HandleRetryTicker(float DeltaTime);

	/** Calculate exponential jittered delay, respecting numeric Retry-After when present. */
	float CalculateRetryDelaySeconds(const TSharedPtr<IHttpResponse>& HttpResponse) const;

	/** Convert HTTP verb enum to string */
	static FString HttpVerbToString(EEGIK_HttpVerb Verb);

	/** Serialize a JSON object to string for request body */
	static FString JsonObjectToString(TSharedPtr<FJsonObject> JsonObject);

	/** Parse JSON string to object, returns nullptr on failure */
	static TSharedPtr<FJsonObject> ParseJsonResponse(const FString& JsonString);

	/** Convert a JSON API error response into a concise, user-facing message. */
	static FString FormatErrorResponse(int32 HttpStatusCode, const FString& ResponseContent);

	/** Create a JSON array from a string array */
	static TArray<TSharedPtr<FJsonValue>> StringArrayToJsonArray(const TArray<FString>& StringArray);

	//~ End Helper Methods

	//~ Begin Logging

	/** Enable/disable verbose logging for this request */
	bool bEnableVerboseLogging = false;

	/** Log category name for this request type (override in subclass for better filtering) */
	virtual FString GetLogCategory() const { return TEXT("EGIK"); }

	/** Log a message with the request's category */
	void LogMessage(const FString& Message) const;
	void LogWarning(const FString& Message) const;
	void LogError(const FString& Message) const;

	//~ End Logging

private:
	/** The active HTTP request */
	TSharedPtr<IHttpRequest> ActiveRequest;

	FTSTicker::FDelegateHandle RetryTickerHandle;
	int32 RetryAttemptCount = 0;
	bool bAddedToRootForRequest = false;
	bool bRequestFinished = false;
};
