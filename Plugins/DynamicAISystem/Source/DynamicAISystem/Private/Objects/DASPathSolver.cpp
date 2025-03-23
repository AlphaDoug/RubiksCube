// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Objects/DASPathSolver.h"
#include "Points/DASPathPoint.h"


#if WITH_ENGINE
class UWorld* UDASPathSolver::GetWorld() const
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

void UDASPathSolver::FilterOutPointsThatCantRun( const TArray<ADASPathPoint*>& PathPoints, TArray<ADASPathPoint*>& AvailablePathPoints )
{
	// reset array if it had any elements and prepare memory to add new points into it
	AvailablePathPoints.Reset( PathPoints.Num() );

	// add only those path points which can run
	for( ADASPathPoint* pathPoint : PathPoints )
	{
		if( pathPoint )
		{
			if( pathPoint->CanRun() )
			{
				AvailablePathPoints.Add( pathPoint );
			}
		}
	}
}
