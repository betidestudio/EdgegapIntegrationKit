// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_ConfirmReservations.h"
#include "EGIKBlueprintFunctionLibrary.h"

UEGIK_SB_ConfirmReservations* UEGIK_SB_ConfirmReservations::ConfirmReservations(
	const FString& RequestId,
	const TArray<FString>& UserIds,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_ConfirmReservations* Node = NewObject<UEGIK_SB_ConfirmReservations>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	Node->Var_UserIds = UserIds;
	return Node;
}

FString UEGIK_SB_ConfirmReservations::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s/reservations:confirm"), *BaseURL, *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_SB_ConfirmReservations::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_SB_ConfirmReservations::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserServerToken() : Var_Overrides.AuthToken;
}

TSharedPtr<FJsonObject> UEGIK_SB_ConfirmReservations::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// Build user_ids array
	TArray<TSharedPtr<FJsonValue>> UserIdsArray;
	for (const FString& UserId : Var_UserIds)
	{
		UserIdsArray.Add(MakeShareable(new FJsonValueString(UserId)));
	}
	JsonObject->SetArrayField(TEXT("user_ids"), UserIdsArray);

	return JsonObject;
}

void UEGIK_SB_ConfirmReservations::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	if (!JsonObject.IsValid())
	{
		HandleError(0, TEXT("Failed to parse JSON response"));
		return;
	}

	FEGIK_SB_SeatConfirmationResponse Response(JsonObject);
	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_SB_ConfirmReservations::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_SB_SeatConfirmationResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
