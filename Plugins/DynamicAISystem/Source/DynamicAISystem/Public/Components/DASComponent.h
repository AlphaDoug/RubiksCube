// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "DynamicAISystem.h"
#include "Components/ActorComponent.h"
#include "Utils/DASTypes.h"
#include "DASComponent.generated.h"



class AAIController;
class ACharacter;
class APawn;
class UDASActionSelector;
class ADASPathPoint;
class ADASActionPoint;
class ADASPathPoint;
class UBehaviorTree;


DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnInitialized );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnPathPointChanged, ADASPathPoint*, PreviousPathPoint, ADASPathPoint*, NewPathPoint );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnActionPointChanged, ADASActionPoint*, PreviousActionPoint, ADASActionPoint*, NewActionPoint );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnPathBehaviorChanged, EDASPathBehavior, PreviousPathBehavior, EDASPathBehavior, NewPathBehavior );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnRunModeChanged, EDASRunMode, PreviousRunMode, EDASRunMode, NewRunMode );


/**
 * Main Component of DAS plugin which manages AI logic while using system and stores data
 * Must be added to AI Pawn/Character to allow using Patrol/Action points and other features of DAS plugin
 * !Don't use white signs ( space, tab ) in name of this component!
 */
UCLASS( Blueprintable, BlueprintType, ClassGroup = ( DAS ), meta = ( BlueprintSpawnableComponent, DisplayName = "DAS Component" ))
class DYNAMICAISYSTEM_API UDASComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDASComponent();

	/************************************************************************/
	/*							PARENT OVERRIDES							*/
	/************************************************************************/
public:
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

protected:
	virtual void OnRegister() override;
	/************************************************************************/





	/************************************************************************/
	/*								SETTINGS                                */
	/************************************************************************/
public:
	/**
	 * Defines whether AI should execute path points ( and action points connected with it )
	 * or only action points from current ActionSelector
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Settings )
	EDASRunMode RunMode;

	/** Path point that AI will start from */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Settings, meta = ( EditCondition = "RunMode == EDASRunMode::ExecutePathPoints" ) )
	ADASPathPoint* InitialPathPoint = nullptr;

	/** Selector defining which action points should be executed */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Settings, Instanced, meta = ( EditCondition = "RunMode == EDASRunMode::ExecuteActionsFromSelector" ) )
	UDASActionSelector* ActionSelector = nullptr;

	/** Changes run mode ( between path points or action selector ) */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetRunMode( EDASRunMode NewRunMode );

	/** Changes action selector */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetActionSelector( UDASActionSelector* NewActionSelector );

	/**
	 * Sets initial path point
	 * Should be called if AI is spawned in runtime to setup path point
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetInitialPathPoint( ADASPathPoint* NewPathPoint );

protected:
	/** Called when RunMode is changed */
	UFUNCTION( BlueprintImplementableEvent, Category = DASComponent )
	void RunModeChanged( EDASRunMode PreviousMode, EDASRunMode NewMode );

	/** Called when action selector is changed */
	UFUNCTION( BlueprintImplementableEvent, Category = DASComponent )
	void ActionSelectorChanged( UDASActionSelector* NewActionSelector );
	/************************************************************************/





	/************************************************************************/
	/*								INITIALIZATION							*/
	/************************************************************************/
public:
	/**
	 * Initializes DAS Component & starts logic of AI
	 * Behavior tree must be or must include BT_DASBase and all of its blackboard keys
	 * @param BehaviorTree - behavior tree that will be running
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	bool Init( UBehaviorTree* BehaviorTree );

	/**
	 * Completely resets all data of DASComponent
	 * may be useful when AI is using pool manager and wants to reset DASComponent on getting to the pool
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void Reset();

	/**
	 * Wraps all important data about current state of DAS Component into struct
	 * which can be used to 'load' DAS Component by function LoadFromSnapshot
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	FDASComponentSnapshot GetSnapshot();

	/**
	 * Loads state of DAS component from given snapshot
	 * @param bLoadOwnerTransform - should also load location/rotation of owner?
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void LoadFromSnapshot( const FDASComponentSnapshot& Snapshot, bool bLoadOwnerTransform = true );

protected:
	/** Flag telling if DAS Component was already initialized */
	UPROPERTY( ReplicatedUsing = "OnRep_IsInitialized", BlueprintReadOnly, Category = DASComponent )
	uint32 bIsInitialized : 1;

	UFUNCTION()
	void OnRep_IsInitialized();

	/**
	 * Changes flag is initialized, takes care of replication and calling events
	 */
	void SetIsInitialized( bool bNewInitialized );

	/** Event called when DASComponent gets initialized */
	UFUNCTION( BlueprintImplementableEvent, Category = DASComponent )
	void ReceiveInit();
	/************************************************************************/





	/************************************************************************/
	/*									EVENTS								*/
	/************************************************************************/
