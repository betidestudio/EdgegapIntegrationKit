// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_GetSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSB_GetSlotResponse, const FEGIK_SB_ServerInstanceSlot&, Slot, const FEGIK_ErrorStruct&, Error);

/**
 * Gets a specific slot by name from a server instance.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_GetSlot : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Get a slot by name
	 * @param RequestId Deployment request ID
	 * @param SlotName Name of the slot
	 * @param Overrides Optional overrides for URL and auth token
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_GetSlot* GetSlot(
		const FString& RequestId,
		const FString& SlotName,
		const FEGIK_SB_Overrides& Overrides = FEGIK_SB_Overrides());

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_GetSlotResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_GetSlotResponse OnFailure;

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
	FString Var_SlotName;
};
