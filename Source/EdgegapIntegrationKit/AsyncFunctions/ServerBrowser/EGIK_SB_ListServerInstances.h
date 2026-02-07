// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_ListServerInstances.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSB_ListServerInstancesResponse, const FEGIK_SB_ListServerInstancesResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Lists all server instances from the Server Browser with optional cursor pagination.
 * Used by game clients to browse available servers.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_ListServerInstances : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * List server instances with optional pagination
	 * @param Cursor Optional cursor for pagination (from previous response)
	 * @param Limit Number of results per page (1-100, default 10)
	 * @param Filter Optional filter string (e.g., "joinable_seats gt 0")
	 * @param OrderBy Optional order by string (e.g., "joinable_seats desc")
	 * @param Overrides Optional overrides for URL and auth token
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_ListServerInstances* ListServerInstances(
		const FString& Cursor = TEXT(""),
		int32 Limit = 10,
		const FString& Filter = TEXT(""),
		const FString& OrderBy = TEXT(""),
		const FEGIK_SB_Overrides& Overrides = FEGIK_SB_Overrides());

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_ListServerInstancesResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_ListServerInstancesResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("ServerBrowser"); }

private:
	FEGIK_SB_Overrides Var_Overrides;
	FString Var_Cursor;
	int32 Var_Limit;
	FString Var_Filter;
	FString Var_OrderBy;
};
