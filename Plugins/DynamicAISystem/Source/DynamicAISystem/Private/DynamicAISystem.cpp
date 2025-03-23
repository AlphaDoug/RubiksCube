// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#include "DynamicAISystem.h"

DEFINE_LOG_CATEGORY( LogDAS );

#define LOCTEXT_NAMESPACE "FDynamicAISystemModule"

void FDynamicAISystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FDynamicAISystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDynamicAISystemModule, DynamicAISystem)