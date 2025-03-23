// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DASWorldSubsystem.generated.h"

class ADASPathPoint;
class ADASActionPoint;

/**
 * Globally accessible system
 * Stores points to all action points & path points that currently exist in the world
 */
UCLASS()
class DYNAMICAISYSTEM_API UDASWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UDASWorldSubsystem();




	/************************************************************************/
	/*								CACHED POINTS                           */
	/************************************************************************/
public:
	/**
	 * All Path Points that are currently in the world
	 * Much more efficient to access them when needed, than using GetAllActorsOfClass
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASWorldSubsystem )
	TArray< ADASPathPoint* > PathPoints;

	/**
	 * All Action Points that are currently in the world
	 * Much more efficient to access them when needed, than using GetAllActorsOfClass
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASWorldSubsystem )
	TArray< ADASActionPoint* > ActionPoints;

	/** function called by path point when its being created to cache its reference */
	void AddPathPoint( ADASPathPoint* PathPoint );

	/** function called by path point when its being destroyed to remove its reference */
	void RemovePathPoint( ADASPathPoint* PathPoint );


	/** function called by action point when its being created to cache its reference */
	void AddActionPoint( ADASActionPoint* ActionPoint );

	/** function called by action point when its being destroyed to remove its reference */
	void RemoveActionPoint( ADASActionPoint* ActionPoint );
	/************************************************************************/





	/** Returns path point in the world by given Id ( if there is any ) */
	UFUNCTION( BlueprintCallable, Category = DASWorldSubsystem )
	ADASPathPoint* FindPathPointById( const FGuid& Id );

	/** Returns action point in the world by given Id ( if there is any ) */
	UFUNCTION( BlueprintCallable, Category = DASWorldSubsystem )
	ADASActionPoint* FindActionPointById( const FGuid& Id );

	/**
	 * Finds closest path point in world to given source location
	 * @param PointTag - if valid, looks for points only matching this tag
	 */
	UFUNCTION( BlueprintCallable, Category = DASWorldSubsystem )
	ADASPathPoint* FindClosestPathPoint( const FVector& SourceLocation, FGameplayTag PointTag );

	/**
	 * Finds closest action point in world to given source location
	 * @param PointTag - if valid, looks for points only matching this tag
	 */
	UFUNCTION( BlueprintCallable, Category = DASWorldSubsystem )
	ADASActionPoint* FindClosestActionPoint( const FVector& SourceLocation, FGameplayTag PointTag );


	
};
