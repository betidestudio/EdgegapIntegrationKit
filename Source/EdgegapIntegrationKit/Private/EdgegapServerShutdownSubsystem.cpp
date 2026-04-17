// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EdgegapServerShutdownSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"
#include "Misc/App.h"
#include "Engine/Engine.h"
#include "Misc/CoreDelegates.h"
#include "HAL/IConsoleManager.h"
#include "Engine/GameInstance.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UEdgegapServerShutdownSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bShutdownInitiated = false;

	// Read the official Edgegap-injected environment variables
	CachedRequestID = FPlatformMisc::GetEnvironmentVariable(TEXT("ARBITRIUM_REQUEST_ID"));
	CachedDeleteURL = FPlatformMisc::GetEnvironmentVariable(TEXT("ARBITRIUM_DELETE_URL"));
	CachedDeleteToken = FPlatformMisc::GetEnvironmentVariable(TEXT("ARBITRIUM_DELETE_TOKEN"));

	UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: ARBITRIUM_REQUEST_ID = '%s'"), *CachedRequestID);
	UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: ARBITRIUM_DELETE_URL = '%s'"), *CachedDeleteURL);
	UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: ARBITRIUM_DELETE_TOKEN = '%s'"), CachedDeleteToken.IsEmpty() ? TEXT("<not set>") : TEXT("<set>"));

	bIsEdgegapEnvironment = !CachedRequestID.IsEmpty() || !CachedDeleteURL.IsEmpty();

	if (bIsEdgegapEnvironment)
	{
		UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: Detected Edgegap environment"));

		// Setup shutdown hooks
		SetupShutdownHooks();

		// Setup crash handlers
		SetupCrashHandlers();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EdgegapServerShutdown: Edgegap runtime environment variables not found."));
	}
}

void UEdgegapServerShutdownSubsystem::Deinitialize()
{
	// Call self-stop API on shutdown if not already called
	if (bIsEdgegapEnvironment && !bShutdownInitiated)
	{
		UE_LOG(LogTemp, Warning, TEXT("EdgegapServerShutdown: Deinitialize called without explicit shutdown, calling self-stop API"));
		CallSelfStopAPI(false);
	}
	
	Super::Deinitialize();
}

bool UEdgegapServerShutdownSubsystem::IsRunningOnEdgegap() const
{
	return bIsEdgegapEnvironment;
}

bool UEdgegapServerShutdownSubsystem::GetDeploymentRequestID(FString& RequestID) const
{
	if (bIsEdgegapEnvironment)
	{
		RequestID = CachedRequestID;
		return true;
	}
	return false;
}

void UEdgegapServerShutdownSubsystem::CallSelfStopAPI(bool bWaitForResponse)
{
	if (bShutdownInitiated)
	{
		UE_LOG(LogTemp, Warning, TEXT("EdgegapServerShutdown: Self-stop API already called, ignoring duplicate call"));
		return;
	}
	
	if (!bIsEdgegapEnvironment || CachedDeleteURL.IsEmpty() || CachedDeleteToken.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("EdgegapServerShutdown: Cannot call self-stop API - ARBITRIUM_DELETE_URL or ARBITRIUM_DELETE_TOKEN is missing"));
		OnServerShutdown.Broadcast(false, TEXT("Self-stop runtime environment variables are missing"));
		return;
	}
	
	bShutdownInitiated = true;
	StopDeploymentInternal();
	
	if (bWaitForResponse)
	{
		// Wait up to 5 seconds for response
		float ElapsedTime = 0.0f;
		const float MaxWaitTime = 5.0f;
		
		if (UWorld* World = GetWorld())
		{
			while (ElapsedTime < MaxWaitTime && ShutdownRequest.IsValid())
			{
				World->GetTimerManager().Tick(0.1f);
				ElapsedTime += 0.1f;
			}
		}
	}
}

