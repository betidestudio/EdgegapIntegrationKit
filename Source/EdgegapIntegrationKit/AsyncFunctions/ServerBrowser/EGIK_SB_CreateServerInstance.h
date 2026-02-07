// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_CreateServerInstance.generated.h"

/**
 * Request structure for creating a server instance
 */
USTRUCT(BlueprintType)
struct FEGIK_SB_CreateServerInstanceRequest
{
	GENERATED_BODY()

	/** Optional overrides for URL and auth token (leave empty to auto-read from settings) */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FEGIK_SB_Overrides Overrides;

	/** Deployment request ID (from Edgegap deployment) */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString RequestId;

	/** Custom metadata as JSON string (can contain game_mode, map, max_players, etc.) */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FString MetadataJson;

	/** Initial slots for this server instance */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	TArray<FEGIK_SB_ServerInstanceSlot> Slots;

	/** Server connection details */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Server Browser")
	FEGIK_SB_Server Server;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSB_CreateServerInstanceResponse, const FEGIK_SB_ServerInstance&, ServerInstance, const FEGIK_ErrorStruct&, Error);

/**
 * Creates a new server instance in the Server Browser.
 * Called by dedicated servers to register themselves as available for players.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_CreateServerInstance : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Create a new server instance in the Server Browser
	 * @param Request The request containing server details and metadata
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_CreateServerInstance* CreateServerInstance(FEGIK_SB_CreateServerInstanceRequest Request);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_CreateServerInstanceResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_CreateServerInstanceResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("ServerBrowser"); }

private:
	FEGIK_SB_CreateServerInstanceRequest Var_Request;
};
