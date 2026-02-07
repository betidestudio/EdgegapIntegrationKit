// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_UpdateSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSB_UpdateSlotResponse, const FEGIK_SB_ServerInstanceSlot&, Slot, const FEGIK_ErrorStruct&, Error);

/**
 * Updates a slot's available seats and/or metadata.
 * Called when players join/leave to update capacity.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_UpdateSlot : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Update a slot's properties
	 * @param RequestId Deployment request ID
	 * @param SlotName Name of the slot to update
	 * @param AvailableSeats New available seats count (-1 to not update)
	 * @param MetadataJson New metadata as JSON string (empty to not update)
	 * @param Overrides Optional overrides for URL and auth token
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_UpdateSlot* UpdateSlot(
		const FString& RequestId,
		const FString& SlotName,
		int32 AvailableSeats = -1,
		const FString& MetadataJson = TEXT(""),
		const FEGIK_SB_Overrides& Overrides = FEGIK_SB_Overrides());

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_UpdateSlotResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_UpdateSlotResponse OnFailure;

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
	FString Var_SlotName;
	int32 Var_AvailableSeats;
	FString Var_MetadataJson;
};
