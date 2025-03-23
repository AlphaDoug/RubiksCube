// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Objects/DASCondition.h"
#include "GameFramework/Character.h"
#include "AIController.h"

UDASCondition::UDASCondition()
{
	CachedConditionResult = ECachedConditionResult::Undefined;
	bIsInitialized = false;
	ConditionOwner = nullptr;
}


#if WITH_ENGINE
UWorld* UDASCondition::GetWorld() const
{
	if( GetOuter() == nullptr )
	{
		return nullptr;
	}

#if WITH_EDITOR
	// Special case for the editor
	if( Cast<UPackage>( GetOuter() ) != nullptr )
	{
		// GetOuter should return a UPackage and its Outer is a UWorld
		return Cast<UWorld>( GetOuter()->GetOuter() );
	}
#endif

	// In all other cases use outer
	return GetOuter()->GetWorld();
}
#endif

bool UDASCondition::IsConditionFulfilled()
{
	// cache previous condition state
	ECachedConditionResult previousResult = CachedConditionResult;

	// get current state of condition
	bool bResult = IsConditionFulfilled_Internal();

	// cache new condition result
	SetCachedConditionResult( bResult );

	// call on changed events only if condition result really changed
	if( previousResult == ECachedConditionResult::Undefined ||
		bResult && previousResult == ECachedConditionResult::False ||
		!bResult && previousResult == ECachedConditionResult::True )
	{
		ConditionResultChanged( bResult );
		OnConditionResultChanged.Broadcast( bResult );
	}

	return bResult;
}


void UDASCondition::Initialize( AActor* Owner )
{
	if( bIsInitialized == false )
	{
		ValidateData();
		ConditionOwner = Owner;
		bIsInitialized = true;
		AddObservers();
	}
}

void UDASCondition::Uninitialize()
{
	if( bIsInitialized )
	{
		bIsInitialized = false;
		CachedConditionResult = ECachedConditionResult::Undefined;
		RemoveObservers();
	}
}

class ACharacter* UDASCondition::GetOwnerAsCharacter()
{
	if( ConditionOwner.IsValid() == false )
		return nullptr;

	ACharacter* character = nullptr;

	character = Cast<ACharacter>( ConditionOwner.Get() );
	if( character )
	{
		return character;
	}

	// if owner is not base character, then check if its AIController
	if( AAIController* AIController = Cast<AAIController>( ConditionOwner.Get() ) )
	{
		character = Cast<ACharacter>( AIController->GetPawn() );
	}

	return character;
}

class AAIController* UDASCondition::GetOwnerAsAIController()
{
	if( ConditionOwner.IsValid() == false )
		return nullptr;

	return Cast<AAIController>( ConditionOwner.Get() );
}

