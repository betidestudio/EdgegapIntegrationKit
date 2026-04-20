// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_SB_ReserveSeats.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "GenericPlatform/GenericPlatformHttp.h"

UEGIK_SB_ReserveSeats* UEGIK_SB_ReserveSeats::ReserveSeats(
	const FString& RequestId,
	const FString& SlotName,
	const TArray<FString>& UserIds,
	const FEGIK_SB_Overrides& Overrides)
{
	UEGIK_SB_ReserveSeats* Node = NewObject<UEGIK_SB_ReserveSeats>();
	Node->Var_Overrides = Overrides;
	Node->Var_RequestId = RequestId;
	Node->Var_SlotName = SlotName;
	Node->Var_UserIds = UserIds;
	return Node;
}

FString UEGIK_SB_ReserveSeats::GetEndpointURL() const
{
	FString BaseURL = Var_Overrides.ServerBrowserURL.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserURL() : Var_Overrides.ServerBrowserURL;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return FString::Printf(TEXT("%s/server-instances/%s/slots/%s/reservations"),
		*BaseURL, *Var_RequestId, *FGenericPlatformHttp::UrlEncode(Var_SlotName));
}

EEGIK_HttpVerb UEGIK_SB_ReserveSeats::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_SB_ReserveSeats::GetAuthorizationHeader() const
{
	return Var_Overrides.AuthToken.IsEmpty() ? UEGIKBlueprintFunctionLibrary::GetServerBrowserClientToken() : Var_Overrides.AuthToken;
}

TSharedPtr<FJsonObject> UEGIK_SB_ReserveSeats::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// Build users array
	TArray<TSharedPtr<FJsonValue>> UsersArray;
	for (const FString& UserId : Var_UserIds)
	{
		TSharedPtr<FJsonObject> UserObj = MakeShareable(new FJsonObject);
		UserObj->SetStringField(TEXT("user_id"), UserId);
		UsersArray.Add(MakeShareable(new FJsonValueObject(UserObj)));
	}
	JsonObject->SetArrayField(TEXT("users"), UsersArray);

	return JsonObject;
}

void UEGIK_SB_ReserveSeats::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	// 200/201 indicates successful reservation
	OnSuccess.Broadcast(FEGIK_ErrorStruct());
}

void UEGIK_SB_ReserveSeats::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
