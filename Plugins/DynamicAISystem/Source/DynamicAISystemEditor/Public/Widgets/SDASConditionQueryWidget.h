// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"
#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "GameplayTagContainer.h"

class IDetailsView;
struct FPropertyChangedEvent;


class SDASConditionQueryWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SDASConditionQueryWidget )
	{}
		SLATE_EVENT( FSimpleDelegate, OnClosePreSave ) // Called when "Save and Close" button clicked
		SLATE_EVENT( FSimpleDelegate, OnSaveAndClose ) // Called when "Save and Close" button clicked
		SLATE_EVENT(FSimpleDelegate, OnCancel) // Called when "Close Without Saving" button clicked
		SLATE_EVENT( FSimpleDelegate, OnQueryChanged )	// Called when the user has modified the query
		SLATE_END_ARGS()


	/** Simple struct holding a condition query and its owner for generic re-use of the widget */
	struct FEditableDASConditionQueryDatum
	{
		/** Constructor */
			FEditableDASConditionQueryDatum( class UObject* InOwnerObj, struct FDASConditionQueryWrapper* InConditionQuery )
			: QueryOwner( InOwnerObj )
			, QueryWrapper( InConditionQuery )
		{}

		/** Owning UObject of the query being edited */
		TWeakObjectPtr<class UObject> QueryOwner;

		/** Condition query to edit */
		struct FDASConditionQueryWrapper* QueryWrapper;
	};

	/** Construct the actual widget */
	void Construct(const FArguments& InArgs, const TArray<FEditableDASConditionQueryDatum>& EditableConditionQueries);

	~SDASConditionQueryWidget();


private:
	/** Containers to modify */
	TArray<FEditableDASConditionQueryDatum> ConditionQueries;

	/** Query created only for purpose of custom editable window, its value will be passed to original variable */
	TWeakObjectPtr<class UDASConditionQuery> EditableQuery;

	/** Called when "Save and Close" is clicked before we save the data. */
	FSimpleDelegate OnClosePreSave;

	/** Called when "Save and Close" is clicked after we have saved the data. */
	FSimpleDelegate OnSaveAndClose;

	/** Called when the user has modified the query */
	FSimpleDelegate OnQueryChanged;

	/** Called when "Close Without Saving" is clicked */
	FSimpleDelegate OnCancel;

	/** Properties Tab */
	TSharedPtr<class IDetailsView> Details;

	/** Called when the user clicks the "Save and Close" button */
	FReply OnSaveAndCloseClicked();

	/** Called when the user clicks the "Close Without Saving" button */
	FReply OnCancelClicked();

	/** Called when user finished changing properties */
	void OnFinishedChangingProperties( const FPropertyChangedEvent& PropertyChangedEvent );

	/** Saves edited query to original variable query */
	void SaveToConditionQuery();

	/** Flag that is set to true upon pressing Close without Saving button */
	uint32 bCancelButtonClicked : 1;
};




