// Copyright (C) 2021 Grzegorz Szewczyk - All Rights Reserved

#pragma once

// ----------------------------------------------------------------------------------------------------------------
// This header is for enums and structs used by classes and blueprints accross the DAS Plugin
// Collecting these in a single header helps avoid problems with recursive header includes
// ----------------------------------------------------------------------------------------------------------------

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "DASTypes.generated.h"


#if WITH_EDITOR
/** Console command to enable/disable debug */
static TAutoConsoleVariable<bool> DAS_Debug(
	TEXT( "DAS.Debug" ),
	1,
	TEXT( "Visualize debug info for DAS system" ) );
#endif


/** Delegate used by Async Nodes, to notify when it was finished, includes finish result value */
DECLARE_DELEGATE_OneParam( FAsyncExecutionFinishedWithResultDelegate, EDASExecutionResult );


/**
 * Running Mode of AI using DAS behavior tree
 * defines whether AI should use path points or just action selector
 */
UENUM( BlueprintType )
enum class EDASRunMode : uint8
{
	Undefined					UMETA( Tooltip = "AI DAS logic should not execute" ),
	ExecutePathPoints			UMETA( Tooltip = "AI should execute path point specified on DAS Component" ),
	ExecuteActionsFromSelector	UMETA( Tooltip = "AI Should execute action points specified on DAS Component Action Selector" )
};

/**
 * Defines what AI currently is doing
 */
UENUM( BlueprintType )
enum class EDASPathBehavior : uint8
{
	Undefined				UMETA( Tooltip = "AI is not executing DAS Behavior Tree" ),
	MovingToPathPoint		UMETA( Tooltip = "AI is moving to path point" ),
	ReturningToPathPoint	UMETA( Tooltip = "AI is returning to path point (after finishing actions points)" ),
	MovingToActionPoint		UMETA( Tooltip = "AI is moving to action point" ),
	ExecutingActionPoint	UMETA( Tooltip = "AI is executing action point" )
};

UENUM( BlueprintType )
enum class EDASExecutionResult : uint8
{
	Success,
	Failed
};

UENUM( BlueprintType )
enum class EDASOperator : uint8
{
	AND,
	OR
};

UENUM( BlueprintType )
enum class EDASPointType : uint8
{
	Undefined,
	PathPoint,
	ActionPoint
};

/**
 * Defines how elements should be executed
 * for example action with array of animations
 */
UENUM( BlueprintType )
enum class EDASExecuteMethod : uint8
{
	None			UMETA( ToolTip = "Don't execute any elements" ),
	Multiple		UMETA( ToolTip = "Execute all elements one by one" ),
	SingleRandom	UMETA( ToolTip = "Execute only single element chosen randomly" )
};


/**
 * Defines how points should be executed
 */
UENUM( BlueprintType )
enum class EDASPointExecutionMethod : uint8
{
	None			UMETA( ToolTip = "Don't execute any element" ),
	Multiple		UMETA( ToolTip = "Execute all elements one by one" ),
	SingleRandom	UMETA( ToolTip = "Execute single element randomly chosen" ),
	SingleClosest	UMETA( ToolTip = "Execute single element closest to caller" )
};

/**
 * Defines when action points connected to path point should be executed
 */
UENUM( BlueprintType )
enum class EDASPathExecuteMethod : uint8
{
	None		UMETA( ToolTip = "Don't execute any actions at all" ),
	BothWays	UMETA( ToolTip = "Execute actions when AI is moving forward and backward" ),
	Forward		UMETA( ToolTip = "Execute actions only when AI is moving forward" ),
	Backward	UMETA( ToolTip = "Execute actions only when AI is moving backward" )
};

/**
 * Spot struct used by points
 * AI is selecting them on moving to the point
 */
USTRUCT( BlueprintType )
struct DYNAMICAISYSTEM_API FDASSpot
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASSpot, meta = ( MakeEditWidget ) )
	FTransform Transform;

	/**
	 * Who is currently using this spot
	 * None if no one is using it
	 */
	UPROPERTY()
	AActor* SpotOwner = nullptr;

	bool IsTaken() const;
	bool IsFree() const { return !IsTaken(); }

	FORCEINLINE void TakeSpot( AActor* Owner ) { if( IsFree() ) SpotOwner = Owner; }
	FORCEINLINE void FreeSpot() { SpotOwner = nullptr; }

	/** overloaded operator used to find spot in array by Actor */
	FORCEINLINE bool operator == ( const AActor* const Actor ) const
	{
		return SpotOwner == Actor;
	}
};


/** Defines current state of action point */
UENUM( BlueprintType )
enum class EDASActionPointState : uint8
{
	Executing,
	NotExecuting
};

/**
 * Wrapper of action selector instance
 * Used for creating action selector instances as variables in blueprints
 * because blueprints doesn't support Instanced UObjects variables
 */
USTRUCT( BlueprintType )
struct DYNAMICAISYSTEM_API FDASActionSelectorWrapper
{
	GENERATED_BODY()

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Instanced, Category = ActionSelectorWrapper )
	class UDASActionSelector* ActionSelector = nullptr;
};

/**
 * Helper struct wrapping data of action point and its state
 * used in condition checking state of action points
 */
USTRUCT( BlueprintType )
struct DYNAMICAISYSTEM_API FDASActionPointWithState
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPointWithState )
	class ADASActionPoint* ActionPoint = nullptr;

	/** State in which Action Point should be in */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ActionPointWithState )
	EDASActionPointState State = EDASActionPointState::Executing;
};


/**
 * DAS Save Game Archiver
 * Saves variables only with flag 'SaveGame'
 * Currently used to save data of UDASActionSelector
 */
struct FDASSaveGameArchive : public FObjectAndNameAsStringProxyArchive
{
	FDASSaveGameArchive( FArchive& InInnerArchive )
		: FObjectAndNameAsStringProxyArchive( InInnerArchive, true )
	{
		ArIsSaveGame = true;
	}
};


/**
 * Snapshot of DAS Component
 * Stores all data needed to recreate state of DAS Component upon loading
 */
USTRUCT( BlueprintType )
struct DYNAMICAISYSTEM_API FDASComponentSnapshot
{
	GENERATED_BODY()

	/** Id of path point that was active */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASComponentSnapshot )
	FGuid ActivePathPointId;

	/** Id of action points that were in queue waiting to be executed */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASComponentSnapshot )
	TArray<FGuid> ActionPointsIdQueue;

	/** Flag defining if AI was moving forward or backward */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASComponentSnapshot )
	uint32 bWasMovingForward : 1;

	/** Flag defining if AI was going back to path point after finishing execution of action points connected with that path point */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASComponentSnapshot )
	uint32 bWasReturningToPathPoint : 1;

	/** Mode that AI was in */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASComponentSnapshot )
	EDASRunMode RunMode;

	/** Class of Action Selector */
	UPROPERTY()
	UClass* ActionSelectorClass = nullptr;

	/** Data of Action Selected */
	UPROPERTY()
	TArray<uint8> ActionSelectorData;

	/** Location of owner actor of DAS Component */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASComponentSnapshot )
	FVector OwnerLocation;

	/** Rotation of owner actor of DAS Component */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASComponentSnapshot )
	FRotator OwnerRotation;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASComponentSnapshot )
	uint32 bHasNewPathPoint : 1;
};



