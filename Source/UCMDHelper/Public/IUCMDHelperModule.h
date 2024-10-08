// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

struct FSlateBrush;

/**
 * The public interface to this module
 */
class IUCMDHelperModule : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IUCMDHelperModule& Get()
	{
		return FModuleManager::LoadModuleChecked< IUCMDHelperModule >( "UCMDHelper" );
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "UCMDHelper" );
	}

	/** Used to callback into calling code when a UAT task completes. First param is the result type, second param is the runtime in sec. */
	typedef TFunction<void(FString, double)> UcmdTaskResultCallack;

	/** Creates and starts up a UAT Task
	  * @param	ResultLocation	The folder where the result of the task will be stored  
	  */
	virtual void CreateUcmdTask(const FString& CommandLine, const FText& PlatformDisplayName, const FText& TaskName, const FText& TaskShortName, const FSlateBrush* TaskIcon, bool PowerShell, UcmdTaskResultCallack ResultCallback = UcmdTaskResultCallack(), const FString& ResultLocation = FString()) = 0;
};

