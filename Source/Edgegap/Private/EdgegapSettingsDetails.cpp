// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EdgegapSettingsDetails.h"
#include "EdgegapSettings.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Settings/PlatformsMenuSettings.h"
#include "EditorDirectories.h"
#include "Fonts/SlateFontInfo.h"
#include "Kismet/KismetMathLibrary.h"

#include "Misc/MessageDialog.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/TabManager.h"

#include "AboutScreen.h"
#include "CreditsScreen.h"
#include "DesktopPlatformModule.h"
#include "ISourceControlModule.h"
#include "GameProjectGenerationModule.h"
#include "Toolkits/GlobalEditorCommonCommands.h"
#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "SourceCodeNavigation.h"
#include "SourceControlWindows.h"
#include "ISettingsModule.h"
#include "Interfaces/IProjectManager.h"
#include "Interfaces/ITargetPlatform.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "PlatformInfo.h"
#include "EditorStyleSet.h"
#include "Settings/EditorExperimentalSettings.h"
#include "CookerSettings.h"
#include "UnrealEdMisc.h"
#include "FileHelpers.h"
#include "EditorAnalytics.h"
#include "LevelEditor.h"
#include "Interfaces/IProjectTargetPlatformEditorModule.h"
#include "InstalledPlatformInfo.h"
#include "Misc/ConfigCacheIni.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Framework/Commands/GenericCommands.h"
#include "Dialogs/SOutputLogDialog.h"
#include "IUATHelperModule.h"


#include "TargetReceipt.h"
#include "UnrealEdGlobals.h"
#include "Async/Async.h"
#include "Editor/UnrealEdEngine.h"

#include "Settings/EditorSettings.h"
#include "AnalyticsEventAttribute.h"
#include "Kismet2/DebuggerCommands.h"
#include "GameMapsSettings.h"
#include "DerivedDataCacheInterface.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"
#include "JsonObjectConverter.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "GenericPlatform/GenericPlatformFile.h"

#include "IDetailChildrenBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "GeneralProjectSettings.h"
#include "SExternalImageReference.h"

DEFINE_LOG_CATEGORY(EdgegapLog);

#define LOCTEXT_NAMESPACE "EdgegapModule"

#define EDGEGAP_API_URL "https://api.edgegap.com/"

namespace
{
	FString GetDeployerToken(const UEdgegapSettings* EdgegapSettings, FString* OutSource = nullptr)
	{
		if (!EdgegapSettings)
		{
			if (OutSource)
			{
				*OutSource = TEXT("None");
			}
			return FString();
		}

		// Backward compatibility: fall back to AuthorizationKey if Deployer Key is not set.
		if (!EdgegapSettings->APIToken.APIToken.IsEmpty())
		{
			if (OutSource)
			{
				*OutSource = TEXT("DeployerKey");
			}
			return EdgegapSettings->APIToken.APIToken;
		}

		if (OutSource)
		{
			*OutSource = TEXT("AuthorizationKeyFallback");
		}
		return EdgegapSettings->AuthorizationKey;
	}

	FString ResolveServerLaunchTargetName(const UEdgegapSettings* EdgegapSettings)
	{
		if (!EdgegapSettings)
		{
			return FString::Printf(TEXT("%sServer"), FApp::GetProjectName());
		}

		const FString LaunchOverride = EdgegapSettings->StartScriptTargetNameOverride.TrimStartAndEnd();
		if (!LaunchOverride.IsEmpty())
		{
			return LaunchOverride;
		}

		if (!EdgegapSettings->OverridableTargetName.IsEmpty())
		{
			return EdgegapSettings->OverridableTargetName;
		}

		return FString::Printf(TEXT("%sServer"), FApp::GetProjectName());
	}
}

class SDeployStatusListItem
	: public SMultiColumnTableRow< TSharedPtr<struct FDeploymentStatusListItem> >
{
public:

	SLATE_BEGIN_ARGS(SDeployStatusListItem) { }
	SLATE_ARGUMENT(TSharedPtr<FDeploymentStatusListItem>, Item)
		SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		Item = InArgs._Item;
		check(Item.IsValid());
		const auto args = FSuperRowType::FArguments();
		SMultiColumnTableRow< TSharedPtr<FDeploymentStatusListItem> >::Construct(args, InOwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (ColumnName == TEXT("URL"))
		{
			return	SNew(SBox)
				.HeightOverride(20)
				.Padding(FMargin(3, 0))
				.VAlign(VAlign_Center)
				[
					SNew(SInlineEditableTextBlock)
					.Text(FText::FromString(Item->DeploymentIP))
					.Style(FCoreStyle::Get(), "InlineEditableTextBlockSmallStyle")
					.Cursor(EMouseCursor::TextEditBeam)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];
		}
		else if (ColumnName == TEXT("Status"))
		{
			return	SNew(SBox)
				.HeightOverride(20)
				.Padding(FMargin(3, 0))
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Item->DeploymentStatus))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];
		}
		else if (ColumnName == TEXT("Control"))
		{
			bool DeploymentReady = Item->DeploymentReady;
			return SNew(SBox)
				.HeightOverride(20)
				.Padding(FMargin(3, 0))
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Text(LOCTEXT("Stop", "Stop"))
					.IsEnabled(DeploymentReady)
					.OnClicked_Lambda([this]()
					{
						SetEnabled(false);

						FEdgegapSettingsDetails::GetInstance()->Request_StopDeploy(this->Item->RequestID, this->Item->API_Key);

						return(FReply::Handled());
					})
				];
		}

		return SNullWidget::NullWidget;
	}

	TSharedPtr< FDeploymentStatusListItem > Item;

};

FString FEdgegapSettingsDetails::_ImageName;
FString FEdgegapSettingsDetails::_RegistryURL;
FString FEdgegapSettingsDetails::_PrivateUsername;
FString FEdgegapSettingsDetails::_PrivateToken;
FString FEdgegapSettingsDetails::_API_key;
FString FEdgegapSettingsDetails::_AppName;
FString FEdgegapSettingsDetails::_VersionName;
FString FEdgegapSettingsDetails::_RecentTag;

TArray< TSharedPtr<FDeploymentStatusListItem > > FEdgegapSettingsDetails::DeployStatusOverrideListSource;
FEdgegapSettingsDetails* FEdgegapSettingsDetails::Singelton;

namespace{
	const TCHAR* GetUATCompilationFlags()
	{
		// We never want to compile editor targets when invoking UAT in this context.
		// If we are installed or don't have a compiler, we must assume we have a precompiled UAT.
		return TEXT("-nocompileeditor");
	}

	FString GetCookingOptionalParams()
	{
		FString OptionalParams;
		const UProjectPackagingSettings* const PackagingSettings = GetDefault<UProjectPackagingSettings>();

		if (PackagingSettings->bSkipEditorContent)
		{
			OptionalParams += TEXT(" -SkipCookingEditorContent");
		}

		if (FDerivedDataCacheInterface* DDC = GetDerivedDataCache())
		{
			OptionalParams += FString::Printf(TEXT(" -ddc=%s"), DDC->GetGraphName());
		}

		return OptionalParams;
	}

	void OnPackageCallaback(FString res, double num)
	{
		if (res != "Completed")
		{
			UE_LOG(EdgegapLog, Warning, TEXT("OnPackageCallaback: Could not package, message:%s"), *res);
			return;
		}

		UProjectPackagingSettings* PackagingSettings = Cast<UProjectPackagingSettings>(UProjectPackagingSettings::StaticClass()->GetDefaultObject());
		UPlatformsMenuSettings* PlatformsSettings = GetMutableDefault<UPlatformsMenuSettings>();

		TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FString("EdgegapIntegrationKit"));
		if (!Plugin.IsValid())
		{
			Plugin = IPluginManager::Get().FindPlugin(FString("UltimateCrossplayIntegrationKit"));
		}
		if (!Plugin.IsValid())
		{
			UE_LOG(EdgegapLog, Error, TEXT("EdgegapIntegrationKit plugin not found (also checked UltimateCrossplayIntegrationKit fallback)"));
			return;
		}
		FString PluginDir = Plugin->GetBaseDir();
		FString DockerFilePath = FPaths::Combine(PluginDir, FString("Dockerfile"));
		FString StartScriptPath = FPaths::Combine(PluginDir, FString("StartServer.sh"));
		FString ServerBuildPath = PlatformsSettings->StagingDirectory.Path; // PackagingSettings->StagingDirectory.Path;
		ServerBuildPath = FPaths::Combine(ServerBuildPath, FString("LinuxServer"));