public:
	/** Event called when path point changes */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnPathPointChanged OnPathPointChanged;

	/** Event called when action point changes */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnActionPointChanged OnActionPointChanged;

	/** Event called when path behavior changes */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnPathBehaviorChanged OnPathBehaviorChanged;

	/** Event called when run mode changes */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnRunModeChanged OnRunModeChanged;

	/** Event called when DAS component gets initialized */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnInitialized OnInitialized;
	/************************************************************************/






	/************************************************************************/
	/*									STATE								*/
	/************************************************************************/
public:
	/**
	 * Defines current state of AI
	 * whether it moves to path point/executes action point etc.
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASComponent )
	EDASPathBehavior PathBehavior;

	/** Changes current path behavior */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetPathBehavior( EDASPathBehavior NewPathBehavior );

protected:
	/** Called when path behavior changes */
	UFUNCTION( BlueprintImplementableEvent, Category = DASComponent )
	void PathBehaviorChanged( EDASPathBehavior PreviousBehavior, EDASPathBehavior NewBehavior );
	/************************************************************************/





	/************************************************************************/
	/*                        GOAL ROTATION & LOCATION                      */
	/************************************************************************/
public:
	
	/**
	 * This value represents how far AI can move from point that is currently executed
	 * if executed point doesn't allow AI to be moved from it and distance from it will exceed that value
	 * then execution of point will be interrupted and AI will try to adjust location and start it again
	 */
	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = Settings )
	float MoveFromPointDistanceTolerance = 50.f;

	/** If true, MoveFromPointDistanceTolerance will be auto calculated based on radius of character capsule */
	UPROPERTY( EditDefaultsOnly, Category = Settings )
	uint32 bAutoCalculateMoveFromPointDistanceTolerance : 1;

	/**
	 * Current goal location where is moving to
	 * it may be location of path point, or action point
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASComponent )
	FVector CurrentGoalLocation;

	/**
	 * Current goal rotation where wants rotate to
	 * it may be rotation of path point, or action point
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASComponent )
	FRotator CurrentGoalRotation;

	/**
	 * Sets current goal location of where AI will want move to
	 * Updates blackboard value as well under key GoalLocation
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetCurrentGoalLocation( const FVector& Location );

	/**
	 * Sets current goal rotation to which AI will rotate to
	 * Updates blackboard value as well under key GoalRotation
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetCurrentGoalRotation( const FRotator& Rotation );

	/**
	 * Returns true if owner is near given location, using MoveFromPointToleranceLimit as distance limit
	 * this function is used to check whether owner didn't move from point while executing it
	 */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASComponent )
	bool IsOwnerAtLocation( const FVector& LocationToCheck );

	/**
	 * Returns distance tolerance limit after which AI will stop executing active point
	 * if it doesn't allow AI to move away from it while being executed
	 */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASComponent )
	float GetMoveFromPointDistanceTolerance();
	/************************************************************************/





	/************************************************************************/
	/*								PATH POINT								*/
	/************************************************************************/
