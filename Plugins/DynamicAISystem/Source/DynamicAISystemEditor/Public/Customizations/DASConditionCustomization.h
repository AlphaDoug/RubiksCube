// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once


#include "IPropertyTypeCustomization.h"



class FDASConditionCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FDASConditionCustomization()); }

	// BEGIN IPropertyTypeCustomization
	void CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils ) override;
	void CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils ) override;
	// END IPropertyTypeCustomization

protected:
	FText GetConditionDescText() const;
	FText GetConditionHintQueryDescText() const;

	TSharedPtr<IPropertyHandle> DescriptionPropertyHandle;
	TSharedPtr<IPropertyHandle> ConditionInstancePropertyHandle;

	// Called when text was changed
	void OnTextChanged( const FText& InText );

	// Called when text was changed
	void OnTextCommitted( const FText& InText, const ETextCommit::Type InTextAction );

	// Saves data from text box to description property
	void SaveDataToProperty( const FText& InText );
};
