// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Customizations/DASConditionQueryCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SDASConditionQueryWidget.h"
#include "DynamicAISystem/Public/Objects/DASConditionQuery.h"



#define LOCTEXT_NAMESPACE "ConditionQueryCustomization"

void FDASConditionQueryCustomization::CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils )
{
	TSharedPtr<IPropertyHandle> InstancePropHandle = PropertyHandle->GetChildHandle( "Instance" );

	StructPropertyHandle = PropertyHandle;

	RefreshQueryDescription();

	bool bIsEditButtonEnabled = GetIsEditButtonEnabled();

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
			.MaxDesiredWidth( 500 )
			.MinDesiredWidth( 300 )
			[
				SNew( SBorder )
				.BorderBackgroundColor( FLinearColor::Black )
				.Padding( 5.f )
				.BorderImage( FCoreStyle::Get().GetBrush( "ToolPanel.GroupBorder" ) )
				[
					SNew( SVerticalBox )

					+SVerticalBox::Slot()
					[
						SNew( SHorizontalBox )

						// INSTANCE PROP BUTTON
						+ SHorizontalBox::Slot()
						//.MaxWidth( 400 )
						.HAlign( HAlign_Fill )
						[
							SNew( SProperty, InstancePropHandle )
							.ShouldDisplayName( false )
						]

						// EDIT BUTTON
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign( VAlign_Center )
						[
							SNew( SButton )
							.Text( LOCTEXT( "ConditionQueryCustomization_Edit", "Edit..." ) )
							.OnClicked( this, &FDASConditionQueryCustomization::OnEditButtonClicked )
							.IsEnabled( bIsEditButtonEnabled )
						]
					]

					// DESCRIPTION TEXT
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew( SBorder )
						.Padding( 10.f )
						.Visibility( this, &FDASConditionQueryCustomization::GetQueryDescVisibility )
						[
							SNew( STextBlock )
							.Text( this, &FDASConditionQueryCustomization::GetQueryDescText )
							.AutoWrapText( true )
						]
					]
				]
		];

	GEditor->RegisterForUndo( this );
}

void FDASConditionQueryCustomization::CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils )
{

}


bool FDASConditionQueryCustomization::GetIsEditButtonEnabled()
{
	// if query instance is valid, return true
	if( ( EditableQueries.Num() > 0 ) && ( EditableQueries[ 0 ].QueryWrapper != nullptr ) )
	{
		if( UDASConditionQuery* queryInst = EditableQueries[ 0 ].QueryWrapper->Instance )
		{
			return true;
		}
	}

	// otherwise return false, there is nothing to edit when query instance is empty
	return false;
}


FReply FDASConditionQueryCustomization::OnEditButtonClicked()
{
	if( ConditionQueryWidgetWindow.IsValid() )
	{
		// already open, just show it
		ConditionQueryWidgetWindow->BringToFront( true );
	}
	else
	{
		if( StructPropertyHandle.IsValid() )
		{
			TArray<UObject*> OuterObjects;
			StructPropertyHandle->GetOuterObjects( OuterObjects );

			// GENERATE TITLE FOR NEW WINDOW EDITOR
			FText Title;
			if( OuterObjects.Num() > 1 )
			{
				FText const AssetName = FText::Format( LOCTEXT( "ConditionDetailsBase_MultipleAssets", "{0} Assets" ), FText::AsNumber( OuterObjects.Num() ) );
				FText const PropertyName = StructPropertyHandle->GetPropertyDisplayName();
				Title = FText::Format( LOCTEXT( "ConditionQueryCustomization_BaseWidgetTitle", "Condition Query Editor: {0} {1}" ), PropertyName, AssetName );
			}
			else if( OuterObjects.Num() > 0 && OuterObjects[ 0 ] )
			{
				FText const AssetName = FText::FromString( OuterObjects[ 0 ]->GetName() );
				FText const PropertyName = StructPropertyHandle->GetPropertyDisplayName();
				Title = FText::Format( LOCTEXT( "ConditionQueryCustomization_BaseWidgetTitle", "Condition Query Editor: {0} {1}" ), PropertyName, AssetName );
			}

			// CREATE QUERY EDITOR WINDOW
			FVector2D ScreenSize( 800, 500 );
			FVector2D ScreenPosition = FSlateApplication::Get().GetCursorPos();
			ScreenPosition.X -= ScreenSize.X;
			ScreenPosition.Y -= ScreenSize.Y + 30;

			ConditionQueryWidgetWindow = SNew( SWindow )
				.Title( Title )
				.HasCloseButton( false )
				.ClientSize( ScreenSize )
				.ScreenPosition( ScreenPosition )
				.AutoCenter( EAutoCenter::None )
				[
					SNew( SDASConditionQueryWidget, EditableQueries )
					.OnSaveAndClose( this, &FDASConditionQueryCustomization::CloseWidgetWindow, false )
					.OnCancel( this, &FDASConditionQueryCustomization::CloseWidgetWindow, true )
					.OnClosePreSave( this, &FDASConditionQueryCustomization::PreSave )
				];

			// NOTE: FGlobalTabmanager::Get()-> is actually dereferencing a SharedReference, not a SharedPtr, so it cannot be null.
			if( FGlobalTabmanager::Get()->GetRootWindow().IsValid() )
			{
				FSlateApplication::Get().AddWindowAsNativeChild( ConditionQueryWidgetWindow.ToSharedRef(), FGlobalTabmanager::Get()->GetRootWindow().ToSharedRef() );
			}
			else
			{
				FSlateApplication::Get().AddWindow( ConditionQueryWidgetWindow.ToSharedRef() );
			}
		}
	}

	return FReply::Handled();
}