public:
	/** Currently active path point that AI is moving to */
	UPROPERTY( BlueprintReadOnly, Category = Settings )
	ADASPathPoint* ActivePathPoint = nullptr;

	/**
	 * True - AI is moving forward along path points
	 * False - AI is moving backward along path points
	 */
	UPROPERTY( BlueprintReadOnly, EditInstanceOnly, Category = Settings )
	uint32 bIsMovingForwardAlongPath : 1;

	/**
	 * True means that AI finished all action points connected with currently active path point
	 * and now is going back to that path point
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASComponent )
	uint32 bIsReturningToPathPoint : 1;

	/**
	 * Flag that is set when Path Point has changed to new one
	 * It gets reset ( to false ) when owner AI reaches currently active Path Point
	 * It helps to decide how AI should behave when reaches final Path Point and has nowhere else to go
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASComponent )
	uint32 bHasNewPathPoint : 1;

	/**
	 * Finds best matching spot for this AI and takes it
	 * updates variables & blackboard GoalLocation & GoalRotation
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	bool RequestPathPointSpot( FVector& OutLocation, FRotator& OutRotation );

	/** Releases any taken spots that this AI took */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void ReleasePathPointSpot();

	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetIsReturningToPathPoint( bool bNewValue ) { bIsReturningToPathPoint = bNewValue; }

	/** Sets moving forward / backward along path */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetIsMovingForwardAlongPath( bool bNewValue ) { bIsMovingForwardAlongPath = bNewValue; }

	/** Sets flag HasNewPathPoint */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetHasNewPathPoint( bool bNewValue ) { bHasNewPathPoint = bNewValue; }

	/** Sets new path point */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetPathPoint( ADASPathPoint* NewPathPoint );

	/**
	 * Takes next or previous path point ( depending on whether AI is moving forward or backward )
	 * from current active path point, and updates it
	 * Called when AI reaches path point and wants to move to next one
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent, meta = ( ReturnDisplayName = "FoundNewPathPoint" ))
	bool UpdatePathPoint();

	/**
	 * Gets all action points connected with active path point
	 * if there are any, sets them in ActionPointsQueue
	 * Called when AI reaches path point, and wants to execute its action points
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent, meta = ( ReturnDisplayName = "HadAnyActions" ))
	bool FetchPathActionPoints();

protected:
	/** Called when active path point changes */
	UFUNCTION( BlueprintImplementableEvent, Category = DASComponent )
	void PathPointChanged( ADASPathPoint* PreviousPathPoint, ADASPathPoint* NewPathPoint );

	/** Called when active action point changes */
	UFUNCTION( BlueprintImplementableEvent, Category = DASComponent )
	void ActionPointChanged( ADASActionPoint* PreviousActionPoint, ADASActionPoint* NewActionPoint );
	/************************************************************************/





	/************************************************************************/
	/*								ACTION POINT							*/
	/************************************************************************/
public:
	/** Currently active action point that AI is moving to / executing */
	UPROPERTY( BlueprintReadOnly, Category = Settings )
	ADASActionPoint* ActiveActionPoint = nullptr;

	/**
	 * Queue of action points that AI wants to execute
	 * action on index 0 is always first that will be executed
	 * after finishing action it gets removed from queue
	 * Means that this queue keeps only actions active or those that will be executed
	 * never actions that were already executed
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASComponent )
	TArray<ADASActionPoint*> ActionPointsQueue;

	/**
	 * Sets new Action Point
	 * Not exposed to Blueprints on purpose
	 * This function is called automatically inside SetFirstActionPointFromQueue
	 * Don't call it directly unless you have some specific setup that requires it
	 */
	UFUNCTION( Category = DASComponent )
	void SetActionPoint( ADASActionPoint* NewActionPoint );

	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void SetActionPointsQueue( const TArray<ADASActionPoint*>& NewActionPointsQueue ) { ActionPointsQueue = NewActionPointsQueue; }

	/** Removes all actions from queue */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void ClearActionPointsQueue() { ActionPointsQueue.Empty(); }

	/**
	 * Removes given action from queue
	 * Usually called when action is finished and needs to be removed from queue
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent )
	void RemoveActionPointFromQueue( ADASActionPoint* ActionPointToRemove ) { ActionPointsQueue.Remove( ActionPointToRemove ); }

	/**
	 * Gets action from index 0 from queue ( if valid and can run ) and sets it as active action
	 */
	UFUNCTION( BlueprintCallable, Category = DASComponent, meta = ( ReturnDisplayName = "New Action Point" ))
	ADASActionPoint* SetFirstActionPointFromQueue();
	/************************************************************************/




	

	/************************************************************************/
	/*                             REFERENCES                               */
	/************************************************************************/
