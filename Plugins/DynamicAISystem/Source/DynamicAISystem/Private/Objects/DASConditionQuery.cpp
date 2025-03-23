// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Objects/DASConditionQuery.h"



/** Struct helper functio to check if condition query within it is fulfilled */
bool FDASConditionQueryWrapper::IsConditionFulfilled()
{
	if( Instance )
	{
		return Instance->IsConditionFulfilled();
	}
	return true;
}

/** Struct helper function to initialize condition query within it */
bool FDASConditionQueryWrapper::Initialize( AActor* QueryOwner )
{
	if( Instance )
	{
		Instance->Initialize( QueryOwner );
		return true;
	}
	return false;
}









UDASConditionQuery::UDASConditionQuery()
{
	bIsInitialized = false;
	CachedConditionResult = ECachedConditionResult::Undefined;
	ConditionOwner = nullptr;
}

void UDASConditionQuery::Initialize( AActor* Owner )
{
	if( bIsInitialized == false )
	{
		bIsInitialized = true;
		ConditionOwner = Owner;

		// loop through all conditions and initialize them
		for( int32 i = Conditions.Num() - 1; i >= 0; i-- )
		{
			if( Conditions[ i ].Instance )
			{
				// init and start observing condition
				Conditions[ i ].Instance->Initialize( Owner );
				Conditions[ i ].Instance->OnConditionResultChanged.AddUniqueDynamic( this, &UDASConditionQuery::OnInnerConditionResultChanged );
			}
			else
			{
				Conditions.RemoveAt( i );
			}
		}

		// call to update current result of condition
		IsConditionFulfilled();
	}
}

void UDASConditionQuery::Uninitialize()
{
	if( bIsInitialized )
	{
		bIsInitialized = false;
		CachedConditionResult = ECachedConditionResult::Undefined;

		// loop through all conditions and initialize them
		for( int32 i = Conditions.Num() - 1; i >= 0; i-- )
		{
			if( Conditions[ i ].Instance )
			{
				// uninit and stop observing condition
				Conditions[ i ].Instance->Uninitialize();
				Conditions[ i ].Instance->OnConditionResultChanged.RemoveDynamic( this, &UDASConditionQuery::OnInnerConditionResultChanged );
			}
			else
			{
				Conditions.RemoveAt( i );
			}
		}
	}
}


bool UDASConditionQuery::IsConditionFulfilled()
{
	// cache previous condition value
	ECachedConditionResult previousResult = CachedConditionResult;

	bool bResult = IsConditionFulfilled_Internal();

	// cache new condition result
	SetCachedConditionResult( bResult );

	// call on changed events only if condition result really changed
	if( previousResult == ECachedConditionResult::Undefined ||
		bResult && previousResult == ECachedConditionResult::False ||
		!bResult && previousResult == ECachedConditionResult::True )
	{
		OnConditionResultChanged.Broadcast( bResult );
	}

	return bResult;
}

bool UDASConditionQuery::IsConditionFulfilled_Internal()
{
	bool bFinalResult = true; // for OR give it false, for AND give it true
	bool bHasCheckedFirstCondition = false;

	// loop through all conditions
	for( FDASConditionWrapper& condWrapper : Conditions )
	{
		// for first condition don't check any logic related to operators because it starts counting from 2nd condition
		if( bHasCheckedFirstCondition == false )
		{
			bHasCheckedFirstCondition = true;
		}

		// if previous condition value was FALSE and operator is AND then condition will fail, no reason to check if further
		else if( bFinalResult == false && condWrapper.Operator == EDASOperator::AND )
		{
			break;
		}

		// if previous condition value was true, and operator is OR, then skip checking this condition because its result won't change the final result
		else if( bFinalResult && condWrapper.Operator == EDASOperator::OR )
		{
			continue;
		}

		// in any other case, get value of condition and assign it to combined conditions result
		bFinalResult = condWrapper.Instance->IsConditionFulfilled();
	}

	return bFinalResult;
}


void UDASConditionQuery::DrawDebug( float DeltaTime, AActor* Caller, bool bIsInEditor )
{
	for( int32 i = 0; i < Conditions.Num(); i++ )
	{
		if( Conditions[ i ].Instance )
		{
			Conditions[ i ].Instance->DrawDebug( DeltaTime, Caller, bIsInEditor, i );
		}
	}
}


void UDASConditionQuery::ValidateData()
{
	for( int32 i = 0; i < Conditions.Num(); i++ )
	{
		if( Conditions[ i ].Instance )
		{
			Conditions[ i ].Instance->ValidateData();
		}
	}
}



FString UDASConditionQuery::GetQueryDescription() const
{
	FString QueryDescription;

#if WITH_EDITOR
	int32 idx = 0;
	for( const FDASConditionWrapper& condWrapper : Conditions )
	{
		if( condWrapper.Instance )
		{
			if( !condWrapper.bIsFirstCondition )
			{
				FString operatorString = ( condWrapper.Operator == EDASOperator::AND ) ? TEXT( "AND" ) : TEXT( "OR" );
				QueryDescription += LINE_TERMINATOR;
				QueryDescription += FString::Printf( TEXT( "             --%s--" ), *operatorString );
				QueryDescription += LINE_TERMINATOR;
			}

			if( condWrapper.Description.IsEmpty() )
			{
				QueryDescription += FString::Printf( TEXT( "[ %d ]  %s" ), idx, *condWrapper.Instance->GetConditionDescription() );
			}
			else
			{
				QueryDescription += FString::Printf( TEXT( "[ %d ]  %s" ), idx, *condWrapper.Description );
			}
		}

		idx++;
	}
#endif
	return QueryDescription;
}



#if WITH_EDITOR
void UDASConditionQuery::PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent )
{
	Super::PostEditChangeProperty( PropertyChangedEvent );

#if WITH_EDITORONLY_DATA
	for( int32 i = 0; i < Conditions.Num(); i++ )
	{
		// set this flag to true only for elem at index 0
		Conditions[ i ].bIsFirstCondition = ( i == 0 );
	}
#endif
}
#endif

void UDASConditionQuery::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITORONLY_DATA
	for( int32 i = 0; i < Conditions.Num(); i++ )
	{
		// set this flag to true only for elem at index 0
		Conditions[ i ].bIsFirstCondition = ( i == 0 );
	}
#endif
}