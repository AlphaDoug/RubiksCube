// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Utils/DASWorldSubsystem.h"
#include "Points/DASPathPoint.h"
#include "Points/DASActionPoint.h"



UDASWorldSubsystem::UDASWorldSubsystem()
{
	// reserve some space for point arrays
	ActionPoints.Reserve( 128 );
	PathPoints.Reserve( 128 );
}

ADASPathPoint* UDASWorldSubsystem::FindPathPointById( const FGuid& Id )
{
	ADASPathPoint* ret = nullptr;

	// find point by Id
	ADASPathPoint** result = PathPoints.FindByPredicate( [&Id]( const ADASPathPoint* point )
		{
			return ( point && point->PointId == Id );
		} );

	// if found, update ret value
	if( result )
	{
		ret = *result;
	}

	return ret;
}

ADASActionPoint* UDASWorldSubsystem::FindActionPointById( const FGuid& Id )
{
	ADASActionPoint* ret = nullptr;

	// find point by Id
	ADASActionPoint** result = ActionPoints.FindByPredicate( [&Id]( const ADASActionPoint* point )
		{
			return ( point && point->PointId == Id );
		} );

	// if found, update ret value
	if( result )
	{
		ret = *result;
	}

	return ret;
}

ADASPathPoint* UDASWorldSubsystem::FindClosestPathPoint( const FVector& SourceLocation, FGameplayTag PointTag )
{
	ADASPathPoint* retPoint = nullptr;
	int32 foundIndex = INDEX_NONE;
	float closestDistance = FLT_MAX;

	for( ADASPathPoint* point : PathPoints )
	{
		if( IsValid( point ) )
		{
			// skip checking this point if its tag doesn't match
			if( !PointTag.IsValid() || !point->PointTag.MatchesTag( PointTag ) )
				continue;


			float distanceToPoint = FVector::Distance( SourceLocation, point->GetActorLocation() );
			if( distanceToPoint < closestDistance )
			{
				closestDistance = distanceToPoint;
				retPoint = point;
			}
		}
	}
	return retPoint;
}

ADASActionPoint* UDASWorldSubsystem::FindClosestActionPoint( const FVector& SourceLocation, FGameplayTag PointTag )
{
	ADASActionPoint* retPoint = nullptr;
	int32 foundIndex = INDEX_NONE;
	float closestDistance = FLT_MAX;

	for( ADASActionPoint* point : ActionPoints )
	{
		if( IsValid( point ) )
		{
			// skip checking this point if its tag doesn't match
			if( !PointTag.IsValid() || !point->PointTag.MatchesTag( PointTag ) )
				continue;


			float distanceToPoint = FVector::Distance( SourceLocation, point->GetActorLocation() );
			if( distanceToPoint < closestDistance )
			{
				closestDistance = distanceToPoint;
				retPoint = point;
			}
		}
	}
	return retPoint;
}

void UDASWorldSubsystem::AddPathPoint( ADASPathPoint* PathPoint )
{
	PathPoints.Add( PathPoint );
}

void UDASWorldSubsystem::RemovePathPoint( ADASPathPoint* PathPoint )
{
	PathPoints.Remove( PathPoint );
}

void UDASWorldSubsystem::AddActionPoint( ADASActionPoint* ActionPoint )
{
	ActionPoints.Add( ActionPoint );
}

void UDASWorldSubsystem::RemoveActionPoint( ADASActionPoint* ActionPoint )
{
	ActionPoints.Remove( ActionPoint );
}
