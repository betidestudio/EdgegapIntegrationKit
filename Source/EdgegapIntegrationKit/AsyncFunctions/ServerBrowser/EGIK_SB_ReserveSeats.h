// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_ReserveSeats.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSB_ReserveSeatsResponse, const FEGIK_ErrorStruct&, Error);

/**
 * Reserves seats for players in a server slot.
 * Called by game clients before connecting to a server.
 * Reservations expire after 30 seconds if not confirmed.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_ReserveSeats : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Reserve seats for players
	 * @param RequestId Deployment request ID
	 * @param SlotName Name of the slot to reserve seats in
	 * @param UserIds Array of unique player IDs to reserve seats for
	 * @param Overrides Optional overrides for URL and auth token
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_ReserveSeats* ReserveSeats(
		const FString& RequestId,
		const FString& SlotName,
		const TArray<FString>& UserIds,
		const FEGIK_SB_Overrides& Overrides = FEGIK_SB_Overrides());

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_ReserveSeatsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_ReserveSeatsResponse OnFailure;

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
	TArray<FString> Var_UserIds;
};
