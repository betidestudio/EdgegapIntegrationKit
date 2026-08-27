// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_ListSlots.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSB_ListSlotsResponse, const FEGIK_SB_ListSlotsResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Lists all slots for a server instance with cursor pagination.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_ListSlots : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * List slots for a server instance
	 * @param RequestId Deployment request ID
	 * @param Cursor Optional cursor for pagination
	 * @param Limit Number of results per page (1-100, default 10)
	 * @param Overrides Optional overrides for URL and auth token
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_ListSlots* ListSlots(
		const FString& RequestId,
		const FString& Cursor = TEXT(""),
		int32 Limit = 10,
		const FEGIK_SB_Overrides& Overrides = FEGIK_SB_Overrides());

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_ListSlotsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_ListSlotsResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("ServerBrowser"); }

private:
	FEGIK_SB_Overrides Var_Overrides;
	FString Var_RequestId;
	FString Var_Cursor;
	int32 Var_Limit;
};