		AsyncTask(ENamedThreads::GameThread, [DockerFilePath, StartScriptPath, ServerBuildPath] {

			const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
			const FString _Registry = EdgegapSettings->Registry;
			const FString _ImageRepository = EdgegapSettings->ImageRepository;
			const FString _PrivateRegistryUsername = EdgegapSettings->PrivateRegistryUsername;
			const FString _PrivateRegistryToken = EdgegapSettings->PrivateRegistryToken;

			const FString _Tag = FEdgegapSettingsDetails::_RecentTag;

			FEdgegapSettingsDetails::Containerize(DockerFilePath, StartScriptPath, ServerBuildPath, _Registry, _ImageRepository, _Tag, _PrivateRegistryUsername, _PrivateRegistryToken);
		});

	}

	void OnDockerLoginCallback(FString res, double num)
	{
		if (res != "Completed")
		{
			UE_LOG(EdgegapLog, Warning, TEXT("OnDockerLoginCallback: Could not login, message:%s"), *res);
			return;
		}
		const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
		const FString _Registry = EdgegapSettings->Registry;
		const FString _ImageRepository = EdgegapSettings->ImageRepository;
		const FString _PrivateRegistryUsername = EdgegapSettings->PrivateRegistryUsername;
		const FString _PrivateRegistryToken = EdgegapSettings->PrivateRegistryToken;
		const FString _AppName = EdgegapSettings->ApplicationName.ToString();

		const FString _Tag = FEdgegapSettingsDetails::_RecentTag;

		FString ImageName = FEdgegapSettingsDetails::MakeImageName(_Registry, _ImageRepository, _AppName, _Tag);

		AsyncTask(ENamedThreads::GameThread, [_Registry, _PrivateRegistryUsername, _PrivateRegistryToken, ImageName] {FEdgegapSettingsDetails::PushContainer(ImageName, _Registry, _PrivateRegistryUsername, _PrivateRegistryToken, true); });
	}

	void OnContainerizeCallback(FString res, double num)
	{
		if (res != "Completed")
		{
			UE_LOG(EdgegapLog, Warning, TEXT("OnContainerizeCallback: Could not generate container, message:%s"), *res);
			return;
		}

		const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
		const FString _Registry = EdgegapSettings->Registry;
		const FString _ImageRepository = EdgegapSettings->ImageRepository;
		const FString _PrivateRegistryUsername = EdgegapSettings->PrivateRegistryUsername;
		const FString _PrivateRegistryToken = EdgegapSettings->PrivateRegistryToken;
		const FString _AppName = EdgegapSettings->ApplicationName.ToString();

		const FString _Tag = FEdgegapSettingsDetails::_RecentTag;

		FString ImageName = FEdgegapSettingsDetails::MakeImageName(_Registry, _ImageRepository, _AppName, _Tag);

		AsyncTask(ENamedThreads::GameThread, [_Registry, _PrivateRegistryUsername, _PrivateRegistryToken, ImageName] {FEdgegapSettingsDetails::PushContainer(ImageName, _Registry, _PrivateRegistryUsername, _PrivateRegistryToken); });
	}	
	
	void OnPushContainerCallback(FString res, double num)
	{
		if (res != "Completed")
		{
			UE_LOG(EdgegapLog, Warning, TEXT("OnPushContainerCallback: Could not push container, message:%s"), *res);
			return;
		}

		const FString _TargetTag = FEdgegapSettingsDetails::_RecentTag;

		const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
		const FString _AppName = EdgegapSettings->ApplicationName.ToString();
		const FString _APIToken = GetDeployerToken(EdgegapSettings);
		const FString _Registry = EdgegapSettings->Registry;
		const FString _ImageRepository = EdgegapSettings->ImageRepository;
		const FString _PrivateRegistryUsername = EdgegapSettings->PrivateRegistryUsername;
		const FString _PrivateRegistryToken = EdgegapSettings->PrivateRegistryToken;

		// Making the container tag name and version name match
		const FString _VersionName = _TargetTag;
		const FString _Tag = _TargetTag;

		FNotificationInfo* Info = new FNotificationInfo(LOCTEXT("OperationSuccess", "Build and Push completed successfully"));
		Info->ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().QueueNotification(Info);

		FEdgegapSettingsDetails::CreateVersion(_AppName, _VersionName, _APIToken, _Registry, _ImageRepository, _Tag, _PrivateRegistryUsername, _PrivateRegistryToken);
	}

	// --- Docker Build callbacks ---

	void OnDockerBuild_GHCRLoginCallback(FString res, double num)
	{
		if (res != "Completed")
		{
			UE_LOG(EdgegapLog, Warning, TEXT("OnDockerBuild_GHCRLoginCallback: Could not login to ghcr.io, message:%s"), *res);

			FNotificationInfo* Info = new FNotificationInfo(LOCTEXT("GHCRLoginFailed", "Failed to login to ghcr.io. Ensure your GitHub PAT has read:packages scope and your account is linked to Epic Games."));
			Info->ExpireDuration = 5.0f;
			FSlateNotificationManager::Get().QueueNotification(Info);
			return;
		}

		UE_LOG(EdgegapLog, Log, TEXT("Successfully logged into ghcr.io"));

		const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
		const FString _Registry = EdgegapSettings->Registry;
		const FString _ImageRepository = EdgegapSettings->ImageRepository;
		const FString _AppName = EdgegapSettings->ApplicationName.ToString();
		const FString _Tag = FEdgegapSettingsDetails::_RecentTag;

		FString ImageName = FEdgegapSettingsDetails::MakeImageName(_Registry, _ImageRepository, _AppName, _Tag);
		FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

		AsyncTask(ENamedThreads::GameThread, [ImageName, ProjectDir] {
			FEdgegapSettingsDetails::DockerBuild_Build(ImageName, ProjectDir);
		});
	}

	void OnDockerBuild_BuildCallback(FString res, double num)
	{
		if (res != "Completed")
		{
			UE_LOG(EdgegapLog, Warning, TEXT("OnDockerBuild_BuildCallback: Docker build failed, message:%s"), *res);

			FNotificationInfo* Info = new FNotificationInfo(LOCTEXT("DockerBuildFailed", "Docker Build failed. Check Output Log for details."));
			Info->ExpireDuration = 5.0f;
			FSlateNotificationManager::Get().QueueNotification(Info);
			return;
		}

		UE_LOG(EdgegapLog, Log, TEXT("Docker Build completed successfully"));

		// Proceed to push (reuses existing push flow)
		const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
		const FString _Registry = EdgegapSettings->Registry;
		const FString _ImageRepository = EdgegapSettings->ImageRepository;
		const FString _PrivateRegistryUsername = EdgegapSettings->PrivateRegistryUsername;
		const FString _PrivateRegistryToken = EdgegapSettings->PrivateRegistryToken;
		const FString _AppName = EdgegapSettings->ApplicationName.ToString();
		const FString _Tag = FEdgegapSettingsDetails::_RecentTag;

		FString ImageName = FEdgegapSettingsDetails::MakeImageName(_Registry, _ImageRepository, _AppName, _Tag);

		AsyncTask(ENamedThreads::GameThread, [_Registry, _PrivateRegistryUsername, _PrivateRegistryToken, ImageName] {
			FEdgegapSettingsDetails::PushContainer(ImageName, _Registry, _PrivateRegistryUsername, _PrivateRegistryToken);
		});
	}

	FString GetProjectPathForTurnkey()
	{
		if (FPaths::IsProjectFilePathSet())
		{
			return FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
		}
		if (FApp::HasProjectName())
		{
			FString ProjectPath = FPaths::ProjectDir() / FApp::GetProjectName() + TEXT(".uproject");
			if (FPaths::FileExists(ProjectPath))
			{
				return ProjectPath;
			}
			ProjectPath = FPaths::RootDir() / FApp::GetProjectName() / FApp::GetProjectName() + TEXT(".uproject");
			if (FPaths::FileExists(ProjectPath))
			{
				return ProjectPath;
			}
		}
		return FString();
	}
	
	bool CheckSupportedPlatforms(FName IniPlatformName)
	{
#if WITH_EDITOR
		return FModuleManager::LoadModuleChecked<IProjectTargetPlatformEditorModule>("ProjectTargetPlatformEditor").ShowUnsupportedTargetWarning(IniPlatformName);
#else
		return true;
#endif
	}

	UProjectPackagingSettings* GetPackagingSettingsForPlatform(FName IniPlatformName)
	{
		FString PlatformString = IniPlatformName.ToString();
		UProjectPackagingSettings* PackagingSettings = nullptr;
		for (TObjectIterator<UProjectPackagingSettings> Itr; Itr; ++Itr)
		{
			if (Itr->GetConfigPlatform() == PlatformString)
			{
				PackagingSettings = *Itr;
				break;
			}
		}
		if (PackagingSettings == nullptr)
		{
			PackagingSettings = NewObject<UProjectPackagingSettings>(GetTransientPackage());
			// Prevent object from being GCed.
			PackagingSettings->AddToRoot();
			// make sure any changes to DefaultGame are updated in this class
			PackagingSettings->LoadSettingsForPlatform(PlatformString);
		}

		return PackagingSettings;
	}

	bool ShouldBuildProject(UProjectPackagingSettings* PackagingSettings, const ITargetPlatform* TargetPlatform)
	{
		const UProjectPackagingSettings::FConfigurationInfo& ConfigurationInfo = UProjectPackagingSettings::ConfigurationInfo[(int)PackagingSettings->BuildConfiguration];

		// Get the target to build
		const FTargetInfo* Target = PackagingSettings->GetBuildTargetInfo();

		// Only build if the user elects to do so
		bool bBuild = false;
		if (PackagingSettings->Build == EProjectPackagingBuild::Always)
		{
			bBuild = true;
		}
		else if (PackagingSettings->Build == EProjectPackagingBuild::Never)
		{
			bBuild = false;
		}
		else if (PackagingSettings->Build == EProjectPackagingBuild::IfProjectHasCode)
		{
			bBuild = true;
			if (FApp::GetEngineIsPromotedBuild())
			{
				FString BaseDir;

				// Get the target name
				FString TargetName;
				if (Target == nullptr)
				{
					TargetName = TEXT("UnrealGame");
				}
				else
				{
					TargetName = Target->Name;
				}

				// Get the directory containing the receipt for this target, depending on whether the project needs to be built or not
				FString ProjectDir = FPaths::GetPath(FPaths::GetProjectFilePath());
				if (Target != nullptr && FPaths::IsUnderDirectory(Target->Path, ProjectDir))
				{
					UE_LOG(EdgegapLog, Log, TEXT("Selected target: %s"), *Target->Name);
					BaseDir = ProjectDir;
				}
				else
				{
					FText Reason;

					FGameProjectGenerationModule& GameProjectModule = FModuleManager::LoadModuleChecked<FGameProjectGenerationModule>(TEXT("GameProjectGeneration"));

					if (TargetPlatform->RequiresTempTarget(GameProjectModule.Get().ProjectHasCodeFiles(), ConfigurationInfo.Configuration, false, Reason))
					{
						UE_LOG(EdgegapLog, Log, TEXT("Project requires temp target (%s)"), *Reason.ToString());
						BaseDir = ProjectDir;
					}
					else
					{
						UE_LOG(EdgegapLog, Log, TEXT("Project does not require temp target"));
						BaseDir = FPaths::EngineDir();
					}
				}

				// Check if the receipt is for a matching promoted target
				FString UBTPlatformName = TargetPlatform->GetTargetPlatformInfo().DataDrivenPlatformInfo->UBTPlatformString;
				
				bool HasPromotedTarget(const TCHAR * BaseDir, const TCHAR * TargetName, const TCHAR * Platform, EBuildConfiguration Configuration, const TCHAR * Architecture);
				if (HasPromotedTarget(*BaseDir, *TargetName, *UBTPlatformName, ConfigurationInfo.Configuration, nullptr))
				{
					bBuild = false;
				}
			}
		}
		else if (PackagingSettings->Build == EProjectPackagingBuild::IfEditorWasBuiltLocally)
		{
			bBuild = !FApp::GetEngineIsPromotedBuild();
		}

		return bBuild;
	}

	bool DoesProjectHaveCode()
	{
		FGameProjectGenerationModule& GameProjectModule = FModuleManager::LoadModuleChecked<FGameProjectGenerationModule>(TEXT("GameProjectGeneration"));
		return GameProjectModule.Get().ProjectHasCodeFiles();
	}

	bool HasPromotedTarget(const TCHAR* BaseDir, const TCHAR* TargetName, const TCHAR* Platform, EBuildConfiguration Configuration, const TCHAR* Architecture)
	{
		// Get the path to the receipt, and check it exists
		const FString ReceiptPath = FTargetReceipt::GetDefaultPath(BaseDir, TargetName, Platform, Configuration, Architecture);
		if (!FPaths::FileExists(ReceiptPath))
		{
			UE_LOG(EdgegapLog, Log, TEXT("Unable to use promoted target - %s does not exist."), *ReceiptPath);
			return false;
		}

		// Read the receipt for this target
		FTargetReceipt Receipt;
		if (!Receipt.Read(ReceiptPath))
		{
			UE_LOG(EdgegapLog, Log, TEXT("Unable to use promoted target - cannot read %s"), *ReceiptPath);
			return false;
		}

		// Check the receipt is for a promoted build
		if (!Receipt.Version.IsPromotedBuild)
		{
			UE_LOG(EdgegapLog, Log, TEXT("Unable to use promoted target - receipt %s is not for a promoted target"), *ReceiptPath);
			return false;
		}

		// Make sure it matches the current build info
		FEngineVersion ReceiptVersion = Receipt.Version.GetEngineVersion();
		FEngineVersion CurrentVersion = FEngineVersion::Current();
		if (!ReceiptVersion.ExactMatch(CurrentVersion))
		{
			UE_LOG(EdgegapLog, Log, TEXT("Unable to use promoted target - receipt version (%s) is not exact match with current engine version (%s)"), *ReceiptVersion.ToString(), *CurrentVersion.ToString());
			return false;
		}

		// Print the matching target info
		UE_LOG(EdgegapLog, Log, TEXT("Found promoted target with matching version at %s"), *ReceiptPath);
		return true;
	}

};

TSharedRef<IDetailCustomization> FEdgegapSettingsDetails::MakeInstance()
{
	auto obj = MakeShareable(new FEdgegapSettingsDetails);
	Singelton = obj.Object;
	return obj;
}

FSlateBrush* FEdgegapSettingsDetails::LoadImage(const FString& InImagePath)
{
	if (!FSlateApplication::IsInitialized())
	{
		return nullptr;
	}

	if (SavedImageBrush.IsValid())
	{
		FSlateApplication::Get().GetRenderer()->ReleaseDynamicResource(*SavedImageBrush.Get());
		SavedImageBrush.Reset();
	}

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*InImagePath))
	{
		return nullptr;
	}

	TArray64<uint8> RawFileData;
	if (FFileHelper::LoadFileToArray(RawFileData, *InImagePath))
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		EImageFormat Format = ImageWrapperModule.DetectImageFormat(RawFileData.GetData(), RawFileData.Num());

		if (Format == EImageFormat::Invalid)
		{
			return nullptr;
		}

		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
		{
			TArray<uint8> RawData;
			if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
			{
				if (FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(*InImagePath, ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), RawData))
				{
					SavedImageBrush = MakeShareable(new FSlateDynamicImageBrush(*InImagePath, FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight())));
				}
			}
		}
	}

	return SavedImageBrush.Get();
}

void FEdgegapSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const FSlateColor BlueSlateColor = FSlateColor(FLinearColor(0.42f, 0.29f, 0.204f, 1.0f));

	IDetailCategoryBuilder& ApiInfoCategory = DetailBuilder.EditCategory("API Key");
	IDetailCategoryBuilder& ApplicationInfoCategory = DetailBuilder.EditCategory("Application Info");

	TSharedPtr<IPropertyHandle> IsTokenVerifiedProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, bIsTokenVerified));
	DetailBuilder.HideProperty(IsTokenVerifiedProperty);

	TSharedPtr<IPropertyHandle> AuthorizationKeyProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, AuthorizationKey));

	if (ApiInfoCategory.IsParentLayoutValid() && AuthorizationKeyProperty.IsValid())
	{
		ApiInfoCategory.AddProperty(AuthorizationKeyProperty);
	}

	// Warning: Authorization Key is a server-only key
	ApiInfoCategory.AddCustomRow(FText::FromString("KeyWarning"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 2)
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush("Icons.InfoWithColor"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.Padding(4, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AuthKeyWarning", "This key is stored in the Editor config hierarchy and will not be included in builds. For deployed servers, set the EDGEGAP_API_KEY environment variable."))
			.AutoWrapText(true)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	];

	// --- Application Info

	TSharedPtr<IPropertyHandle> ApplicationNameProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, ApplicationName));
	TSharedPtr<IPropertyHandle> VersionNameProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, VersionName));
	TSharedPtr<IPropertyHandle> ImagePathProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, ImagePath));
	TSharedPtr<IPropertyHandle> TagProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, Tag));

	// Customize the property's appearance and behavior
	if (ApplicationNameProperty.IsValid())
	{
		DetailBuilder.HideProperty(ApplicationNameProperty);

		AppNameWidgetRow = &ApplicationInfoCategory.AddCustomRow(FText::FromString("Application Name"))
		.NameContent()
		[
			ApplicationNameProperty->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			ApplicationNameProperty->CreatePropertyValueWidget()
		];
	}

	if (VersionNameProperty.IsValid())
	{
		ApplicationInfoCategory.AddProperty(VersionNameProperty);
	}

	if (ImagePathProperty.IsValid())
	{
		DetailBuilder.HideProperty(ImagePathProperty);

		// Customize the Application Image property's appearance and behavior

		AppImageWidgetRow = &ApplicationInfoCategory.AddCustomRow(FText::FromString("Application Image"))
			.NameContent()
			[
				ImagePathProperty->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SExternalImageReference, GetApplicationImageFilename(false), GetApplicationImageFilename(true))
				.IsEnabled_Lambda([ImagePathProperty]() -> bool
				{
					return ImagePathProperty.IsValid() && ImagePathProperty->IsEditable();
				})
				.FileDescription(LOCTEXT("Edgegap Appliction Image", "Edgegap Appliction Image"))
				.OnPreExternalImageCopy(FOnPreExternalImageCopy::CreateSP(this, &FEdgegapSettingsDetails::HandlePreExternalIconCopy))
				.OnGetPickerPath(FOnGetPickerPath::CreateSP(this, &FEdgegapSettingsDetails::GetPickerPath))
				.OnPostExternalImageCopy(FOnPostExternalImageCopy::CreateSP(this, &FEdgegapSettingsDetails::HandlePostExternalIconCopy))
			];
	}

	// Create Application Button

	TSharedRef<SHorizontalBox> CreateApplicationBtn_HB = SNew(SHorizontalBox);

	CreateApplicationBtn_HB->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(2.f)
		[
			SAssignNew(CreateApplication_SBtn, SButton)
			.IsEnabled_Lambda([ApplicationNameProperty]() -> bool
			{
				return ApplicationNameProperty.IsValid() && ApplicationNameProperty->IsEditable();
			})
			.Text(FText::FromString("Create Application"))
			.OnClicked(FOnClicked::CreateLambda([this]() {
				CreateApplication_SBtn->SetEnabled(false);

				Request_CreateApplication(CreateApplication_SBtn);
				return FReply::Handled();
			}))
		];

	ApplicationInfoCategory.AddCustomRow(LOCTEXT("ApplicationInfo", "Application Info"))
		.ValueContent()
		[
			CreateApplicationBtn_HB
		];

	// --- Container Registry

	IDetailCategoryBuilder& ContainerRegistryCategory = DetailBuilder.EditCategory(TEXT("Container Registry"));

	ContainerRegistryCategory.InitiallyCollapsed(false); // Open by default so the user can click Build and Push
	ContainerRegistryCategory.RestoreExpansionState(true);

	TSharedPtr<IPropertyHandle> bUseCustomContainerRegistryProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, bUseCustomContainerRegistry));
	TSharedPtr<IPropertyHandle> RegistryProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, Registry));
	TSharedPtr<IPropertyHandle> ImageRepositoryProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, ImageRepository));
	TSharedPtr<IPropertyHandle> PrivateRegistryUsernameProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, PrivateRegistryUsername));
	TSharedPtr<IPropertyHandle> PrivateRegistryTokenProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UEdgegapSettings, PrivateRegistryToken));

	ContainerRegistryCategory.AddProperty(bUseCustomContainerRegistryProperty);
	ContainerRegistryCategory.AddProperty(RegistryProperty);
	ContainerRegistryCategory.AddProperty(ImageRepositoryProperty);
	ContainerRegistryCategory.AddProperty(PrivateRegistryUsernameProperty);
	ContainerRegistryCategory.AddProperty(PrivateRegistryTokenProperty);

	// Create Build and Push

	TSharedRef<SHorizontalBox> CreateBuildAndPushBtn_HB = SNew(SHorizontalBox);

	CreateBuildAndPushBtn_HB->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(2.f)
		[
			SAssignNew(CreateBuilAndPushBtn_SBtn, SButton)
			.Text_Lambda([]() -> FText
			{
				const UEdgegapSettings* EGSettings = GetDefault<UEdgegapSettings>();
				if (EGSettings->BuildMode == EEdgegapBuildMode::DockerBuild)
				{
					return FText::FromString("Docker Build and Push");
				}
				return FText::FromString("Build and Push");
			})
			.IsEnabled_Lambda([this, RegistryProperty, ImageRepositoryProperty, PrivateRegistryUsernameProperty, PrivateRegistryTokenProperty]() -> bool
			{
				FString RegistryStr;
				FString ImageRepositoryStr;
				FString PrivateRegistryUsernameStr;
				FString PrivateRegistryTokenStr;

				RegistryProperty->GetValue(RegistryStr);
				ImageRepositoryProperty->GetValue(ImageRepositoryStr);
				PrivateRegistryUsernameProperty->GetValue(PrivateRegistryUsernameStr);
				PrivateRegistryTokenProperty->GetValue(PrivateRegistryTokenStr);

				bool bIsClickable = !RegistryStr.IsEmpty() && !ImageRepositoryStr.IsEmpty() && !PrivateRegistryUsernameStr.IsEmpty() && !PrivateRegistryTokenStr.IsEmpty();

				// For Docker Build mode, also validate GitHub credentials
				const UEdgegapSettings* EGSettings = GetDefault<UEdgegapSettings>();
				if (EGSettings->BuildMode == EEdgegapBuildMode::DockerBuild)
				{
					bIsClickable = bIsClickable && !EGSettings->GitHubUsername.IsEmpty() && !EGSettings->GitHubPAT.IsEmpty() && !EGSettings->UEDockerImageTag.IsEmpty();
				}

				return bIsClickable;
 			})
			.OnClicked(FOnClicked::CreateLambda([this]() {
				const UEdgegapSettings* EGSettings = GetDefault<UEdgegapSettings>();
				if (EGSettings->BuildMode == EEdgegapBuildMode::DockerBuild)
				{
					DockerBuildAndPush();
				}
				else
				{
					PackageProject("linux");
				}
				return FReply::Handled();
			}))
		];

	// The button should be always visible, and this category persists its collapse state, so moving it to the Application Info category
	// Update: We still want it in ContainerRegistryCategory but open up the category on click
	// ApplicationInfoCategory
	ContainerRegistryCategory.AddCustomRow(LOCTEXT("BuildAndPush", "Build and Push"))
		.ValueContent()
		[
			CreateBuildAndPushBtn_HB
		];

	// --- Deployments

	IDetailCategoryBuilder& DepStatusCategory = DetailBuilder.EditCategory("Deployments");

	DepStatusCategory.AddCustomRow(LOCTEXT("DeploymentStatus", "Deployments"))
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.Padding(2)
		.AutoWidth()
		[
			SAssignNew(DeploymentStatuRefresh_SBtn, SButton)
			.Text(LOCTEXT("Refresh", "Refresh"))
		.OnClicked_Lambda([this]()
			{
				DeploymentStatuRefresh_SBtn->SetEnabled(false);

				const UEdgegapSettings* EdgegapSettings = GetDefault<UEdgegapSettings>();
				Request_GetDeploymentsInfo(GetDeployerToken(EdgegapSettings), DeploymentStatuRefresh_SBtn);
				return(FReply::Handled());
			})
		]
	+ SHorizontalBox::Slot()
		.Padding(2)
		.AutoWidth()
		[
			SAssignNew(CreateNewDeployment_SBtn, SButton)
			.Text(LOCTEXT("CreateNewDeployment", "Create New Deployment"))
		.OnClicked_Lambda([this, ApplicationNameProperty, VersionNameProperty]()
			{
				CreateNewDeployment_SBtn->SetEnabled(false);

				FText AppNameTxt;
				FString VersionNameStr;
				ApplicationNameProperty->GetValue(AppNameTxt);
				VersionNameProperty->GetValue(VersionNameStr);

				const UEdgegapSettings* EdgegapSettings = GetDefault<UEdgegapSettings>();
				Request_DeployApp(AppNameTxt.ToString(), VersionNameStr, GetDeployerToken(EdgegapSettings), CreateNewDeployment_SBtn);
				return(FReply::Handled());
			})
		]
		];

	DepStatusCategory.AddCustomRow(LOCTEXT("CurrentDeployments", "Current Deployments"))
		[
			SAssignNew(DeploymentStatusListItemListView, SDeploymentStatusListItemListView)
		.ListItemsSource(&DeployStatusOverrideListSource)
		.OnGenerateRow(this, &FEdgegapSettingsDetails::HandleGenerateDeployStatusWidget)
		.SelectionMode(ESelectionMode::None)
		.HeaderRow(
			SNew(SHeaderRow)
			+ SHeaderRow::Column("URL")
			.HAlignCell(HAlign_Left)
			.FillWidth(1)
			.HeaderContentPadding(FMargin(4))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CurrentDeployments_URL", "URL"))
			.Visibility(EVisibility::Visible)
			.IsEnabled(true)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			]
	+ SHeaderRow::Column("Status")
		.HAlignCell(HAlign_Left)
		.FillWidth(1)
		.HeaderContentPadding(FMargin(4))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CurrentDeployments_Status", "Status"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	+ SHeaderRow::Column("Control")
		.HAlignCell(HAlign_Left)
		.FillWidth(1)
		.HeaderContentPadding(FMargin(4))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CurrentDeployments_Control", "Control"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	)
		];

	// --- Matchmaking Key Warnings
	IDetailCategoryBuilder& MatchmakingCategory = DetailBuilder.EditCategory("Matchmaking");

	MatchmakingCategory.AddCustomRow(FText::FromString("MMKeyInfo"))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 4, 0)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.InfoWithColor"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MMTokenInfo", "Matchmaker URL and Auth Token are stored in DefaultEngine.ini and may be included in builds. For runtime overrides, prefer EDGEGAP_MM_URL and EDGEGAP_MM_AUTH_TOKEN environment variables."))
				.AutoWrapText(true)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
	];

	// --- Server Browser Key Warnings
	IDetailCategoryBuilder& ServerBrowserCategory = DetailBuilder.EditCategory("Server Browser");

	ServerBrowserCategory.AddCustomRow(FText::FromString("SBKeyInfo"))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 4, 0)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.InfoWithColor"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SBServerTokenWarning", "Server Token is stored in the Editor config hierarchy and will not be included in builds. For deployed servers, set the EDGEGAP_SB_SERVER_TOKEN environment variable."))
				.AutoWrapText(true)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 4, 0)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.InfoWithColor"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SBClientTokenInfo", "Client Token and Server Browser URL are stored in DefaultEngine.ini and will be included in builds."))
				.AutoWrapText(true)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
	];

	// --- Ad

	IDetailCategoryBuilder& AdCategoryBuilder = DetailBuilder.EditCategory("Edgegap");

	AdCategoryBuilder.AddCustomRow(LOCTEXT("Edgegap", "Edgegap"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Bottom)
						[
							SNew(STextBlock)
							.Font(IDetailLayoutBuilder::GetDetailFontBold())
							.AutoWrapText(true)
							.Text(LOCTEXT("Ad", "Need more than one game server?"))
						]
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(10.f, 0.f, 10.f, 10.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Bottom)
						.Padding(2)
						[
							SNew(SButton)
							.Text(FText::FromString("Documentation"))
							.OnClicked(FOnClicked::CreateLambda([this]() {
								FPlatformProcess::LaunchURL(TEXT("https://egik.betide.studio/"), NULL, NULL);
								return(FReply::Handled());
							}))
						]
					+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Bottom)
						.Padding(2)
						[
							SNew(SButton)
							.Text(LOCTEXT("ClickHere", "Click here!"))
							.ForegroundColor(BlueSlateColor)
							.OnClicked_Lambda([this]()
							{
								FPlatformProcess::LaunchURL(TEXT("https://app.edgegap.com/user-settings?tab=memberships"), NULL, NULL);
								return(FReply::Handled());
							})
						]
				]
		]
	];

	// Get Defaults

	const UEdgegapSettings* EdgegapSettings = GetDefault<UEdgegapSettings>();
	Request_GetDeploymentsInfo(GetDeployerToken(EdgegapSettings), nullptr);
	Request_VerifyToken();

	// --- Binds and Delegates

	OnIsTokenVerifiedChanged.BindLambda([this, IsTokenVerifiedProperty](bool bIsVerified)
		{
			IsTokenVerifiedProperty->SetValue(bIsVerified);

			AppNameWidgetRow->IsEnabled(bIsVerified);
			AppImageWidgetRow->IsEnabled(bIsVerified);
		});

	bool bIsTokenVerified = true;
	if (IsTokenVerifiedProperty.IsValid())
	{
		IsTokenVerifiedProperty->GetValue(bIsTokenVerified);
	}
	if (AppNameWidgetRow)
	{
		AppNameWidgetRow->IsEnabled(bIsTokenVerified);
	}
	if (AppImageWidgetRow)
	{
		AppImageWidgetRow->IsEnabled(bIsTokenVerified);
	}
}

