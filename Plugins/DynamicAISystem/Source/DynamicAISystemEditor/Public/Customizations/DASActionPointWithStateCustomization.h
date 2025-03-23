// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once


#include "IPropertyTypeCustomization.h"



class FDASActionPointWithStateCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShareable(new FDASActionPointWithStateCustomization()); }

	// BEGIN IPropertyTypeCustomization
	void CustomizeHeader( TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils ) override;
	void CustomizeChildren( TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils ) override;
	// END IPropertyTypeCustomization
};
