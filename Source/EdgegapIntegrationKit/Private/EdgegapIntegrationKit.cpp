// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EdgegapIntegrationKit.h"

#define LOCTEXT_NAMESPACE "FEdgegapIntegrationKitModule"

void FEdgegapIntegrationKitModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FEdgegapIntegrationKitModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEdgegapIntegrationKitModule, EdgegapIntegrationKit)