bool FEdgegapSettingsDetails::HandlePreExternalIconCopy(const FString& InChosenImage)
{
	return true;
}

FString FEdgegapSettingsDetails::GetPickerPath()
{
	return FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_OPEN);
}


bool FEdgegapSettingsDetails::HandlePostExternalIconCopy(const FString& InChosenImage)
{
	UEdgegapSettings* MutableEdgegapSettings = GetMutableDefault<UEdgegapSettings>();
	MutableEdgegapSettings->ImagePath.FilePath = InChosenImage;
	MutableEdgegapSettings->SaveConfig();

	FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_OPEN, FPaths::GetPath(InChosenImage));
	return true;
}

void FEdgegapSettingsDetails::SaveAll()
{
	const bool bPromptUserToSave = false;
	const bool bSaveMapPackages = true;
	const bool bSaveContentPackages = true;
	const bool bFastSave = false;
	const bool bNotifyNoPackagesSaved = false;
	const bool bCanBeDeclined = false;
	FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave, bNotifyNoPackagesSaved, bCanBeDeclined);
}

bool FEdgegapSettingsDetails::CheckDockerRunning()
{
	int32 ReturnCode;
	FString OutString;
	FString ErrString;

	UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
	const FString DockerPath = EdgegapSettings->DockerPath;
	if(DockerPath.IsEmpty())
	{
		UE_LOG(EdgegapLog, Error, TEXT("Docker path is empty. Please set the path to Docker in the Edgegap settings."));
		return false;
	}
	//Check if file exists
	if (!FPaths::FileExists(DockerPath))
	{
		UE_LOG(EdgegapLog, Error, TEXT("Docker path is invalid. Please set the path to Docker in the Edgegap settings."));
		return false;
	}

	// Command to check if Docker is running
	FString CommandLine = TEXT("info");

	// Executes the docker info command using the full path
	FPlatformProcess::ExecProcess(*DockerPath, *CommandLine, &ReturnCode, &OutString, &ErrString);

	if (ReturnCode != 0)
	{
		// Docker is not running, handle the error
		UE_LOG(EdgegapLog, Error, TEXT("Docker isn't running - please start Docker and try again!"));
		return false;
	}

	// If Docker is running, proceed with the script
	UE_LOG(EdgegapLog, Log, TEXT("Docker is running! Proceeding with containerization."));
	return true;
}


void FEdgegapSettingsDetails::PackageProject(const FName IniPlatformName)
{
	if(!CheckDockerRunning())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("DockerNotRunning", "Docker isn't running - please start Docker and try again!"));
		return;
	}
	
	// Handle Build and Push button
	const UGeneralProjectSettings& ProjectSettings = *GetDefault<UGeneralProjectSettings>();
	UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();

	// Prepare the Tag beforehand

	FDateTime Now = FDateTime::Now();
	FString FormattedTime = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M"));

	_RecentTag = FormattedTime;

	// get a in-memory defaults which will have the user-settings, like the per-platform config/target platform stuff
	UProjectPackagingSettings* AllPlatformPackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
	UPlatformsMenuSettings* PlatformsSettings = GetMutableDefault<UPlatformsMenuSettings>();

	// installed builds only support standard Game type builds (not Client, Server, etc) so instead of looking up a setting that the user can't set, 
	// always use the base PlatformInfo for Game builds, which will be named the same as the platform itself
	const PlatformInfo::FTargetPlatformInfo* PlatformInfo = nullptr;
	if (FApp::IsInstalled())
	{
		PlatformInfo = PlatformInfo::FindPlatformInfo(IniPlatformName);
	}
	else
	{
		PlatformInfo = PlatformInfo::FindPlatformInfo(PlatformsSettings->GetTargetFlavorForPlatform(IniPlatformName));
	}
	// this is unexpected to be able to happen, but it could if there was a bad value saved in the UProjectPackagingSettings - if this trips, we should handle errors
	check(PlatformInfo != nullptr);

	const FString UBTPlatformString = PlatformInfo->DataDrivenPlatformInfo->UBTPlatformString;
	const FString ProjectPath = GetProjectPathForTurnkey();

	// check that we can proceed
	{
		if (FInstalledPlatformInfo::Get().IsPlatformMissingRequiredFile(UBTPlatformString))
		{
			if (!FInstalledPlatformInfo::OpenInstallerOptions())
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("MissingPlatformFilesCook", "Missing required files to cook for this platform."));
			}
			return;
		}

		if (!CheckSupportedPlatforms(IniPlatformName))
		{
			return;
		}
	}

	// force a save of dirty packages before proceeding to run UAT
	// this may delete UProjectPackagingSettings , don't hold it across this call
	FEdgegapSettingsDetails::SaveAll();

	// basic BuildCookRun params we always want
	FString BuildCookRunParams = FString::Printf(TEXT("-nop4 -utf8output %s -cook "), GetUATCompilationFlags());


	// set locations to engine and project
	if (!ProjectPath.IsEmpty())
	{
		BuildCookRunParams += FString::Printf(TEXT(" -project=\"%s\""), *ProjectPath);
	}

	UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
	const FString TargetName = EdgegapSettings->OverridableTargetName;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
	if(TargetName.IsEmpty())
	{
		BuildCookRunParams += FString::Printf(TEXT(" -target=%s"), *FString::Printf(TEXT("%sServer"), FApp::GetProjectName()));
	}
	else
	{
		BuildCookRunParams += FString::Printf(TEXT(" -target=%s"), *TargetName);
	}
#else
	bool bIsProjectBuildTarget = false;
	const FTargetInfo* BuildTargetInfo = PlatformsSettings->GetBuildTargetInfoForPlatform(IniPlatformName, bIsProjectBuildTarget);
	if (BuildTargetInfo && bIsProjectBuildTarget)
	{
		if(TargetName.IsEmpty())
		{
			BuildCookRunParams += FString::Printf(TEXT(" -target=%s"), *FString::Printf(TEXT("%sServer"), FApp::GetProjectName()));
		}
		else
		{
			BuildCookRunParams += FString::Printf(TEXT(" -target=%s"), *TargetName);
		}
	}
#endif

	// set the platform we are preparing content for
	{
		BuildCookRunParams += FString::Printf(TEXT(" -platform=%s"), *UBTPlatformString);
	}

	// Append any extra UAT flags specified for this platform flavor
	if (!PlatformInfo->UATCommandLine.IsEmpty())
	{
		BuildCookRunParams += FString::Printf(TEXT(" %s"), *PlatformInfo->UATCommandLine);
	}

	// optional settings
	if (PackagingSettings->bSkipEditorContent)
	{
		BuildCookRunParams += TEXT(" -SkipCookingEditorContent");
	}
	if (FDerivedDataCacheInterface* DDC = GetDerivedDataCache())
	{
		BuildCookRunParams += FString::Printf(TEXT(" -ddc=%s"), DDC->GetGraphName());
	}
	if (FApp::IsEngineInstalled())
	{
		BuildCookRunParams += TEXT(" -installed");
	}

	// per mode settings
	FText ContentPrepDescription;
	FText ContentPrepTaskName;
	const FSlateBrush* ContentPrepIcon = nullptr;
	if (true)
	{
		ContentPrepDescription = LOCTEXT("PackagingProjectTaskName", "Packaging project");
		ContentPrepTaskName = LOCTEXT("PackagingTaskName", "Packaging");
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 3
		ContentPrepIcon = FAppStyle::GetBrush(TEXT("MainFrame.PackageProject"));
#else
		ContentPrepIcon = FEditorStyle::GetBrush(TEXT("MainFrame.PackageProject"));
#endif

		// let the user pick a target directory
		if (PlatformsSettings->StagingDirectory.Path.IsEmpty())
		{
			PlatformsSettings->StagingDirectory.Path = FPaths::ProjectDir();
		}

		FString OutFolderName;

		if (!FDesktopPlatformModule::Get()->OpenDirectoryDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr), LOCTEXT("PackageDirectoryDialogTitle", "Package project...").ToString(), PlatformsSettings->StagingDirectory.Path, OutFolderName))
		{
			return;
		}

		PlatformsSettings->StagingDirectory.Path = OutFolderName;
		PlatformsSettings->SaveConfig();
		// @TODO: Check whether SaveConfig for AllPlatformPackagingSettings is still relevant/required now
		AllPlatformPackagingSettings->SaveConfig();

		BuildCookRunParams += TEXT(" -stage -archive -package");

		const ITargetPlatform* TargetPlatform = GetTargetPlatformManager()->FindTargetPlatform(PlatformInfo->Name);
		if (ShouldBuildProject(PackagingSettings, TargetPlatform))
		{
			BuildCookRunParams += TEXT(" -build");
		}

		if (PackagingSettings->FullRebuild)
		{
			BuildCookRunParams += TEXT(" -clean");
		}

		if (PackagingSettings->bCompressed)
		{
			BuildCookRunParams += TEXT(" -compressed");
		}

		if (PackagingSettings->bUseIoStore)
		{
			BuildCookRunParams += TEXT(" -iostore");

			// Pak file(s) must be used when using container file(s)
			PackagingSettings->UsePakFile = true;
		}

		if (PackagingSettings->UsePakFile)
		{
			BuildCookRunParams += TEXT(" -pak");
		}

		if (PackagingSettings->IncludePrerequisites)
		{
			BuildCookRunParams += TEXT(" -prereqs");
		}

		if (!PackagingSettings->ApplocalPrerequisitesDirectory.Path.IsEmpty())
		{
			BuildCookRunParams += FString::Printf(TEXT(" -applocaldirectory=\"%s\""), *(PackagingSettings->ApplocalPrerequisitesDirectory.Path));
		}
		else if (PackagingSettings->IncludeAppLocalPrerequisites)
		{
			BuildCookRunParams += TEXT(" -applocaldirectory=\"$(EngineDir)/Binaries/ThirdParty/AppLocalDependencies\"");
		}

		BuildCookRunParams += FString::Printf(TEXT(" -archivedirectory=\"%s\""), *PlatformsSettings->StagingDirectory.Path);

		if (PackagingSettings->ForDistribution)
		{
			BuildCookRunParams += TEXT(" -distribution");
		}

		if (PackagingSettings->bGenerateChunks)
		{
			BuildCookRunParams += TEXT(" -manifests");
		}

		// Whether to include the crash reporter.
		if (PackagingSettings->IncludeCrashReporter && PlatformInfo->DataDrivenPlatformInfo->bCanUseCrashReporter)
		{
			BuildCookRunParams += TEXT(" -CrashReporter");
		}

		if (PackagingSettings->bBuildHttpChunkInstallData)
		{
			BuildCookRunParams += FString::Printf(TEXT(" -manifests -createchunkinstall -chunkinstalldirectory=\"%s\" -chunkinstallversion=%s"), *(PackagingSettings->HttpChunkInstallDataDirectory.Path), *(PackagingSettings->HttpChunkInstallDataVersion));
		}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
		EProjectPackagingBuildConfigurations BuildConfig = EProjectPackagingBuildConfigurations::PPBC_Development;
#else
		EProjectPackagingBuildConfigurations BuildConfig = PlatformsSettings->GetBuildConfigurationForPlatform(IniPlatformName);