public:
	/** Returns AI Controller of owner */
	UFUNCTION( BlueprintPure, BlueprintCallable, Category = DASComponent )
	AAIController* GetOwnerAIController();

	/** Returns owner casted as character ( valid only if owner is indeed a character ) */
	UFUNCTION( BlueprintPure, BlueprintCallable, Category = DASComponent )
	ACharacter* GetOwnerAsCharacter();

	/** Returns owner casted as pawn */
	UFUNCTION( BlueprintPure, BlueprintCallable, Category = DASComponent )
	APawn* GetOwnerAsPawn();

protected:
	UPROPERTY()
	AAIController* OwnerAIController;

	UPROPERTY()
	ACharacter* OwnerAsCharacter;

	UPROPERTY()
	APawn* OwnerAsPawn;
	/************************************************************************/




	/************************************************************************/
	/*                         BLACKBOARD KEYS UPDATE                       */
	/************************************************************************/
public:
	/**
	 * Force refresh of all blackboard keys to reflect current data
	 * Called when component gets initialized
	 */
	UFUNCTION( BlueprintCallable, Category = "DASComponent|Blackboard" )
	void RefreshBlackboardKeys();

	/** Updates path point value stored in BBKey */
	UFUNCTION( BlueprintCallable, Category = "DASComponent|Blackboard" )
	void UpdatePathPointBBKey();

	/** Updates action point value stored in BBKey */
	UFUNCTION( BlueprintCallable, Category = "DASComponent|Blackboard" )
	void UpdateActionPointBBKey();

	/** Updates Run Mode value stored in BBKey */
	UFUNCTION( BlueprintCallable, Category = "DASComponent|Blackboard" )
	void UpdateRunModeBBKey();

	/** Updates action selector value stored in BBKey */
	UFUNCTION( BlueprintCallable, Category = "DASComponent|Blackboard" )
	void UpdateActionSelectorBBKey();

	/** Updates action point value stored in BBKey */
	UFUNCTION( BlueprintCallable, Category = "DASComponent|Blackboard" )
	void UpdateCanRunActionPointBBKey( bool bCanRunActionPoint );

	/** Updates action point value stored in BBKey */
	UFUNCTION( BlueprintCallable, Category = "DASComponent|Blackboard" )
	void UpdateCanRunPathPointBBKey( bool bCanRunPathPoint );

	/** Updates action point value stored in BBKey */
	UFUNCTION( BlueprintCallable, Category = "DASComponent|Blackboard" )
	void UpdateIsActionPointTakenBBKey( bool bIsActionPointTaken );
	/************************************************************************/



		

	/************************************************************************/
	/*								DEBUG                                   */
	/************************************************************************/
public:

	UFUNCTION( BlueprintCallable, Category = Settings, meta = ( DevelopmentOnly ) )
	virtual void ValidateData();

	/**
	 * Validates data
	 * For example if has any invalid references
	 */
	UFUNCTION( BlueprintCallable, Category = Settings, meta = ( DevelopmentOnly ) )
	virtual void RefreshInstancedObjects();

	/**
	 * Draws debug info about DAS Component
	 * @param bIsInEditor - true means function is called in editor, true that function is called in runtime ( game is running )
	 * @param bIsSelected - true means that owning actor of this component is selected ( clicked )
	 */
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = Debug )
	void DrawDebug( float DeltaTime, bool bIsInEditor, bool bIsSelected );
	void DrawDebug_Implementation( float DeltaTime, bool bIsInEditor, bool bIsSelected );

protected:
#if WITH_EDITORONLY_DATA
	class UDASVisComponent* DasVisComponent;
#endif
	/************************************************************************/

};



/************************************************************************/
/*                      VISUALIZER COMPONENT                            */
/************************************************************************/

/**
 * Visualizer component used only by DASComponent
 * Takes care of calling its debug logic
 */
UCLASS()
class DYNAMICAISYSTEM_API UDASVisComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UDASVisComponent();

	virtual void TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
	UDASComponent* DASComponent;
};
/************************************************************************/