EVisibility FDASConditionQueryCustomization::GetQueryDescVisibility() const
{
	return QueryDescription.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

FText FDASConditionQueryCustomization::GetQueryDescText() const
{
	return FText::FromString( QueryDescription );
}

void FDASConditionQueryCustomization::PreSave()
{
	if( StructPropertyHandle.IsValid() )
	{
		StructPropertyHandle->NotifyPreChange();
	}
}

void FDASConditionQueryCustomization::CloseWidgetWindow( bool WasCancelled )
{
	if( !WasCancelled && StructPropertyHandle.IsValid() )
	{
		StructPropertyHandle->NotifyPostChange( EPropertyChangeType::Unspecified );
	}

	if( ConditionQueryWidgetWindow.IsValid() )
	{
		ConditionQueryWidgetWindow->RequestDestroyWindow();
		ConditionQueryWidgetWindow = nullptr;

		RefreshQueryDescription();
	}
}

void FDASConditionQueryCustomization::BuildEditableQueryList()
{
	EditableQueries.Empty();

	if( StructPropertyHandle.IsValid() )
	{
		TArray<void*> RawStructData;
		StructPropertyHandle->AccessRawData( RawStructData );

		TArray<UObject*> OuterObjects;
		StructPropertyHandle->GetOuterObjects( OuterObjects );

		for( int32 Idx = 0; Idx < RawStructData.Num(); ++Idx )
		{
			// Null outer objects may mean that we are inside a UDataTable. This is ok though. We can still dirty the data table via FNotify Hook. (see ::CloseWidgetWindow). However undo will not work.
			UObject* Obj = OuterObjects.IsValidIndex( Idx ) ? OuterObjects[ Idx ] : nullptr;

			EditableQueries.Add( SDASConditionQueryWidget::FEditableDASConditionQueryDatum( Obj, ( FDASConditionQueryWrapper* )RawStructData[ Idx ] ) );
		}
	}
}

void FDASConditionQueryCustomization::RefreshQueryDescription()
{
	// Rebuild Editable Containers as container references can become unsafe
	BuildEditableQueryList();

	// Clear the list
	QueryDescription.Empty();

	if( ( EditableQueries.Num() > 0 ) && ( EditableQueries[ 0 ].QueryWrapper != nullptr ) )
	{
		if( UDASConditionQuery* queryInst = EditableQueries[ 0 ].QueryWrapper->Instance )
		{
			QueryDescription = queryInst->GetQueryDescription();
		}
	}
}

FDASConditionQueryCustomization::~FDASConditionQueryCustomization()
{
	if( ConditionQueryWidgetWindow.IsValid() )
	{
		ConditionQueryWidgetWindow->RequestDestroyWindow();
	}

	GEditor->UnregisterForUndo( this );
}



#undef LOCTEXT_NAMESPACE
