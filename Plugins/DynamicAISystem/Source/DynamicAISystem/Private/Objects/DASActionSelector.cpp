// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Objects/DASActionSelector.h"
#include "Points/DASActionPoint.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Points/DASPathPoint.h"


#if WITH_EDITORONLY_DATA
#include "DrawDebugHelpers.h"
#endif




AActor* UDASActionSelector::GetOuterAsActor()
{
	if( UObject* outer = GetOuter() )
	{
		return Cast<AActor>( outer );
	}
	return nullptr;
}

ADASPathPoint* UDASActionSelector::GetOuterAsPathPoint()
{
	if( UObject* outer = GetOuter() )
	{
		return Cast<ADASPathPoint>( outer );
	}
	return nullptr;
}



#if WITH_ENGINE
UWorld* UDASActionSelector::GetWorld() const
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




