// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "APIToken/APITokenSettings.h"
#include "Engine/DeveloperSettings.h"
#include "Settings/ProjectPackagingSettings.h"
#include "EdgegapSettings.generated.h"

UENUM()
enum class EEdgegapBuildMode : uint8
{
	LocalBuild UMETA(DisplayName = "Local Build (requires Linux cross-compilation)"),
	DockerBuild UMETA(DisplayName = "Docker Build (no source required)")
};

USTRUCT(BlueprintType)
struct FEdgegapPortConfig
{
	GENERATED_BODY()

	// The internal port number
	UPROPERTY(Config, EditAnywhere, Category = "Port Config", DisplayName = "Port Number")
	int32 Port = 7777;

	// The protocol for this port (TCP, UDP, TCP/UDP)
	UPROPERTY(Config, EditAnywhere, Category = "Port Config", DisplayName = "Protocol")
	FString Protocol = TEXT("TCP/UDP");

	// Whether to check this port
	UPROPERTY(Config, EditAnywhere, Category = "Port Config", DisplayName = "Check Port") 
	bool bToCheck = false;

	// Whether to upgrade to TLS
	UPROPERTY(Config, EditAnywhere, Category = "Port Config", DisplayName = "TLS Upgrade")
	bool bTLSUpgrade = false;

	// Name of the port
	UPROPERTY(Config, EditAnywhere, Category = "Port Config", DisplayName = "Port Name")
	FString Name = TEXT("gameport");
};

USTRUCT(BlueprintType)
struct FEdgegapEnvironmentVariable
{
	GENERATED_BODY()

	// The environment variable key
	UPROPERTY(Config, EditAnywhere, Category = "Environment Variable", DisplayName = "Key")
	FString Key;

	// The environment variable value
	UPROPERTY(Config, EditAnywhere, Category = "Environment Variable", DisplayName = "Value")
	FString Value;

	// Whether this variable contains sensitive information
	UPROPERTY(Config, EditAnywhere, Category = "Environment Variable", DisplayName = "Is Hidden")
	bool bIsHidden = false;
};