#endif
		const UEdgegapSettings* EdgegapSettingsLocal = GetMutableDefault<UEdgegapSettings>();
		if(EdgegapSettingsLocal)
		{
			BuildConfig = EdgegapSettingsLocal->BuildConfiguration;
		}
		const UProjectPackagingSettings::FConfigurationInfo& ConfigurationInfo = UProjectPackagingSettings::ConfigurationInfo[(int)BuildConfig];

		BuildCookRunParams += FString::Printf(TEXT(" -server -noclient -serverconfig=%s"), LexToString(ConfigurationInfo.Configuration));

		if (ConfigurationInfo.Configuration == EBuildConfiguration::Shipping && !PackagingSettings->IncludeDebugFiles)
		{
			BuildCookRunParams += TEXT(" -nodebuginfo");
		}
	}

	FString TurnkeyParams = FString::Printf(TEXT("-command=VerifySdk -platform=%s -UpdateIfNeeded"), *UBTPlatformString);
	if (!ProjectPath.IsEmpty())
	{
		TurnkeyParams.Appendf(TEXT(" -project=\"%s\""), *ProjectPath);
	}

	FString CommandLine;
	if (!ProjectPath.IsEmpty())
	{
		CommandLine.Appendf(TEXT("-ScriptsForProject=\"%s\" "), *ProjectPath);
	}
	CommandLine.Appendf(TEXT("Turnkey %s BuildCookRun %s"), *TurnkeyParams, *BuildCookRunParams);

	IUATHelperModule::Get().CreateUatTask(CommandLine, PlatformInfo->DisplayName, ContentPrepDescription, ContentPrepTaskName, ContentPrepIcon, nullptr, &OnPackageCallaback);
}

void FEdgegapSettingsDetails::AddMessageLog(const FText& Text, const FText& Detail, const FString& TutorialLink, const FString& DocumentationLink)
{
	TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Error);
	Message->AddToken(FTextToken::Create(Text));
	Message->AddToken(FTextToken::Create(Detail));
	if (!TutorialLink.IsEmpty())
	{
		Message->AddToken(FTutorialToken::Create(TutorialLink));
	}
	if (!DocumentationLink.IsEmpty())
	{
		Message->AddToken(FDocumentationToken::Create(DocumentationLink));
	}
	FMessageLog MessageLog("PackagingResults");
	MessageLog.AddMessage(Message);
	MessageLog.Open();
}

