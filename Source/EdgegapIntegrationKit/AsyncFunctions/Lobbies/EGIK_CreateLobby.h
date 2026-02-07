// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_CreateLobby.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateLobbyResponse, FEGIK_LobbyInfo, Result, FEGIK_ErrorStruct, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateLobby : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:

	//Create a named lobby.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Lobbies")
	static UEGIK_CreateLobby* CreateLobby(FString LobbyName);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Lobbies")
	FCreateLobbyResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Lobbies")
	FCreateLobbyResponse OnFailure;

protected:
	//~ Begin UEGIK_AsyncRequestBase Interface
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("Lobbies"); }
	//~ End UEGIK_AsyncRequestBase Interface

private:
	FString Var_LobbyName;
};
