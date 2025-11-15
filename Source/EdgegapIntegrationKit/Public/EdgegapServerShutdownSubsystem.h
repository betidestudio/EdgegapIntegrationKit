// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "EdgegapServerShutdownSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnServerShutdown, bool, bSuccess, FString, Message);

/**
 * Subsystem to handle server shutdown and self-stop API calls
 * Automatically calls Edgegap's self-stop API when the server exits
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEdgegapServerShutdownSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Call the Edgegap self-stop API
	 * @param bWaitForResponse - If true, will wait up to 5 seconds for the API response
	 */
	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Server")
	void CallSelfStopAPI(bool bWaitForResponse = false);

	/**
	 * Check if the server is running in Edgegap environment
	 * @return True if EDGEGAP_REQUEST_ID environment variable is set
	 */
	UFUNCTION(BlueprintPure, Category = "Edgegap Integration Kit | Server")
	bool IsRunningOnEdgegap() const;

	/**
	 * Get the current deployment request ID from environment
	 * @param RequestID - Output parameter for the request ID
	 * @return True if request ID was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Edgegap Integration Kit | Server")
	bool GetDeploymentRequestID(FString& RequestID) const;

	/** Event fired when server shutdown API call completes */
	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Server")
	FOnServerShutdown OnServerShutdown;

private:
	/** The deployment request ID from environment */
	FString CachedRequestID;

	/** Whether we're running on Edgegap (have request ID) */
	bool bIsEdgegapEnvironment;

	/** Flag to prevent multiple shutdown calls */
	bool bShutdownInitiated;

	/** Handle for shutdown request */
	TSharedPtr<class IHttpRequest> ShutdownRequest;

	/** Callback for shutdown API response */
	void OnShutdownResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);

	/** Internal method to stop deployment */
	void StopDeploymentInternal(const FString& RequestID);

	/** Setup crash handlers */
	void SetupCrashHandlers();

	/** Setup shutdown hooks */
	void SetupShutdownHooks();

	/** Static crash handler callback */
	static void StaticCrashHandler(const TCHAR* ErrorMessage, bool bIsAssert);

	/** Console command handler for RequestExit */
	void HandleRequestExitCommand();
};

