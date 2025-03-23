// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once


#include "IPropertyTypeCustomization.h"
#include "Widgets/SDASConditionQueryWidget.h"
#include "EditorUndoClient.h"



class FDASConditionQueryCustomization : public IPropertyTypeCustomization, public FEditorUndoClient
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FDASConditionQueryCustomization()); }

	~FDASConditionQueryCustomization();

	// BEGIN IPropertyTypeCustomization
	void CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils ) override;
	void CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils ) override;
	// END IPropertyTypeCustomization

protected:
	/** Description of query displayed when query details panel is hidden */
	FString QueryDescription;

	/** Defines when user can press Edit query button */
	bool GetIsEditButtonEnabled();

	/** Called when the edit button is clicked; Launches the gameplay condition editor */
	FReply OnEditButtonClicked();

	/** Returns the visibility of the tags list box (collapsed when there are no conditions) */
	EVisibility GetQueryDescVisibility() const;

	FText GetQueryDescText() const;

	void PreSave();

	void CloseWidgetWindow( bool WasCancelled );

	/** Build List of Editable Queries */
	void BuildEditableQueryList();

	void RefreshQueryDescription();

	/** Cached property handle */
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	/** The Window for the GameplayConditionWidget */
	TSharedPtr<SWindow> ConditionQueryWidgetWindow; //GameplayConditionQueryWidgetWindow;

	/** The array of queries this objects has */
	TArray<SDASConditionQueryWidget::FEditableDASConditionQueryDatum> EditableQueries;
};
