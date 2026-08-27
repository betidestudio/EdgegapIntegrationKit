// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_Monitor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSB_MonitorResponse, const FEGIK_SB_MonitorResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Checks the health status of the Server Browser service.
 * Returns "Healthy" when the service is operational.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_Monitor : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Check the Server Browser service health
	 * @param Overrides Optional overrides for URL (auth not needed for monitor)
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_Monitor* Monitor(const FEGIK_SB_Overrides& Overrides = FEGIK_SB_Overrides());

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_MonitorResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_MonitorResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("ServerBrowser"); }

private:
	FEGIK_SB_Overrides Var_Overrides;
};
