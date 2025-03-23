// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Customizations/DASConditionCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "DynamicAISystem/Public/Objects/DASCondition.h"



#define LOCTEXT_NAMESPACE "ConditionCustomization"


void FDASConditionCustomization::CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils )
{
	DescriptionPropertyHandle = PropertyHandle->GetChildHandle( "Description" );
	ConditionInstancePropertyHandle = PropertyHandle->GetChildHandle( "Instance" );

	TSharedPtr<IPropertyHandle> OperatorPropHandle = PropertyHandle->GetChildHandle( "Operator" );
	TSharedPtr<IPropertyHandle> FirstConditionPropHandle = PropertyHandle->GetChildHandle( "bIsFirstCondition" );

	bool bIsFirstCondition = true;
	if( FirstConditionPropHandle.IsValid() )
	{
		FirstConditionPropHandle->GetValue( bIsFirstCondition );
	}

	// FIRST CONDITION HEADER ( WITHOUT AND/OR OPERATOR )
	if( bIsFirstCondition )
	{
		HeaderRow.NameContent()
			[
				SNew( SBorder )
				.BorderBackgroundColor( FLinearColor::Black )
				.Padding( 10.f )
				.BorderImage( FCoreStyle::Get().GetBrush( "ToolPanel.GroupBorder" ) )
				[
					SNew( SHorizontalBox )

					+SHorizontalBox::Slot()
					[
						PropertyHandle->CreatePropertyNameWidget()
					]
				]
			];
	}
	// NOT FIRST CONDITION ( INCLUDES AND/OR OPERATOR )
	else
	{
		HeaderRow.NameContent()
			.MaxDesiredWidth( 600.f )
			.MinDesiredWidth( 600.f )
			[
				SNew( SBorder )
				.BorderBackgroundColor( FLinearColor::Black )
				.Padding( 10.f )
				.BorderImage( FCoreStyle::Get().GetBrush( "ToolPanel.GroupBorder" ) )
				[
					SNew( SHorizontalBox )

					// Array index text
					+SHorizontalBox::Slot()
					[
						PropertyHandle->CreatePropertyNameWidget()
					]

					// operator sign button OR/AND
					+ SHorizontalBox::Slot()
					.FillWidth( 1.f )
					.AutoWidth()
					.Padding( 10.f, 0.f, 0.f, 0.f )
					[
						SNew( SHorizontalBox )
						+ SHorizontalBox::Slot()
						.HAlign( HAlign_Right )
						[
							SNew( SProperty, OperatorPropHandle )
							.ShouldDisplayName( false )
						]
					]
				]
			];
	}
	

	HeaderRow.ValueContent()
		.MaxDesiredWidth( 600.f )
		.MinDesiredWidth( 600.f )
		[
			SNew( SBorder )
				.BorderBackgroundColor( FLinearColor::Black )
				.Padding( 10.f )
				.BorderImage( FCoreStyle::Get().GetBrush( "ToolPanel.GroupBorder" ) )
				[
					SNew( SMultiLineEditableTextBox)
					.AutoWrapText( true )
					.Text( this, &FDASConditionCustomization::GetConditionDescText )
					.HintText( this, &FDASConditionCustomization::GetConditionHintQueryDescText )
					.OnTextChanged( this, &FDASConditionCustomization::OnTextChanged )
					.OnTextCommitted( this, &FDASConditionCustomization::OnTextCommitted )
				]
		];

}

void FDASConditionCustomization::CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils )
{
	uint32 childNum;
	if( PropertyHandle->GetNumChildren( childNum ) == FPropertyAccess::Success )
	{
		for( uint32 childIndex = 0; childIndex < childNum; ++childIndex )
		{
			TSharedPtr<IPropertyHandle> childProperty = PropertyHandle->GetChildHandle( childIndex );
			FName propName = childProperty->GetProperty()->GetFName();

			if( propName != TEXT( "Operator" ) 
				&& propName != TEXT( "bIsFirstCondition" )
				&& propName != TEXT( "Description" ) 
				)
			{
				IDetailPropertyRow& row = ChildBuilder.AddProperty( childProperty.ToSharedRef() );
				row.CustomWidget( true )
					[
						SNew( SHorizontalBox )
						+SHorizontalBox::Slot()
						.MaxWidth( 1150.f )
						[
							SNew( SBorder )
							.ColorAndOpacity( FColor::Orange )
							.BorderImage( FCoreStyle::Get().GetBrush( "ToolPanel.GroupBorder" ) )
							.Padding( 5.f )
							[
								SNew( SProperty, childProperty )
								.ShouldDisplayName( false )
							]
						]
					];
			}
		}
	}
}

FText FDASConditionCustomization::GetConditionDescText() const
{
	if( DescriptionPropertyHandle.IsValid() )
	{
		FString desc = FString();
		DescriptionPropertyHandle.Get()->GetValue(desc);
		return FText::FromString( desc );
	}

	return FText();
}

FText FDASConditionCustomization::GetConditionHintQueryDescText() const
{
	if( ConditionInstancePropertyHandle.IsValid() )
	{
		UObject* ConditionInstance = nullptr;
		ConditionInstancePropertyHandle.Get()->GetValue( ConditionInstance );

		if( ConditionInstance )
		{
			if( UDASCondition* Condition = Cast<UDASCondition>( ConditionInstance ) )
			{
				return FText::FromString( Condition->GetConditionDescription() );
			}
		}
	}

	return FText();
}

void FDASConditionCustomization::OnTextChanged( const FText& InText )
{
}

void FDASConditionCustomization::OnTextCommitted( const FText& InText, const ETextCommit::Type InTextAction )
{
	SaveDataToProperty( InText );
}

void FDASConditionCustomization::SaveDataToProperty(const FText& InText)

{
	if( DescriptionPropertyHandle.IsValid() )
	{
		DescriptionPropertyHandle.Get()->SetValue( InText.ToString() );
	}
}

#undef LOCTEXT_NAMESPACE