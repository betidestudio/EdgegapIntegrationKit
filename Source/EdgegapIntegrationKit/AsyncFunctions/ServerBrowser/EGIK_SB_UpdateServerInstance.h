// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_UpdateServerInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSB_UpdateServerInstanceResponse, const FEGIK_SB_ServerInstance&, ServerInstance, const FEGIK_ErrorStruct&, Error);

/**
 * Updates a server instance's metadata in the Server Browser.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_UpdateServerInstance : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Update a server instance's metadata
	 * @param RequestId Deployment request ID
	 * @param MetadataJson New metadata as JSON string
	 * @param Overrides Optional overrides for URL and auth token
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_UpdateServerInstance* UpdateServerInstance(
		const FString& RequestId,
		const FString& MetadataJson,
		const FEGIK_SB_Overrides& Overrides = FEGIK_SB_Overrides());

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_UpdateServerInstanceResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_UpdateServerInstanceResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("ServerBrowser"); }

private:
	FEGIK_SB_Overrides Var_Overrides;
	FString Var_RequestId;
	FString Var_MetadataJson;
};