UCLASS(config=EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Edgegap Plugin"))
class UEdgegapSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEdgegapSettings();

	//~ Begin UObject Interface
#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateSettings, UEdgegapSettings const*);
	static FOnUpdateSettings OnSettingsChange;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
	//~ End UObject Interface

	// Main Edgegap API key. SERVER-ONLY: do not ship in client builds.
	// For dedicated servers, prefer setting the EDGEGAP_API_KEY environment variable.
	UPROPERTY(Config, EditAnywhere, Category = "API Key", DisplayName = "Authorization Key")
	FString AuthorizationKey;

	// Legacy token field kept for backward compatibility. AuthorizationKey is the active field.
	UPROPERTY(Config)
	FAPITokenSettings APIToken;

	UPROPERTY(Config, EditAnywhere, Category = "Docker Settings", DisplayName = "Docker Path")
	FString DockerPath;

	UPROPERTY(Config, EditAnywhere, Category = "Build Configurations", DisplayName = "Build Configurations")
	EProjectPackagingBuildConfigurations BuildConfiguration = EProjectPackagingBuildConfigurations::PPBC_Development;
	
	UPROPERTY(Config, EditAnywhere, Category = "Build Configurations", DisplayName = "Target Name Override")
	FString OverridableTargetName = FString::Printf(TEXT("%sServer"), FApp::GetProjectName());

	UPROPERTY(Config, EditAnywhere, Category = "Build Configurations", DisplayName = "Build Mode")
	EEdgegapBuildMode BuildMode = EEdgegapBuildMode::LocalBuild;

	// GitHub username for ghcr.io login (must have access to Epic's UE Docker images)
	UPROPERTY(Config, EditAnywhere, Category = "Build Configurations|Docker Build",
		Meta = (EditCondition = "BuildMode == EEdgegapBuildMode::DockerBuild", EditConditionHides),
		DisplayName = "GitHub Username")
	FString GitHubUsername;

	// GitHub Personal Access Token with read:packages scope
	UPROPERTY(Config, EditAnywhere, Category = "Build Configurations|Docker Build",
		Meta = (EditCondition = "BuildMode == EEdgegapBuildMode::DockerBuild", EditConditionHides),
		DisplayName = "GitHub PAT (read:packages)")
	FString GitHubPAT;

	// UE Docker image tag (e.g., "dev-5.5.4", "dev-5.4.4")
	UPROPERTY(Config, EditAnywhere, Category = "Build Configurations|Docker Build",
		Meta = (EditCondition = "BuildMode == EEdgegapBuildMode::DockerBuild", EditConditionHides),
		DisplayName = "UE Docker Image Tag")
	FString UEDockerImageTag = TEXT("dev-5.5.4");

	// Additional build arguments passed to RunUAT.sh inside the Docker container
	UPROPERTY(Config, EditAnywhere, Category = "Build Configurations|Docker Build",
		Meta = (EditCondition = "BuildMode == EEdgegapBuildMode::DockerBuild", EditConditionHides),
		DisplayName = "Extra Build Args")
	FString DockerExtraBuildArgs;

	UPROPERTY(Config, EditAnywhere, Category = "Application Info", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Application Name")
	FText ApplicationName;

	UPROPERTY(Config, EditAnywhere, Category = "Application Info", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Application Image")
	FFilePath ImagePath;

	//Units of vCPU needed (1024 = 1vcpu)
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Extra Settings", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Required CPU")
	int32 RequiredCPU = 1024;

	//Units of memory in MB needed (1024 = 1GB)
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Extra Settings", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Required Memory")
	int32 RequiredMemory = 1024;

	//Units of GPU needed (1024 = 1 GPU)
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Extra Settings", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Required GPU")
	int32 RequiredGPU = 0;

	//The Max duration of the game in minute. 0 means forever.
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Extra Settings", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Max Duration")
	int32 MaxDuration = 0;

	//Estimated maximum time in seconds to deploy, after this time we will consider it not working and retry.
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Extra Settings", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Estimated Maximum Deploy Time")
	int32 EstimatedDeployTime = 120;

	//Entrypoint/Command override of your Container - Leave empty to use the default one
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Extra Settings", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Entrypoint Override")
	FString EntrypointOverride;
	
	//The Arguments to pass to the command
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Extra Settings", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Command Arguments")
	FString CommandArguments;
	
	// Port configurations to include in every deployment
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Ports and Environment", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Additional Ports")
	TArray<FEdgegapPortConfig> AdditionalPorts;
	
	// Default environment variables to include in every deployment
	UPROPERTY(Config, EditAnywhere, Category = "Application Info|Ports and Environment", Meta = (EditCondition = "bIsTokenVerified"), DisplayName = "Default Environment Variables")
	TArray<FEdgegapEnvironmentVariable> DefaultEnvironmentVariables;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry")
	bool bUseCustomContainerRegistry = false;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry", Meta = (EditCondition = "bUseCustomContainerRegistry"), DisplayName = "Registry URL")
	FString Registry;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry", Meta = (EditCondition = "bUseCustomContainerRegistry"), DisplayName = "Repository")
	FString ImageRepository;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry", Meta = (EditCondition = "bUseCustomContainerRegistry"), DisplayName = "Username")
	FString PrivateRegistryUsername;

	UPROPERTY(Config, EditAnywhere, Category = "Container Registry", Meta = (EditCondition = "bUseCustomContainerRegistry"), DisplayName = "Token")
	FString PrivateRegistryToken;

	// Server Browser URL (unique per app)
	UPROPERTY(Config, EditAnywhere, Category = "Server Browser", DisplayName = "Server Browser URL")
	FString ServerBrowserURL;

	// Server Token for server-side operations (register, heartbeat, slots, confirm)
	UPROPERTY(Config, EditAnywhere, Category = "Server Browser", DisplayName = "Server Token")
	FString ServerBrowserServerToken;

	// Client Token for client-side operations (browse, reserve seats)
	UPROPERTY(Config, EditAnywhere, Category = "Server Browser", DisplayName = "Client Token")
	FString ServerBrowserClientToken;

	// Matchmaker URL (unique per matchmaker config)
	UPROPERTY(Config, EditAnywhere, Category = "Matchmaking", DisplayName = "Matchmaker URL")
	FString MatchmakingURL;

	// Matchmaker auth token used by matchmaking/group/backfill APIs.
	// Leave empty if you provide tokens dynamically at runtime.
	UPROPERTY(Config, EditAnywhere, Category = "Matchmaking", DisplayName = "Auth Token")
	FString MatchmakingAuthToken;

	UPROPERTY(Config)
	FString Tag;

	UPROPERTY(Config)
	FString VersionName;

	//@TODO: Check the best way to handle verification and toggle edit conditions accordingly
	UPROPERTY(Config, EditAnywhere, Category = "Application Info")
	bool bIsTokenVerified = true;
};