void UEdgegapServerShutdownSubsystem::StopDeploymentInternal()
{
	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		UE_LOG(LogTemp, Error, TEXT("EdgegapServerShutdown: Could not get HTTP module"));
		OnServerShutdown.Broadcast(false, TEXT("Failed to initialize HTTP module"));
		return;
	}

	ShutdownRequest = Http->CreateRequest();
	ShutdownRequest->SetURL(CachedDeleteURL);
	ShutdownRequest->SetVerb("DELETE");
	ShutdownRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	ShutdownRequest->SetHeader(TEXT("Authorization"), *CachedDeleteToken);
	ShutdownRequest->SetTimeout(5);

	ShutdownRequest->OnProcessRequestComplete().BindUObject(this, &UEdgegapServerShutdownSubsystem::OnShutdownResponseReceived);

	UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: Calling self-stop API (URL: %s)"), *CachedDeleteURL);

	if (!ShutdownRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Error, TEXT("EdgegapServerShutdown: Failed to process shutdown request"));
		OnServerShutdown.Broadcast(false, TEXT("Failed to process HTTP request"));
		ShutdownRequest.Reset();
	}
}

void UEdgegapServerShutdownSubsystem::OnShutdownResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	bool bSuccess = false;
	FString Message;
	
	if (HttpResponse.IsValid())
	{
		int32 ResponseCode = HttpResponse->GetResponseCode();
		if (ResponseCode >= 200 && ResponseCode < 300)
		{
			bSuccess = true;
			Message = FString::Printf(TEXT("Successfully called self-stop API (Response: %d)"), ResponseCode);
			UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: %s"), *Message);
		}
		else
		{
			Message = FString::Printf(TEXT("Self-stop API returned error code: %d"), ResponseCode);
			UE_LOG(LogTemp, Warning, TEXT("EdgegapServerShutdown: %s"), *Message);
		}
	}
	else
	{
		Message = TEXT("Self-stop API request failed or no response received");
		UE_LOG(LogTemp, Warning, TEXT("EdgegapServerShutdown: %s"), *Message);
	}
	
	ShutdownRequest.Reset();
	OnServerShutdown.Broadcast(bSuccess, Message);
}

void UEdgegapServerShutdownSubsystem::SetupShutdownHooks()
{
	// Register console command for RequestExit
	// Note: Using a lambda to capture 'this' since the command is registered statically
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Edgegap.RequestExit"),
		TEXT("Request server exit and call self-stop API"),
		FConsoleCommandDelegate::CreateLambda([this]()
		{
			HandleRequestExitCommand();
		})
	);
	
	// Hook into PreExit to catch application termination
	FCoreDelegates::OnPreExit.AddLambda([this]()
	{
		if (!bShutdownInitiated)
		{
			UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: PreExit detected, calling self-stop API"));
			CallSelfStopAPI(false);
		}
	});
	
	// Hook into Engine shutdown
	FCoreDelegates::OnExit.AddLambda([this]()
	{
		if (!bShutdownInitiated)
		{
			UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: Engine exit detected, calling self-stop API"));
			CallSelfStopAPI(false);
		}
	});
}

void UEdgegapServerShutdownSubsystem::HandleRequestExitCommand()
{
	UE_LOG(LogTemp, Log, TEXT("EdgegapServerShutdown: RequestExit command received"));
	CallSelfStopAPI(true);
	
	// Request engine exit after calling stop API
	if (GEngine)
	{
		FGenericPlatformMisc::RequestExit(false);
	}
}

void UEdgegapServerShutdownSubsystem::SetupCrashHandlers()
{
	// Set up crash handler using a lambda wrapper since the signature may vary
	FCoreDelegates::OnHandleSystemError.AddLambda([]()
	{
		StaticCrashHandler(TEXT("System error detected"), false);
	});
}

void UEdgegapServerShutdownSubsystem::StaticCrashHandler(const TCHAR* ErrorMessage, bool bIsAssert)
{
	// Try to call self-stop API on crash
	// Note: This is a best-effort attempt as the process may terminate quickly
	UE_LOG(LogTemp, Error, TEXT("EdgegapServerShutdown: Crash detected - %s"), ErrorMessage);
	
	// Try to get the subsystem and call stop API
	// This may not always work during crash, but we try
	// Access GameInstance through the world if available
	if (GEngine && GWorld)
	{
		if (UGameInstance* GameInstance = GWorld->GetGameInstance())
		{
			if (UEdgegapServerShutdownSubsystem* Subsystem = GameInstance->GetSubsystem<UEdgegapServerShutdownSubsystem>())
			{
				if (!Subsystem->bShutdownInitiated)
				{
					Subsystem->CallSelfStopAPI(false);
				}
			}
		}
	}
}

