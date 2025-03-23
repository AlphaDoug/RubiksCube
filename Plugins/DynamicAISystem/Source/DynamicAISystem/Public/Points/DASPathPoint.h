// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DASBasePoint.h"
#include "DASPathPoint.generated.h"


class UDASActionSelector;
class UDASPathSolver;
class UDASComponent;

/**
 * Point which is used to create paths that AI is moving along
 * Path Point can be used by multiple AI at same time
 */
UCLASS( Abstract, Blueprintable, BlueprintType )
class DYNAMICAISYSTEM_API ADASPathPoint : public ADASBasePoint
{
	GENERATED_BODY()
	
public:	
	ADASPathPoint();


	/************************************************************************/
	/*							PARENT OVERRIDES                            */
	/************************************************************************/
public:
	virtual void PostDuplicate( EDuplicateMode::Type DuplicateMode ) override;
	virtual void PostRename( UObject* OldOuter, const FName OldName ) override;
	virtual void PostInitializeComponents() override;
	virtual void PostActorCreated() override;

	virtual void ValidateData_Implementation();
	virtual void DrawDebug( float DeltaTime, bool bIsInEditor ) override;
	virtual void RefreshInstancedObjects() override;

protected:
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
	/************************************************************************/



	

	/************************************************************************/
	/*							PATH CONNECTIONS							*/
	/************************************************************************/
public:
	/** Flag that enables/disables next path points, quick way to toggle it without modifying array of NextPathPoints */
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings", meta = ( InlineEditConditionToggle ) )
	uint32 bCanMoveForward : 1;

	/** Connections with other path points which are used when AI is moving forward */
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings", NonPIEDuplicateTransient, meta = ( EditCondition = "bCanMoveForward" ) )
	TArray<ADASPathPoint*> NextPathPoints;

	/**
	 * Defines how next path point should be picked
	 * None means that next path point will be picked randomly
	 */
	UPROPERTY( Instanced, EditInstanceOnly, BlueprintReadWrite, Category = "Settings" )
	UDASPathSolver* NextPathPointSolver = nullptr;


	/** Flag that enables/disables previous path points, quick way to toggle it without modifying array of PreviousPathPoints */
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings", meta = ( InlineEditConditionToggle ) )
	uint32 bCanMoveBackward : 1;

	/** Connections with other path points which are used when AI is moving backward */
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings", NonPIEDuplicateTransient, meta = ( EditCondition = "bCanMoveBackward" ) )
	TArray<ADASPathPoint*> PreviousPathPoints;

	/**
	 * Defines how next path point should be picked
	 * None means that next path point will be picked randomly
	 */
	UPROPERTY( Instanced, EditInstanceOnly, BlueprintReadWrite, Category = "Settings" )
	UDASPathSolver* PreviousPathPointSolver = nullptr;

	/**
	 * Returns next path point that AI should go to when this one is finished
	 * Called when AI is moving forward
	 */
	UFUNCTION( BlueprintCallable, Category = Settings)
	ADASPathPoint* GetNextPathPoint( UDASComponent* DASComponent );

	/**
	 * Returns previous path point that AI should go to when this one is finished
	 * Called when AI is moving backward
	 */
	UFUNCTION( BlueprintCallable, Category = Settings )
	ADASPathPoint* GetPreviousPathPoint( UDASComponent* DASComponent );
	/************************************************************************/





	/************************************************************************/
	/*							PATH ACTIONS								*/
	/************************************************************************/
public:
	/**
	 * Defines when AI should execute actions from ActionSelector on reaching this path point
	 * None - don't execute any actions
	 * BothWays - execute actions when AI is moving both forward and backward
	 * Forward	- execute actions only when AI is moving forward
	 * Backward - execute actions only when AI is moving backward
	 */
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Actions" )
	EDASPathExecuteMethod PathActionExecutionMethod = EDASPathExecuteMethod::BothWays;

	/**
	 * Object which is used for selecting ActionPoints
	 * that will be used when AI reached that path point
	 */
	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Settings|Actions", Instanced, meta = ( EditCondition = "PathActionExecutionMethod != EDASPathExecuteMethod::None" ) )
	UDASActionSelector* ActionSelector = nullptr;

	/**
	 * This only has effects if path point has connections with any action points
	 * Should AI return to path point after finishing action points connected to it?
	 * if not, then after finishing Action Points, AI will go straight to next path point without returning to current one
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	uint32 bReturnToPathPointAfterExecutingActions : 1;
	/************************************************************************/
	

	


	/************************************************************************/
	/*								SPOTS									*/
	/************************************************************************/
public:
	/**
	 * Array of spots which are used as movement goal for AI ( places where AI will go to reach this path point )
	 * If doesn't have any elements, then PathPoint itself will be used as a spot
	 * This may be useful if single PathPoint will be used by multiple AIs
	 * AI moving to path point will always look for closest spot, that is not used by other AIs
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement", meta = ( DisplayPriority = 10 ) )
	TArray<FDASSpot> Spots;

	/**
	 * If there aren't any spots, it simply returns location and rotation of this path point
	 * If there are spots, then it picks closest one that is free and marks it as taken
	 * so other AIs using same path point are aware of it
	 */
	UFUNCTION( BlueprintCallable, Category = "Settings|Movement" )
	virtual void GetPointLocationAndRotation( FVector& OutLocation, FRotator& OutRotation, AActor* Querier );

	/**
	 * Marks all spots that were taken by given Querier as free
	 * This function is called when AI finished moving to path point
	 */
	UFUNCTION( BlueprintCallable, Category = "DASPathPoint")
	void ReleaseSpot( AActor* Querier );

protected:
	/**
	 * Finds closest and free spot ( or random one if all are taken )
	 * and marks it as taken by Querier ( AI )
	 */
	void RequestSpot( FDASSpot& OutSpot, AActor* Querier );

	/**
	 * Generates spots in random positions near path point
	 * This is editor-only function
	 */
	UFUNCTION( CallInEditor, Category = Settings )
	void GenerateSpots();

	/**
	 * Removes generated spots
	 * This is editor-only function
	 */
	UFUNCTION( CallInEditor, Category = Settings )
	void ClearSpots();

#if WITH_EDITORONLY_DATA
	/** Number of spots to generate on pressing button GenerateSpots */
	UPROPERTY( EditAnywhere, Category = Settings, AdvancedDisplay )
	int32 NumOfSpotsToGenerate = 3;

	/** Unique name of this point, used when duplicating to copy data from source path point */
	UPROPERTY( VisibleInstanceOnly, Category = Editor )
	FString SelfName;
#endif
	/************************************************************************/

};