void FEdgegapSettingsDetails::Containerize(FString DockerFilePath, FString StartScriptPath, FString ServerBuildPath, FString RegistryURL, FString ImageRepository,  FString Tag, FString PrivateUsername, FString PrivateToken)
{
	const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
	const FString LocalAppName = EdgegapSettings->ApplicationName.ToString();
	const UGeneralProjectSettings& ProjectSettings = *GetDefault<UGeneralProjectSettings>();

	_PrivateUsername = PrivateUsername;
	_PrivateToken= PrivateToken;
	_RegistryURL = RegistryURL;

	_ImageName = FEdgegapSettingsDetails::MakeImageName(RegistryURL, ImageRepository, LocalAppName, Tag);

	UProjectPackagingSettings* PackagingSettings = Cast<UProjectPackagingSettings>(UProjectPackagingSettings::StaticClass()->GetDefaultObject());
	UPlatformsMenuSettings* PlatformsSettings = GetMutableDefault<UPlatformsMenuSettings>();

	FString PathToServerBuild = FPaths::Combine(PlatformsSettings->StagingDirectory.Path, FString("LinuxServer"));

	FString DockerFileContent;
	FString NewDockerFilePath = FPaths::Combine(ServerBuildPath, FPaths::GetCleanFilename(DockerFilePath));
	IPlatformFile::GetPlatformPhysical().CopyFile(*NewDockerFilePath, *DockerFilePath);
	FFileHelper::LoadFileToString(DockerFileContent, *NewDockerFilePath);
	DockerFileContent = DockerFileContent.Replace(*FString("<PROJECT_NAME>"), FApp::GetProjectName());
	FFileHelper::SaveStringToFile(DockerFileContent, *NewDockerFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	FString StartScriptContent;
	FString NewStartScriptPath = FPaths::Combine(ServerBuildPath, FPaths::GetCleanFilename(StartScriptPath));
	IPlatformFile::GetPlatformPhysical().CopyFile(*NewStartScriptPath, *StartScriptPath);
	FFileHelper::LoadFileToString(StartScriptContent, *NewStartScriptPath);
	const FString LaunchTargetName = ResolveServerLaunchTargetName(EdgegapSettings);
	StartScriptContent = StartScriptContent.Replace(*FString("<TARGET_FILE_NAME>"), *LaunchTargetName);
	StartScriptContent = StartScriptContent.Replace(*FString("<PROJECT_NAME>"), FApp::GetProjectName());
	FFileHelper::SaveStringToFile(StartScriptContent, *NewStartScriptPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	FString CommandLine = FString::Printf(TEXT("docker build -t \"%s\" \"%s\""), *_ImageName, *ServerBuildPath);
	UE_LOG(EdgegapLog, Log, TEXT("%s"), *CommandLine);
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 3
	IUCMDHelperModule::Get().CreateUcmdTask(CommandLine, LOCTEXT("DisplayName", "Docker"), LOCTEXT("ContainerizingProjectTaskName", "Containerizing server"), LOCTEXT("ContainerizingTaskName", "Containerizing"), FAppStyle::GetBrush(TEXT("MainFrame.PackageProject")), false, &OnContainerizeCallback);
#else
	IUCMDHelperModule::Get().CreateUcmdTask(CommandLine, LOCTEXT("DisplayName", "Docker"), LOCTEXT("ContainerizingProjectTaskName", "Containerizing server"), LOCTEXT("ContainerizingTaskName", "Containerizing"), FEditorStyle::GetBrush(TEXT("MainFrame.PackageProject")), false, &OnContainerizeCallback);
#endif
}

void FEdgegapSettingsDetails::PushContainer(FString ImageName, FString RegistryURL, FString PrivateUsername, FString PrivateToken, bool LoggedIn)
{
	_ImageName = ImageName;
	_RegistryURL = RegistryURL;
	_PrivateUsername = PrivateUsername;
	_PrivateToken = PrivateToken;

	if (!LoggedIn)
	{
		DockerLogin(RegistryURL, PrivateUsername, PrivateToken);
		return;
	}

	FString CommandLine = FString::Printf(TEXT("docker image push %s"), *ImageName);
	UE_LOG(EdgegapLog, Log, TEXT("%s"), *CommandLine);
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 3
	IUCMDHelperModule::Get().CreateUcmdTask(CommandLine, LOCTEXT("DisplayName", "Docker"), LOCTEXT("PushContainerProjectTaskName", "Pushing Container"), LOCTEXT("ContainerizingTaskName", "Docker Login"), FAppStyle::GetBrush(TEXT("MainFrame.PackageProject")), false, &OnPushContainerCallback);
#else
	IUCMDHelperModule::Get().CreateUcmdTask(CommandLine, LOCTEXT("DisplayName", "Docker"), LOCTEXT("PushContainerProjectTaskName", "Pushing Container"), LOCTEXT("ContainerizingTaskName", "Docker Login"), FEditorStyle::GetBrush(TEXT("MainFrame.PackageProject")), false, &OnPushContainerCallback);
#endif
}

void FEdgegapSettingsDetails::DockerLogin(FString RegistryURL, FString PrivateUsername, FString PrivateToken)
{
	FString CommandLine = FString::Printf(TEXT("echo \'%s\' | docker login -u \'%s\' --password-stdin %s"), *PrivateToken, *PrivateUsername, *RegistryURL);
	UE_LOG(EdgegapLog, Log, TEXT("%s"), *CommandLine);
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 3
	IUCMDHelperModule::Get().CreateUcmdTask(CommandLine, LOCTEXT("DisplayName", "Docker"), LOCTEXT("DockerLoginProjectTaskName", "Logging into Registry"), LOCTEXT("ContainerizingTaskName", "Docker Login"), FAppStyle::GetBrush(TEXT("MainFrame.PackageProject")), true, &OnDockerLoginCallback);
#else
	IUCMDHelperModule::Get().CreateUcmdTask(CommandLine, LOCTEXT("DisplayName", "Docker"), LOCTEXT("DockerLoginProjectTaskName", "Logging into Registry"), LOCTEXT("ContainerizingTaskName", "Docker Login"), FEditorStyle::GetBrush(TEXT("MainFrame.PackageProject")), true, &OnDockerLoginCallback);
#endif
}

// --- Docker Build Pipeline ---

void FEdgegapSettingsDetails::DockerBuildAndPush()
{
	if (!CheckDockerRunning())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("DockerNotRunning", "Docker isn't running - please start Docker and try again!"));
		return;
	}

	const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();

	// Validate Docker Build settings
	if (EdgegapSettings->GitHubUsername.IsEmpty() || EdgegapSettings->GitHubPAT.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("MissingGitHubCredentials",
				"GitHub credentials are required for Docker Build mode.\nPlease set your GitHub Username and PAT in Build Configurations > Docker Build."));
		return;
	}

	if (EdgegapSettings->UEDockerImageTag.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("MissingImageTag", "UE Docker Image Tag is required for Docker Build mode."));
		return;
	}

	// Warn user about first-time build duration
	EAppReturnType::Type UserResponse = FMessageDialog::Open(EAppMsgType::OkCancel,
		LOCTEXT("DockerBuildWarning",
			"Docker Build will build the dedicated server inside a Docker container.\n\n"
			"The first build may take a long time (~30+ minutes) as it needs to pull the Unreal Engine Docker image (~30GB).\n"
			"Subsequent builds will be much faster due to Docker caching.\n\n"
			"Make sure you have sufficient disk space (50GB+).\n\n"
			"Press OK to continue."));

	if (UserResponse != EAppReturnType::Ok)
	{
		return;
	}

	// Prepare the tag
	_RecentTag = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M"));

	// Save all dirty packages
	SaveAll();

	// Generate Dockerfile, StartServer.sh, and .dockerignore in the project root
	FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());

	FString DockerfileContent = GenerateMultiStageDockerfile();
	FString DockerfilePath = FPaths::Combine(ProjectDir, TEXT("Dockerfile"));
	FFileHelper::SaveStringToFile(DockerfileContent, *DockerfilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	FString StartServerContent = GenerateDockerBuildStartServerScript();
	FString StartServerPath = FPaths::Combine(ProjectDir, TEXT("StartServer.sh"));
	FFileHelper::SaveStringToFile(StartServerContent, *StartServerPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	FString DockerIgnoreContent = GenerateDockerIgnoreContent();
	FString DockerIgnorePath = FPaths::Combine(ProjectDir, TEXT(".dockerignore"));
	FFileHelper::SaveStringToFile(DockerIgnoreContent, *DockerIgnorePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	UE_LOG(EdgegapLog, Log, TEXT("Generated Dockerfile, StartServer.sh, and .dockerignore in %s"), *ProjectDir);

	// Step 1: Login to ghcr.io
	DockerBuild_LoginToGHCR(EdgegapSettings->GitHubUsername, EdgegapSettings->GitHubPAT);
}

FString FEdgegapSettingsDetails::GenerateMultiStageDockerfile()
{
	const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();

	const FString ImageTag = EdgegapSettings->UEDockerImageTag;
	const FString TargetName = EdgegapSettings->OverridableTargetName.IsEmpty()
		? FString::Printf(TEXT("%sServer"), FApp::GetProjectName())
		: EdgegapSettings->OverridableTargetName;

	const FString ProjectName = FString(FApp::GetProjectName());

	// Determine server config string
	const UProjectPackagingSettings::FConfigurationInfo& ConfigInfo =
		UProjectPackagingSettings::ConfigurationInfo[(int)EdgegapSettings->BuildConfiguration];
	FString ServerConfig = LexToString(ConfigInfo.Configuration);

	FString ExtraBuildArgs = EdgegapSettings->DockerExtraBuildArgs;

	// Build the Dockerfile content
	FString Dockerfile;

	// Builder stage
	Dockerfile += FString::Printf(TEXT("ARG UE_IMAGE_TAG=%s\n"), *ImageTag);
	Dockerfile += TEXT("FROM ghcr.io/epicgames/unreal-engine:${UE_IMAGE_TAG} AS builder\n\n");
	Dockerfile += TEXT("# Copy project into the container\n");
	Dockerfile += TEXT("COPY --chown=ue4:ue4 . /tmp/project\n\n");
	Dockerfile += TEXT("# Build the dedicated server\n");
	Dockerfile += FString::Printf(
		TEXT("RUN /home/ue4/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \\\n")
		TEXT("    -serverconfig=%s \\\n")
		TEXT("    -project=/tmp/project/%s.uproject \\\n")
		TEXT("    -target=%s \\\n")
		TEXT("    -utf8output -nodebuginfo -allmaps -noP4 -cook -ddc=NoZenLocalFallback -build -stage -prereqs -pak -package -archive \\\n")
		TEXT("    -archivedirectory=/tmp/project/Packaged \\\n")
		TEXT("    -platform=Linux -server -noclient"),
		*ServerConfig, *ProjectName, *TargetName);

	if (!ExtraBuildArgs.IsEmpty())
	{
		Dockerfile += FString::Printf(TEXT(" \\\n    %s"), *ExtraBuildArgs);
	}
	Dockerfile += TEXT("\n\n");

	// Runtime stage
	Dockerfile += TEXT("# ---- Runtime stage ----\n");
	Dockerfile += TEXT("FROM ubuntu:22.04 AS runtime\n\n");
	Dockerfile += TEXT("RUN apt-get update && apt-get install -y jq curl && \\\n");
	Dockerfile += TEXT("    apt-get clean && rm -rf /var/lib/{apt,dpkg,cache,log}/\n\n");
	Dockerfile += TEXT("RUN useradd -rm -d /home/ue4 -s /bin/bash -u 1000 ue4\n");
	Dockerfile += TEXT("USER ue4\n\n");
	Dockerfile += TEXT("COPY --from=builder --chown=ue4:ue4 /tmp/project/Packaged/LinuxServer /home/ue4/project\n");
	Dockerfile += TEXT("COPY --chown=ue4:ue4 StartServer.sh /home/ue4/project/StartServer.sh\n");
	Dockerfile += TEXT("RUN sed -i 's/\\r$//' /home/ue4/project/StartServer.sh\n");
	Dockerfile += TEXT("RUN chmod +x /home/ue4/project/StartServer.sh\n\n");
	Dockerfile += TEXT("WORKDIR /home/ue4/project\n");
	Dockerfile += TEXT("CMD [\"/bin/bash\", \"/home/ue4/project/StartServer.sh\"]\n");

	return Dockerfile;
}

FString FEdgegapSettingsDetails::GenerateDockerBuildStartServerScript()
{
	const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
	const FString TargetName = ResolveServerLaunchTargetName(EdgegapSettings);

	FString Script;
	Script += TEXT("#!/bin/sh\n\n");
	Script += TEXT("GAME_PORT=$(echo $ARBITRIUM_PORTS_MAPPING | jq '.ports.gameport.internal')\n\n");
	Script += TEXT("printenv\n");
	Script += TEXT("env\n\n");
	Script += TEXT("# Function to call Edgegap self-stop API\n");
	Script += TEXT("call_stop_api() {\n");
	Script += TEXT("    local EXIT_CODE=$1\n");
	Script += TEXT("    local REQUEST_ID=\"${EG_REQUEST_ID:-${EDGEGAP_REQUEST_ID:-${EDGE_REQUEST_ID:-${ARBITRIUM_REQUEST_ID}}}}\"\n");
	Script += TEXT("    local API_KEY=\"${EG_API_KEY:-${EDGEGAP_API_KEY:-${EDGE_API_KEY}}}\"\n\n");
	Script += TEXT("    if [ -z \"$REQUEST_ID\" ]; then\n");
	Script += TEXT("        echo \"No Edgegap request ID found\"\n");
	Script += TEXT("        exit $EXIT_CODE\n");
	Script += TEXT("    fi\n\n");
	Script += TEXT("    if [ -z \"$API_KEY\" ]; then\n");
	Script += TEXT("        echo \"No Edgegap API key found\"\n");
	Script += TEXT("        exit $EXIT_CODE\n");
	Script += TEXT("    fi\n\n");
	Script += TEXT("    echo \"Calling Edgegap self-stop API for request ID: $REQUEST_ID\"\n");
	Script += TEXT("    curl -s -X DELETE \\\n");
	Script += TEXT("        -H \"Content-Type: application/json\" \\\n");
	Script += TEXT("        -H \"Authorization: $API_KEY\" \\\n");
	Script += TEXT("        \"https://api.edgegap.com/v1/stop/$REQUEST_ID\" \\\n");
	Script += TEXT("        --max-time 5\n\n");
	Script += TEXT("    exit $EXIT_CODE\n");
	Script += TEXT("}\n\n");
	Script += TEXT("trap 'call_stop_api $?' EXIT INT TERM\n\n");
	Script += FString::Printf(TEXT("$(dirname \"$0\")/%s.sh -log -PORT=$GAME_PORT\n"), *TargetName);
	Script += TEXT("SERVER_EXIT_CODE=$?\n");
	Script += TEXT("call_stop_api $SERVER_EXIT_CODE\n");

	return Script;
}

FString FEdgegapSettingsDetails::GenerateDockerIgnoreContent()
{
	FString Content;
	Content += TEXT(".git\n");
	Content += TEXT("Saved/\n");
	Content += TEXT("Intermediate/\n");
	Content += TEXT("DerivedDataCache/\n");
	Content += TEXT("Binaries/\n");
	Content += TEXT("Build/\n");
	Content += TEXT("*.sln\n");
	Content += TEXT("*.xcworkspace\n");
	Content += TEXT("*.xcodeproj\n");
	Content += TEXT("*.pdb\n");
	Content += TEXT("Plugins/*/Intermediate/\n");
	Content += TEXT("Plugins/*/Binaries/\n");
	return Content;
}

void FEdgegapSettingsDetails::DockerBuild_LoginToGHCR(FString GitHubUsername, FString GitHubPAT)
{
#if PLATFORM_WINDOWS
	FString CommandLine = FString::Printf(
		TEXT("echo %s | docker login ghcr.io -u %s --password-stdin"),
		*GitHubPAT, *GitHubUsername);
#else
	FString CommandLine = FString::Printf(
		TEXT("echo '%s' | docker login ghcr.io -u '%s' --password-stdin"),
		*GitHubPAT, *GitHubUsername);
#endif

	UE_LOG(EdgegapLog, Log, TEXT("Logging into ghcr.io as %s"), *GitHubUsername);

	IUCMDHelperModule::Get().CreateUcmdTask(
		CommandLine,
		LOCTEXT("DisplayName", "Docker"),
		LOCTEXT("GHCRLoginTaskName", "Logging into ghcr.io"),
		LOCTEXT("GHCRLoginShortName", "GHCR Login"),
		FAppStyle::GetBrush(TEXT("MainFrame.PackageProject")),
#if PLATFORM_WINDOWS
		false,
#else
		false,
#endif
		&OnDockerBuild_GHCRLoginCallback);
}

void FEdgegapSettingsDetails::DockerBuild_Build(FString ImageName, FString ProjectDir)
{
	// Detect platform for ARM Macs
	FString PlatformFlag;
#if PLATFORM_MAC
	int32 ReturnCode;
	FString ArchOutput;
	FString ErrString;
	FPlatformProcess::ExecProcess(TEXT("/usr/bin/uname"), TEXT("-m"), &ReturnCode, &ArchOutput, &ErrString);
	ArchOutput.TrimStartAndEndInline();
	if (ArchOutput.Contains(TEXT("arm64")) || ArchOutput.Contains(TEXT("aarch64")))
	{
		PlatformFlag = TEXT("--platform linux/amd64 ");
	}
#endif

	FString CommandLine = FString::Printf(
		TEXT("docker build %s-t \"%s\" \"%s\""),
		*PlatformFlag, *ImageName, *ProjectDir);

	UE_LOG(EdgegapLog, Log, TEXT("Docker Build command: %s"), *CommandLine);

	IUCMDHelperModule::Get().CreateUcmdTask(
		CommandLine,
		LOCTEXT("DisplayName", "Docker"),
		LOCTEXT("DockerBuildTaskName", "Building server in Docker (this may take a while)"),
		LOCTEXT("DockerBuildShortName", "Docker Build"),
		FAppStyle::GetBrush(TEXT("MainFrame.PackageProject")),
		false,
		&OnDockerBuild_BuildCallback);
}

void FEdgegapSettingsDetails::Request_VerifyToken()
{
	const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
	FString TokenSource;
	FString APIToken = GetDeployerToken(EdgegapSettings, &TokenSource);

	if (APIToken.IsEmpty())
	{
		UE_LOG(EdgegapLog, Warning, TEXT("VerifyToken: token is empty. Source=%s"), *TokenSource);

		FNotificationInfo Info(LOCTEXT("VerifyTokenMissing", "Deployer Key is empty. Please set it before verifying."));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return;
	}

	UE_LOG(EdgegapLog, Log, TEXT("VerifyToken: sending request. Source=%s, TokenLength=%d"), *TokenSource, APIToken.Len());

	const FString endpoint = FString::Printf(TEXT("v1/wizard/init-quick-start"));

	FString URL = FString::Printf(TEXT("%s%s"), TEXT(EDGEGAP_API_URL), *endpoint);

	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		UE_LOG(EdgegapLog, Error, TEXT("Could not get a pointer to http module!"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}

	FHttpRequestRef Request = Http->CreateRequest();

	Request->SetURL(URL);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader(TEXT("Authorization"), *APIToken);

	FString Source = "unreal";

	FString JsonString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonString);
	JsonWriter->WriteObjectStart();
	JsonWriter->WriteValue("source", Source);
	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();

	Request->SetContentAsString(JsonString);

	// Binding
	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
		{
			if (!ResponsePtr.IsValid())
			{
				OnIsTokenVerifiedChanged.ExecuteIfBound(false);
				if (UEdgegapSettings* MutableEdgegapSettings = GetMutableDefault<UEdgegapSettings>())
				{
					MutableEdgegapSettings->bIsTokenVerified = false;
					MutableEdgegapSettings->SaveConfig(CPF_Config, *MutableEdgegapSettings->GetDefaultConfigFilename());
				}

				UE_LOG(EdgegapLog, Warning, TEXT("VerifyTokenComplete callback: HTTP response invalid"));

				FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);
				return;
			}

			int32 ResponseCode = ResponsePtr->GetResponseCode();
			UE_LOG(EdgegapLog, Log, TEXT("VerifyToken: completed. Success=%s, ResponseCode=%d"), bWasSuccessful ? TEXT("true") : TEXT("false"), ResponseCode);

			if (!bWasSuccessful || ResponseCode < 200 || ResponseCode > 299)
			{
				OnIsTokenVerifiedChanged.ExecuteIfBound(false);
				if (UEdgegapSettings* MutableEdgegapSettings = GetMutableDefault<UEdgegapSettings>())
				{
					MutableEdgegapSettings->bIsTokenVerified = false;
					MutableEdgegapSettings->SaveConfig(CPF_Config, *MutableEdgegapSettings->GetDefaultConfigFilename());
				}

				FString Response = ResponsePtr->GetContentAsString();
				UE_LOG(EdgegapLog, Warning, TEXT("VerifyTokenComplete callback: HTTP request failed with code %d and response: %s"), ResponseCode, *Response);

				FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);

				return;
			}

			FString Response = ResponsePtr->GetContentAsString();

			TSharedPtr<FJsonValue> JsonValue;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

			if (ResponseCode == 204 || FJsonSerializer::Deserialize(Reader, JsonValue))
			{
				if (ResponseCode != 204 && JsonValue)
				{
					if (JsonValue->AsObject()->HasField(UTF8_TO_TCHAR("message")))
					{
						OnIsTokenVerifiedChanged.ExecuteIfBound(false);
						if (UEdgegapSettings* MutableEdgegapSettings = GetMutableDefault<UEdgegapSettings>())
						{
							MutableEdgegapSettings->bIsTokenVerified = false;
							MutableEdgegapSettings->SaveConfig(CPF_Config, *MutableEdgegapSettings->GetDefaultConfigFilename());
						}

						FString message = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("message"));
						UE_LOG(EdgegapLog, Error, TEXT("VerifyTokenComplete callback: Failed, message:%s"), *message);

						FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
						Info.ExpireDuration = 3.0f;
						FSlateNotificationManager::Get().AddNotification(Info);

						return;
					}
				}

				bool bIsVerified = true;

				OnIsTokenVerifiedChanged.ExecuteIfBound(bIsVerified);

				UEdgegapSettings* MutableEdgegapSettings = GetMutableDefault<UEdgegapSettings>();
				MutableEdgegapSettings->bIsTokenVerified = true;
				GConfig->Flush(false, GEditorPerProjectIni);
				MutableEdgegapSettings->SaveConfig(CPF_Config, *MutableEdgegapSettings->GetDefaultConfigFilename());

				FNotificationInfo Info(LOCTEXT("OperationSuccess", "Token verified successfully"));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);

				Request_RegistryCredentials();
			}
			else
			{
				OnIsTokenVerifiedChanged.ExecuteIfBound(false);
				if (UEdgegapSettings* MutableEdgegapSettings = GetMutableDefault<UEdgegapSettings>())
				{
					MutableEdgegapSettings->bIsTokenVerified = false;
					MutableEdgegapSettings->SaveConfig(CPF_Config, *MutableEdgegapSettings->GetDefaultConfigFilename());
				}

				UE_LOG(EdgegapLog, Error, TEXT("VerifyTokenComplete callback: Could not deserialize response into Json, Response:%s"), *Response);

				FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);

				return;
			}
		});

	if (!Request->ProcessRequest())
	{
		UE_LOG(EdgegapLog, Error, TEXT("VerifyToken: Could not process HTTP request"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}
}

void FEdgegapSettingsDetails::Request_CreateApplication(TSharedPtr<SButton> InCreateApplication_SBtn)
{
	FHttpModule* Http = &FHttpModule::Get();

	if (!Http)
	{
		if (InCreateApplication_SBtn)
		{
			InCreateApplication_SBtn->SetEnabled(true);
		}

		UE_LOG(EdgegapLog, Error, TEXT("Could not get a pointer to http module!"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}

	const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
	FString AppName = EdgegapSettings->ApplicationName.ToString();
	FString APIToken = GetDeployerToken(EdgegapSettings);
	FString ImagePath = EdgegapSettings->ImagePath.FilePath;

	const FString endpoint = "v1/app";
	FString URL = FString::Printf(TEXT("%s%s"), TEXT(EDGEGAP_API_URL), *endpoint);

	FHttpRequestRef Request = Http->CreateRequest();

	if (!FPaths::FileExists(ImagePath))
	{
		if (InCreateApplication_SBtn)
		{
			InCreateApplication_SBtn->SetEnabled(true);
		}

		// check if the file is relative to the save/BouncedWavFiles directory
		UE_LOG(EdgegapLog, Error, TEXT("CreateApp: File does not exist!, %s"), *ImagePath);

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}

	// Read the file into a byte array
	TArray<uint8> Payload;
	FFileHelper::LoadFileToArray(Payload, *ImagePath, 0);
	FString EncodedImage = FBase64::Encode(Payload);

	// Set request fields
	Request->SetURL(URL);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader(TEXT("Authorization"), *APIToken);

	FString JsonString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonString);
	JsonWriter->WriteObjectStart();
	JsonWriter->WriteValue("name", AppName);
	JsonWriter->WriteValue("image", EncodedImage);
	JsonWriter->WriteValue("is_active", true);
	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();

	Request->SetContentAsString(JsonString);

	Request->OnProcessRequestComplete().BindLambda([this, InCreateApplication_SBtn](FHttpRequestPtr Request, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
	{
		if (InCreateApplication_SBtn)
		{
			InCreateApplication_SBtn->SetEnabled(true);
		}

		if (!bWasSuccessful || ResponsePtr->GetResponseCode() < 200 || ResponsePtr->GetResponseCode() > 299)
		{
			FString Response = ResponsePtr->GetContentAsString();
			UE_LOG(EdgegapLog, Warning, TEXT("onCreateAppComplete: HTTP request failed with code %d and response: %s"), ResponsePtr->GetResponseCode(), *Response);
			return;
		}

		FString Response = ResponsePtr->GetContentAsString();

		TSharedPtr<FJsonValue> JsonValue;

		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

		if (FJsonSerializer::Deserialize(Reader, JsonValue))
		{
			if (JsonValue->AsObject()->HasField(UTF8_TO_TCHAR("message")))
			{
				FString message = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("message"));
				UE_LOG(EdgegapLog, Error, TEXT("onCreateAppComplete: Failed, message:%s"), *message);

				FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);

				return;
			}

			FNotificationInfo Info(LOCTEXT("OperationSuccess", "Application created successfully"));
			Info.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		else
		{
			UE_LOG(EdgegapLog, Error, TEXT("onCreateAppComplete: Could not deserialize response into Json, Response:%s"), *Response);

			FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
			Info.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(Info);

			return;
		}
	});

	if (!Request->ProcessRequest())
	{
		UE_LOG(EdgegapLog, Error, TEXT("CreateApp: Could not process HTTP request"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}
}

void FEdgegapSettingsDetails::Request_RegistryCredentials()
{
	FHttpModule* Http = &FHttpModule::Get();

	if (!Http)
	{
		UE_LOG(EdgegapLog, Error, TEXT("Could not get a pointer to http module!"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}

	const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
	FString AppName = EdgegapSettings->ApplicationName.ToString();
	FString APIToken = GetDeployerToken(EdgegapSettings);

	const FString endpoint = "v1/wizard/registry-credentials";
	FString URL = FString::Printf(TEXT("%s%s"), TEXT(EDGEGAP_API_URL), *endpoint);
	URL += "?source=unreal";

	FHttpRequestRef Request = Http->CreateRequest();

	Request->SetURL(URL);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader(TEXT("Authorization"), APIToken);

	// Binding
	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
	{
		UEdgegapSettings* MutableEdgegapSettings = GetMutableDefault<UEdgegapSettings>();

		if (!bWasSuccessful || ResponsePtr->GetResponseCode() < 200 || ResponsePtr->GetResponseCode() > 299)
		{
			FString Response = ResponsePtr->GetContentAsString();
			UE_LOG(EdgegapLog, Warning, TEXT("Callback_RegistryCredentials: HTTP request failed with code %d and response: %s"), ResponsePtr->GetResponseCode(), *Response);
			return;
		}

		FString Response = ResponsePtr->GetContentAsString();

		TSharedPtr<FJsonValue> JsonValue;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

		if (FJsonSerializer::Deserialize(Reader, JsonValue))
		{
			if (JsonValue->AsObject()->HasField(UTF8_TO_TCHAR("message")))
			{
				FString message = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("message"));
				UE_LOG(EdgegapLog, Error, TEXT("Callback_RegistryCredentials: Failed, message:%s"), *message);

				FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);

				return;
			}

			const FString RegistryUrl = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("registry_url"));
			const FString Project = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("project"));
			const FString Username = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("username"));
			const FString Token = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("token"));

			MutableEdgegapSettings->Registry = RegistryUrl;
			MutableEdgegapSettings->ImageRepository = Project;
			MutableEdgegapSettings->PrivateRegistryUsername = Username;
			MutableEdgegapSettings->PrivateRegistryToken = Token;

			MutableEdgegapSettings->SaveConfig();
#if WITH_EDITOR
			UEdgegapSettings::OnSettingsChange.Broadcast(MutableEdgegapSettings);
#endif
		}
		else
		{
			UE_LOG(EdgegapLog, Error, TEXT("Callback_RegistryCredentials: Could not deserialize response into Json, Response:%s"), *Response);

			FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
			Info.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(Info);

			return;
		}
	});

	if (!Request->ProcessRequest())
	{
		UE_LOG(EdgegapLog, Error, TEXT("Callback_RegistryCredentials: Could not process HTTP request"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}
}

void FEdgegapSettingsDetails::CreateVersion(FString AppName, FString VersionName, FString API_key, FString RegistryURL, FString ImageRepository, FString Tag, FString PrivateUsername, FString PrivateToken)
{
	_API_key = API_key;
	_AppName = AppName;
	_VersionName = VersionName;

	const FString ComposedRepository = FString::Printf(TEXT("%s/%s"), *ImageRepository, *AppName.ToLower());

	const FString endpoint = FString::Printf(TEXT("v1/app/%s/version"), *AppName);

	FString URL = FString::Printf(TEXT("%s%s"), TEXT(EDGEGAP_API_URL), *endpoint);

	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		UE_LOG(EdgegapLog, Error, TEXT("Could not get a pointer to http module!"));

		FNotificationInfo* Info = new FNotificationInfo(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info->ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().QueueNotification(Info);

		return;
	}

	FHttpRequestRef Request = Http->CreateRequest();
	
	Request->OnProcessRequestComplete().BindStatic(&FEdgegapSettingsDetails::onCreateVersionComplete);

	// Set request fields
	Request->SetURL(URL);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader(TEXT("Authorization"), *API_key);
	UEdgegapSettings* EGSettings = GetMutableDefault<UEdgegapSettings>();
	// prepare json data
	FString JsonString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonString);
	JsonWriter->WriteObjectStart();
	JsonWriter->WriteValue("name", VersionName);
	JsonWriter->WriteValue("docker_repository", RegistryURL);
	JsonWriter->WriteValue("docker_image", ComposedRepository);
	JsonWriter->WriteValue("docker_tag", Tag);
	JsonWriter->WriteValue("private_username", PrivateUsername);
	JsonWriter->WriteValue("private_token", PrivateToken);
	if(EGSettings)
	{
		JsonWriter->WriteValue("req_cpu", EGSettings->RequiredCPU);
		JsonWriter->WriteValue("req_memory", EGSettings->RequiredMemory);
		JsonWriter->WriteValue("req_video", EGSettings->RequiredGPU);
		JsonWriter->WriteValue("max_duration", EGSettings->MaxDuration);
		JsonWriter->WriteValue("time_to_deploy", EGSettings->EstimatedDeployTime);
	}
	else
	{
		JsonWriter->WriteValue("req_cpu", 128);
		JsonWriter->WriteValue("req_memory", 256);
		JsonWriter->WriteValue("req_video", 0);
		JsonWriter->WriteValue("max_duration", 60);
		JsonWriter->WriteValue("time_to_deploy", 120);
	}
	JsonWriter->WriteValue("use_telemetry", false);
	JsonWriter->WriteValue("inject_context_env", true);
	JsonWriter->WriteValue("force_cache", false);
	JsonWriter->WriteValue("whitelisting_active", false);

	// Add default environment variables
	if (EGSettings && EGSettings->DefaultEnvironmentVariables.Num() > 0)
	{
		JsonWriter->WriteArrayStart(TEXT("env_vars"));
		for (const FEdgegapEnvironmentVariable& EnvVar : EGSettings->DefaultEnvironmentVariables)
		{
			JsonWriter->WriteObjectStart();
			JsonWriter->WriteValue("key", EnvVar.Key);
			JsonWriter->WriteValue("value", EnvVar.Value);
			JsonWriter->WriteValue("is_hidden", EnvVar.bIsHidden);
			JsonWriter->WriteObjectEnd();
		}
		JsonWriter->WriteArrayEnd();
	}
	if(EGSettings)
	{
		if(!EGSettings->EntrypointOverride.IsEmpty())
		{
			JsonWriter->WriteValue("command", EGSettings->EntrypointOverride);
		}
		if(!EGSettings->CommandArguments.IsEmpty())
		{
			JsonWriter->WriteValue("arguments", EGSettings->CommandArguments);
		}
	}

	JsonWriter->WriteArrayStart(TEXT("ports"));

	// Add additional ports from settings
	if (EGSettings && EGSettings->AdditionalPorts.Num() > 0)
	{
		for (const FEdgegapPortConfig& PortConfig : EGSettings->AdditionalPorts)
		{
			JsonWriter->WriteObjectStart();
			JsonWriter->WriteValue("port", PortConfig.Port);
			JsonWriter->WriteValue("protocol", PortConfig.Protocol);
			JsonWriter->WriteValue("to_check", PortConfig.bToCheck);
			JsonWriter->WriteValue("tls_upgrade", PortConfig.bTLSUpgrade);
			JsonWriter->WriteValue("name", PortConfig.Name);
			JsonWriter->WriteObjectEnd();
		}
	}
	else
	{
		// Add default gameport
		JsonWriter->WriteObjectStart();
		JsonWriter->WriteValue("port", 7777);
		JsonWriter->WriteValue("protocol", TEXT("TCP/UDP"));
		JsonWriter->WriteValue("to_check", false);
		JsonWriter->WriteValue("tls_upgrade", false);
		JsonWriter->WriteValue("name", TEXT("gameport"));
		JsonWriter->WriteObjectEnd();
	}
	
	JsonWriter->WriteArrayEnd();

	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();
	// Insert the content into the request
	Request->SetContentAsString(JsonString);

	if (!Request->ProcessRequest())
	{
		UE_LOG(EdgegapLog, Error, TEXT("CreateVersion: Could not process HTTP request"));

		FNotificationInfo* Info = new FNotificationInfo(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info->ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().QueueNotification(Info);
		return;
	}
}

void FEdgegapSettingsDetails::onCreateVersionComplete(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
{
	if (!bWasSuccessful || ResponsePtr->GetResponseCode() < 200 || ResponsePtr->GetResponseCode() > 299)
	{
		FString Response = ResponsePtr->GetContentAsString();
		UE_LOG(EdgegapLog, Warning, TEXT("onCreateVersionComplete: HTTP request failed with code %d and response: %s"), ResponsePtr->GetResponseCode(), *Response);
		return;
	}

	FString Response = ResponsePtr->GetContentAsString();

	TSharedPtr<FJsonValue> JsonValue;
	// Create a reader pointer to read the json data
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

	if (FJsonSerializer::Deserialize(Reader, JsonValue))
	{
		if (JsonValue->AsObject()->HasField(UTF8_TO_TCHAR("message")))
		{
			FString message = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("message"));
			UE_LOG(EdgegapLog, Error, TEXT("onCreateVersionComplete: Failed, message:%s"), *message);

			FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
			Info.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(Info);

			return;
		}

		FNotificationInfo Info(LOCTEXT("OperationSuccess", "Version created successfully"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	else
	{
		UE_LOG(EdgegapLog, Error, TEXT("onCreateVersionComplete: Could not deserialize response into Json, Response:%s"), *Response);

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}
}

void FEdgegapSettingsDetails::Request_DeployApp(FString AppName, FString VersionName, FString API_key, TSharedPtr<SButton> InCreateNewDeployment_SBtn)
{
	const FString IpifyIPsURL = "https://api.ipify.org/";

	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		if (InCreateNewDeployment_SBtn)
		{
			InCreateNewDeployment_SBtn->SetEnabled(true);
		}

		UE_LOG(EdgegapLog, Error, TEXT("Could not get a pointer to http module!"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}

	FHttpRequestRef IpifyRequest = Http->CreateRequest();

	// Set request fields
	IpifyRequest->SetURL(IpifyIPsURL);
	IpifyRequest->SetVerb("GET");
	IpifyRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

	IpifyRequest->SetHeader("Content-Type", "application/json");

	IpifyRequest->OnProcessRequestComplete().BindLambda([this, AppName, VersionName, InCreateNewDeployment_SBtn, API_key](FHttpRequestPtr RequestRes, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
		{
			if (InCreateNewDeployment_SBtn)
			{
				InCreateNewDeployment_SBtn->SetEnabled(true);
			}

			if (!bWasSuccessful || ResponsePtr->GetResponseCode() < 200 || ResponsePtr->GetResponseCode() > 299)
			{
				FString Response = ResponsePtr->GetContentAsString();
				UE_LOG(EdgegapLog, Warning, TEXT("onIpifyRequest: HTTP request failed with code %d and response: %s"), ResponsePtr->GetResponseCode(), *Response);

				FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);

				return;
			}

			FString Response = ResponsePtr->GetContentAsString();

			TSharedPtr<FJsonValue> JsonValue;
			// Create a reader pointer to read the json data
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

			if (FJsonSerializer::Deserialize(Reader, JsonValue))
			{
				if (JsonValue->AsObject()->HasField(UTF8_TO_TCHAR("message")))
				{
					FString message = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("message"));
					UE_LOG(EdgegapLog, Error, TEXT("onIpifyRequest: Failed, message:%s"), *message);

					FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
					Info.ExpireDuration = 3.0f;
					FSlateNotificationManager::Get().AddNotification(Info);

					return;
				}
			}
			if (!Response.IsEmpty())
			{
				// Deployment Request

				const FString endpoint = FString::Printf(TEXT("v1/deploy"));

				FString URL = FString::Printf(TEXT("%s%s"), TEXT(EDGEGAP_API_URL), *endpoint);

				FHttpModule* Http = &FHttpModule::Get();
				if (!Http)
				{
					if (InCreateNewDeployment_SBtn)
					{
						InCreateNewDeployment_SBtn->SetEnabled(true);
					}

					UE_LOG(EdgegapLog, Error, TEXT("Could not get a pointer to http module!"));

					FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
					Info.ExpireDuration = 3.0f;
					FSlateNotificationManager::Get().AddNotification(Info);

					return;
				}

				FHttpRequestRef Request = Http->CreateRequest();

				// Set request fields
				Request->SetURL(URL);
				Request->SetVerb("POST");
				Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

				Request->SetHeader("Content-Type", "application/json");
				Request->SetHeader(TEXT("Authorization"), *API_key);

				// prepare json data
				FString JsonString;
				TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&JsonString);
				JsonWriter->WriteObjectStart();
				JsonWriter->WriteValue(TEXT("app_name"), AppName);
				JsonWriter->WriteValue(TEXT("version_name"), VersionName);

				JsonWriter->WriteRawJSONValue(TEXT("ip_list"), "[\
					\""+ Response + "\"\
				]");

				JsonWriter->WriteObjectEnd();
				JsonWriter->Close();

				// Insert the content into the request
				Request->SetContentAsString(JsonString);

				Request->OnProcessRequestComplete().BindLambda([this, InCreateNewDeployment_SBtn, API_key](FHttpRequestPtr Request, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
					{
						if (InCreateNewDeployment_SBtn)
						{
							InCreateNewDeployment_SBtn->SetEnabled(true);
						}

						if (!bWasSuccessful || ResponsePtr->GetResponseCode() < 200 || ResponsePtr->GetResponseCode() > 299)
						{
							FString Response = ResponsePtr->GetContentAsString();
							UE_LOG(EdgegapLog, Warning, TEXT("onDeployApp: HTTP request failed with code %d and response: %s"), ResponsePtr->GetResponseCode(), *Response);

							FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
							Info.ExpireDuration = 3.0f;
							FSlateNotificationManager::Get().AddNotification(Info);

							return;
						}

						FString Response = ResponsePtr->GetContentAsString();

						TSharedPtr<FJsonValue> JsonValue;
						// Create a reader pointer to read the json data
						TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

						if (FJsonSerializer::Deserialize(Reader, JsonValue))
						{
							if (JsonValue->AsObject()->HasField(UTF8_TO_TCHAR("message")))
							{
								FString message = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("message"));
								UE_LOG(EdgegapLog, Error, TEXT("onDeployApp: Failed, message:%s"), *message);

								FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
								Info.ExpireDuration = 3.0f;
								FSlateNotificationManager::Get().AddNotification(Info);

								return;
							}
						}
						else
						{
							UE_LOG(EdgegapLog, Error, TEXT("onDeployApp: Could not deserialize response into Json, Response:%s"), *Response);

							FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
							Info.ExpireDuration = 3.0f;
							FSlateNotificationManager::Get().AddNotification(Info);

							return;
						}

						Request_GetDeploymentsInfo(API_key, nullptr);
					});

				if (!Request->ProcessRequest())
				{
					UE_LOG(EdgegapLog, Error, TEXT("onDeployApp: Could not process HTTP request"));

					FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
					Info.ExpireDuration = 3.0f;
					FSlateNotificationManager::Get().AddNotification(Info);

					return;
				}
			}
			else
			{
				UE_LOG(EdgegapLog, Error, TEXT("onIpifyRequest: Could not deserialize response into Json, Response:%s"), *Response);

				FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
				Info.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Info);

				return;
			}

			Request_GetDeploymentsInfo(API_key, nullptr);
		});

	if (!IpifyRequest->ProcessRequest())
	{
		UE_LOG(EdgegapLog, Error, TEXT("onDeployApp: Could not process HTTP request"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}
}

void FEdgegapSettingsDetails::Request_GetDeploymentsInfo(FString API_key, TSharedPtr<SButton> InRefreshBtn)
{
	const FString endpoint = FString::Printf(TEXT("v1/deployments"));

	FString URL = FString::Printf(TEXT("%s%s"), TEXT(EDGEGAP_API_URL), *endpoint);

	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		if (InRefreshBtn)
		{
			InRefreshBtn->SetEnabled(true);
		}

		UE_LOG(EdgegapLog, Error, TEXT("Could not get a pointer to http module!"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}

	FHttpRequestRef Request = Http->CreateRequest();

	// Set request fields
	Request->SetURL(URL);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader(TEXT("Authorization"), *API_key);

	Request->OnProcessRequestComplete().BindLambda([InRefreshBtn, API_key](FHttpRequestPtr Request, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
	{
		if (InRefreshBtn)
		{
			InRefreshBtn->SetEnabled(true);
		}

		FEdgegapSettingsDetails::GetInstance()->DeployStatusOverrideListSource.Empty();

		if (!bWasSuccessful || ResponsePtr->GetResponseCode() < 200 || ResponsePtr->GetResponseCode() > 299)
		{
			FString Response = ResponsePtr->GetContentAsString();
			UE_LOG(EdgegapLog, Warning, TEXT("Callback_GetDeploymentsInfo: HTTP request failed with code %d and response: %s"), ResponsePtr->GetResponseCode(), *Response);
			return;
		}

		FString Response = ResponsePtr->GetContentAsString();

		TSharedPtr<FJsonValue> JsonValue;
		// Create a reader pointer to read the json data
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);

		if (!FJsonSerializer::Deserialize(Reader, JsonValue))
		{
			UE_LOG(EdgegapLog, Error, TEXT("Callback_GetDeploymentsInfo: Could not deserialize response into Json, Response:%s"), *Response);

			FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
			Info.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(Info);

			return;
		}

		if (!JsonValue->AsObject()->HasField(UTF8_TO_TCHAR("data")))
		{
			FString message = JsonValue->AsObject()->GetStringField(UTF8_TO_TCHAR("message"));
			UE_LOG(EdgegapLog, Error, TEXT("Callback_GetDeploymentsInfo: Failed, message:%s"), *message);

			FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
			Info.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(Info);

			return;
		}

		for (auto deployment : JsonValue->AsObject()->GetArrayField(UTF8_TO_TCHAR("data")))
		{
			FString link;

			if (auto ports_obj = deployment->AsObject()->GetObjectField(UTF8_TO_TCHAR("ports")))
			{
				if (auto obj_field_gameport = ports_obj->GetObjectField(UTF8_TO_TCHAR("gameport")))
				{
					link = ports_obj->GetObjectField(UTF8_TO_TCHAR("gameport"))->GetStringField(UTF8_TO_TCHAR("link"));
				}
			}

			if (link.IsEmpty())
			{
				link = "Empty";
			}

			FString RequestID = deployment->AsObject()->GetStringField(UTF8_TO_TCHAR("request_id"));
			FString Status = deployment->AsObject()->GetStringField(UTF8_TO_TCHAR("status"));
			bool bReady = deployment->AsObject()->GetBoolField(UTF8_TO_TCHAR("ready"));
			FEdgegapSettingsDetails* ESD = FEdgegapSettingsDetails::GetInstance();
			ESD->DeployStatusOverrideListSource.Add(MakeShareable(new FDeploymentStatusListItem(link, Status, RequestID, API_key, bReady)));

			ESD->DeploymentStatusListItemListView->RebuildList();
			ESD->DeploymentStatusListItemListView->RequestListRefresh();
		}

		if (auto ESD = FEdgegapSettingsDetails::GetInstance())
		{
			auto listView = ESD->DeploymentStatusListItemListView;

			if (listView)
			{
				listView->SetItemsSource(&ESD->DeployStatusOverrideListSource);
			}
		}
	});

	if (!Request->ProcessRequest())
	{
		UE_LOG(EdgegapLog, Error, TEXT("Callback_GetDeploymentsInfo: Could not process HTTP request"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}
}

void FEdgegapSettingsDetails::Callback_GetDeploymentsInfo(FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
{

}

void FEdgegapSettingsDetails::Request_StopDeploy(FString RequestID, FString API_key)
{
	const FString endpoint = FString::Printf(TEXT("v1/stop/%s"), *RequestID);

	FString URL = FString::Printf(TEXT("%s%s"), TEXT(EDGEGAP_API_URL), *endpoint);

	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		UE_LOG(EdgegapLog, Error, TEXT("Could not get a pointer to http module!"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}

	FHttpRequestRef Request = Http->CreateRequest();

	Request->SetURL(URL);
	Request->SetVerb("DELETE");
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Authorization"), *API_key);

	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr ResponsePtr, bool bWasSuccessful)
	{
		if (!bWasSuccessful || ResponsePtr->GetResponseCode() < 200 || ResponsePtr->GetResponseCode() > 299)
		{
			FString Response = ResponsePtr->GetContentAsString();
			UE_LOG(EdgegapLog, Warning, TEXT("Callback_StopDeploy: HTTP request failed with code %d and response: %s"), ResponsePtr->GetResponseCode(), *Response);
			return;
		}

		const UEdgegapSettings* EdgegapSettings = GetMutableDefault<UEdgegapSettings>();
		const FString _APIToken = GetDeployerToken(EdgegapSettings);

		Request_GetDeploymentsInfo(_APIToken, nullptr);
	});

	if (!Request->ProcessRequest())
	{
		UE_LOG(EdgegapLog, Error, TEXT("Callback_StopDeploy: Could not process HTTP request"));

		FNotificationInfo Info(LOCTEXT("OperationFailed", "Operation failed. See logs for more information"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		return;
	}
}

TSharedRef<ITableRow> FEdgegapSettingsDetails::HandleGenerateDeployStatusWidget(TSharedPtr<FDeploymentStatusListItem> InItem, const TSharedRef<STableViewBase>& InOwnerTable)
{
	UE_LOG(EdgegapLog, Log, TEXT("HandleGenerateDeployStatusWidget %s : %s : %s : %d"), *InItem->DeploymentIP, *InItem->DeploymentStatus, *InItem->RequestID, InItem->DeploymentReady);
	return SNew(SDeployStatusListItem, InOwnerTable).Item(InItem);
}


#undef LOCTEXT_NAMESPACE
