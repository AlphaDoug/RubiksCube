// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Customizations/DASActionPointWithStateCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "DynamicAISystem/Public/Objects/DASCondition.h"



#define LOCTEXT_NAMESPACE "ActionPointWithStateCustomization"


void FDASActionPointWithStateCustomization::CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils )
{
	TSharedPtr<IPropertyHandle> StatePropertyHandle = PropertyHandle->GetChildHandle( "State" );
	TSharedPtr<IPropertyHandle> ActionPointPropertyHandle = PropertyHandle->GetChildHandle( "ActionPoint" );

	HeaderRow.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		];

	HeaderRow.ValueContent()
	.MinDesiredWidth( 500.f )
	.HAlign(HAlign_Left)
		[
			SNew( SHorizontalBox )

			+SHorizontalBox::Slot()
			.FillWidth( 0.7f )
			[
				SNew( SProperty, ActionPointPropertyHandle )
				.ShouldDisplayName( false )
			]

			+SHorizontalBox::Slot()
			.FillWidth( 0.3f )
			[
				SNew( SProperty, StatePropertyHandle )
				.ShouldDisplayName( false )
			]
		];
}

void FDASActionPointWithStateCustomization::CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils )
{

}

#undef LOCTEXT_NAMESPACE