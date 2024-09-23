// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_DeployLobby.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDeployLobbyResponse, FEGIK_LobbyInfo, Result, FEGIK_ErrorStruct, Error);
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_DeployLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Lobbies")
	static UEGIK_DeployLobby* DeployLobby(FString LobbyName);

	virtual void Activate() override;
	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Lobbies")
	FDeployLobbyResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Lobbies")
	FDeployLobbyResponse OnFailure;

private:
	FString Var_LobbyName;
};
