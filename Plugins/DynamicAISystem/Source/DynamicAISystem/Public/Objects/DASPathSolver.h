// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DASPathSolver.generated.h"


class ADASPathPoint;

/**
 * Object used to solve which path point should be selected by AI as next one from available links
 */
UCLASS( BlueprintType, Blueprintable, Abstract, EditInlineNew, DefaultToInstanced )
class DYNAMICAISYSTEM_API UDASPathSolver : public UObject
{
	GENERATED_BODY()
	
public:
#if WITH_ENGINE
	/** Overriding this function gives access to all function requiring WorldContext */
	virtual class UWorld* GetWorld() const override;
#endif

	/**
	 * Should return one of path points from given array that AI will go to
	 * @param DASComponent - component of AI asking to select next path point
	 * @param PathPoints - All connected path points that AI should pick next one from
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = DASPathSolver )
	ADASPathPoint* SelectPathPoint( UDASComponent* DASComponent, const TArray<ADASPathPoint*>& LinkedPathPoints );
	ADASPathPoint* SelectPathPoint_Implementation( UDASComponent* DASComponent, const TArray<ADASPathPoint*>& LinkedPathPoints ) { return nullptr; }

	/**
	 * Removes all actions points that can't run ( failed condition ) from given array
	 * and returns only free ones in new filtered array
	 */
	UFUNCTION( BlueprintCallable, Category = DASPathSolver )
	void FilterOutPointsThatCantRun( const TArray<ADASPathPoint*>& PathPoints, TArray<ADASPathPoint*>& AvailablePathPoints );
};

