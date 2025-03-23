// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#include "DynamicAISystemEditor.h"
#include "Customizations/DASConditionCustomization.h"
#include "Customizations/DASConditionQueryCustomization.h"
#include "Customizations/DASActionPointWithStateCustomization.h"


#define LOCTEXT_NAMESPACE "FDynamicAISystemModule"

void FDynamicAISystemEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>( "PropertyEditor" );

	PropertyModule.RegisterCustomPropertyTypeLayout( "DASConditionWrapper", FOnGetPropertyTypeCustomizationInstance::CreateStatic( &FDASConditionCustomization::MakeInstance ) );
	PropertyModule.RegisterCustomPropertyTypeLayout( "DASConditionQueryWrapper", FOnGetPropertyTypeCustomizationInstance::CreateStatic( &FDASConditionQueryCustomization::MakeInstance ) );
	PropertyModule.RegisterCustomPropertyTypeLayout( "DASActionPointWithState", FOnGetPropertyTypeCustomizationInstance::CreateStatic( &FDASActionPointWithStateCustomization::MakeInstance ) );

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FDynamicAISystemEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if( FModuleManager::Get().IsModuleLoaded( "PropertyEditor" ) )
	{
		auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>( "PropertyEditor" );

		PropertyModule.UnregisterCustomPropertyTypeLayout( "DASConditionWrapper" );
		PropertyModule.UnregisterCustomPropertyTypeLayout( "DASConditionQueryWrapper" );
		PropertyModule.UnregisterCustomPropertyTypeLayout( "DASActionPointWithState" );
	}
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDynamicAISystemEditorModule, DynamicAISystemEditor)