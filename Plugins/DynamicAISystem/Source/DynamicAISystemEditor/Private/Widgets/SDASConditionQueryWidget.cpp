// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#include "Widgets/SDASConditionQueryWidget.h"

#include "EditorStyleSet.h"
#include "DynamicAISystem/Public/Objects/DASConditionQuery.h"


#define LOCTEXT_NAMESPACE "DASConditionQueryWidget"

void SDASConditionQueryWidget::Construct( const FArguments& InArgs, const TArray<FEditableDASConditionQueryDatum>& EditableConditionQueries )
{
	ensure( EditableConditionQueries.Num() > 0 );
	
	ConditionQueries = EditableConditionQueries;

	OnClosePreSave = InArgs._OnClosePreSave;
	OnSaveAndClose = InArgs._OnSaveAndClose;
	OnQueryChanged = InArgs._OnQueryChanged;
	OnCancel = InArgs._OnCancel;


	// create details view for the editable query object
	FDetailsViewArgs ViewArgs;
	ViewArgs.bHideSelectionTip = true;
	ViewArgs.bShowObjectLabel = false;
	
	UObject* outer = ConditionQueries[ 0 ].QueryOwner.Get();
	UDASConditionQuery* queryCopy = DuplicateObject( ConditionQueries[ 0 ].QueryWrapper->Instance, outer );
	EditableQuery = queryCopy;
	if( EditableQuery.IsValid() )
	{
		EditableQuery->AddToRoot();
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
	Details = PropertyModule.CreateDetailView( ViewArgs );
	Details->SetObject( queryCopy );
	Details->OnFinishedChangingProperties().AddSP( this, &SDASConditionQueryWidget::OnFinishedChangingProperties );

	ChildSlot
		[
			SNew( SVerticalBox )

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew( SHorizontalBox )

				// CLOSE WITHOUT SAVING BUTTON
				+SHorizontalBox::Slot()
				.HAlign( HAlign_Left )
				[
					SNew( SButton )
					.OnClicked( this, &SDASConditionQueryWidget::OnCancelClicked )
					.Text( LOCTEXT( "DASConditionQueryWidget_Cancel", "Close without Saving" ) )
				]

				// CLOSE AND SAVE BUTTON
				+SHorizontalBox::Slot()
				.HAlign( HAlign_Right )
				[
					SNew( SButton )
					.OnClicked( this, &SDASConditionQueryWidget::OnSaveAndCloseClicked )
					.Text( LOCTEXT( "DASConditionQueryWidget_SaveAndClose", "Save and Close" ) )
				]
			]

			// DETAILS VIEW OF CONDITIONS
			+SVerticalBox::Slot()
			[
				SNew( SBorder )
				.BorderImage( FEditorStyle::GetBrush( "ToolPanel.GroupBorder" ) )
				[
					Details.ToSharedRef()
				]
			]
			
		];
}

SDASConditionQueryWidget::~SDASConditionQueryWidget()
{
	if( EditableQuery.IsValid() )
	{
		// save only if closing didn't happen through pressing cancel button
		if( bCancelButtonClicked == false )
		{
			OnClosePreSave.ExecuteIfBound();

			SaveToConditionQuery();

			OnSaveAndClose.ExecuteIfBound();
		}
		

		EditableQuery.Get()->RemoveFromRoot();
	}
}

FReply SDASConditionQueryWidget::OnSaveAndCloseClicked()
{
	OnClosePreSave.ExecuteIfBound();

	SaveToConditionQuery();

	OnSaveAndClose.ExecuteIfBound();
	return FReply::Handled();
}

FReply SDASConditionQueryWidget::OnCancelClicked()
{
	bCancelButtonClicked = true;
	OnCancel.ExecuteIfBound();
	return FReply::Handled();
}

void SDASConditionQueryWidget::OnFinishedChangingProperties( const FPropertyChangedEvent& PropertyChangedEvent )
{
	OnQueryChanged.ExecuteIfBound();
}

void SDASConditionQueryWidget::SaveToConditionQuery()
{
	if( EditableQuery.IsValid() )
	{
		for( FEditableDASConditionQueryDatum& data : ConditionQueries )
		{
			data.QueryWrapper->Instance = EditableQuery.Get();

			if( data.QueryOwner.IsValid() )
			{
				data.QueryOwner->MarkPackageDirty();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE