// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_ServerBrowserTypes.h"
#include "EGIK_SB_ConfirmReservations.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSB_ConfirmReservationsResponse, const FEGIK_SB_SeatConfirmationResponse&, Response, const FEGIK_ErrorStruct&, Error);

/**
 * Confirms seat reservations for players who have connected.
 * Called by the dedicated server when players connect.
 * Returns information about accepted, expired, and unknown players.
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_SB_ConfirmReservations : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	/**
	 * Confirm reservations for connected players
	 * @param RequestId Deployment request ID
	 * @param UserIds Array of player IDs that have connected
	 * @param Overrides Optional overrides for URL and auth token
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Server Browser")
	static UEGIK_SB_ConfirmReservations* ConfirmReservations(
		const FString& RequestId,
		const TArray<FString>& UserIds,
		const FEGIK_SB_Overrides& Overrides = FEGIK_SB_Overrides());

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_ConfirmReservationsResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server Browser")
	FSB_ConfirmReservationsResponse OnFailure;

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
	TArray<FString> Var_UserIds;
};
