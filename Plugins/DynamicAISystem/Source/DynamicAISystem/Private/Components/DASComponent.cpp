// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Components/DASComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Utils/DASBPLibrary.h"
#include "GameFramework/Character.h"
#include "Utils/DASDeveloperSettings.h"
#include "Kismet/KismetMathLibrary.h"
#include "Points/DASPathPoint.h"
#include "Objects/DASActionSelector.h"
#include "Points/DASActionPoint.h"
#include "Utils/DASWorldSubsystem.h"
#include "BrainComponent.h"
#include "AIController.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utils/DASInterface.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"


#if WITH_EDITORONLY_DATA
#include "DrawDebugHelpers.h"
#endif
#include "Objects/DASAction.h"




UDASComponent::UDASComponent()
{
	// by default disable tick as it is not needed
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// settings
	bIsMovingForwardAlongPath = true;
	bIsReturningToPathPoint = false;
	bAutoCalculateMoveFromPointDistanceTolerance = true;
	RunMode = EDASRunMode::ExecutePathPoints;

	SetIsReplicatedByDefault( true );
}


void UDASComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST( UDASComponent, bIsInitialized, SharedParams );
}

void UDASComponent::OnRegister()
{
	// create visualization component which will take care of calling debug logic for DASComponent
#if WITH_EDITORONLY_DATA
	AActor* owner = GetOwner();
	if( ( owner != nullptr ) && !IsRunningCommandlet() )
	{
		if( DasVisComponent == nullptr )
		{
			DasVisComponent = NewObject<UDASVisComponent>( owner, NAME_None, RF_Transactional | RF_TextExportTransient );
			DasVisComponent->SetIsVisualizationComponent( true );
			DasVisComponent->CreationMethod = CreationMethod;
			DasVisComponent->RegisterComponentWithWorld( GetWorld() );
			DasVisComponent->DASComponent = this;
		}
	}
#endif

	Super::OnRegister();
}


bool UDASComponent::Init( UBehaviorTree* BehaviorTree )
{
	// check if wasn't already initialized before
	if( bIsInitialized )
		return false;

	AActor* owner = GetOwner();

	// owner of component must be valid
	if( !owner )
		return false;

	// allow to init only on server
	if( !owner->HasAuthority() )
		return false;

	// allow to init only for Pawn or Character
	if( !Cast<APawn>( owner ) )
	{
		UE_LOG( LogDAS, Error, TEXT( "%s must be either Pawn or Character to init DASComponent" ), *owner->GetName() );
		return false;
	}

	// check if owner implements DAS interface
	if( !owner->GetClass()->ImplementsInterface( UDASInterface::StaticClass() ) )
	{
		UE_LOG( LogDAS, Error, TEXT( "%s must implement DASInterface to be able to initialize DASComponent" ), *owner->GetName() );
		return false;
	}


	// behavior tree asset must be valid
	if( !BehaviorTree )
	{
		UE_LOG( LogDAS, Error, TEXT( "Trying to initialize DAS Component for %s with invalid behavior tree" ), *owner->GetName() );
		return false;
	}

	// cache owner AIController
	OwnerAIController = UAIBlueprintHelperLibrary::GetAIController( GetOwner() );

	// make sure AIController is valid
	if( !OwnerAIController )
	{
		UE_LOG( LogDAS, Error, TEXT( "%s has invalid AI Controller" ), *owner->GetName() );
		return false;
	}

	// run behavior tree
	if( OwnerAIController->RunBehaviorTree( BehaviorTree ) )
	{
		// refresh blackboard keys
		RefreshBlackboardKeys();

		// pass value of initial path point to active one
		if( IsValid( InitialPathPoint ) )
		{
			SetPathPoint( InitialPathPoint );
		}

		SetIsInitialized( true );
		return true;
	}

	return false;
}

void UDASComponent::SetIsInitialized( bool bNewInitialized )
{
	if( bIsInitialized != bNewInitialized )
	{
		bIsInitialized = bNewInitialized;
		MARK_PROPERTY_DIRTY_FROM_NAME( UDASComponent, bIsInitialized, this );
		OnRep_IsInitialized();
	}
}

void UDASComponent::OnRep_IsInitialized()
{
	// cache owner as pawn and character to have easier access to them if needed
	OwnerAsPawn = Cast<APawn>( GetOwner() );
	OwnerAsCharacter = Cast<ACharacter>( GetOwner() );

	// call blueprint event & broadcast change
	ReceiveInit();
	OnInitialized.Broadcast();
}




void UDASComponent::SetRunMode( EDASRunMode NewRunMode )
{
	if( RunMode != NewRunMode )
	{
		EDASRunMode previousMode = RunMode;
		RunMode = NewRunMode;

		// reset action points
		if( previousMode == EDASRunMode::ExecuteActionsFromSelector ||
			previousMode == EDASRunMode::ExecutePathPoints )
		{
			ClearActionPointsQueue();
			SetActionPoint( nullptr );
		}

		// update bb key
		UpdateRunModeBBKey();

		// call blueprint event & broadcast change
		RunModeChanged( previousMode, RunMode );
		OnRunModeChanged.Broadcast( previousMode, RunMode );
	}
}

void UDASComponent::SetActionSelector( UDASActionSelector* NewActionSelector )
{
	if( ActionSelector != NewActionSelector )
	{
		ActionSelector = NewActionSelector;

		// if AI was already executing actions from selector, reset them to pick new actions from new selector
		if( RunMode == EDASRunMode::ExecuteActionsFromSelector )
		{
			ClearActionPointsQueue();
			SetActionPoint( nullptr );
		}

		// update bb key
		UpdateActionSelectorBBKey();

		// call blueprint event
		ActionSelectorChanged( ActionSelector );
	}
}


void UDASComponent::SetInitialPathPoint( ADASPathPoint* NewPathPoint )
{
	// if comp was already initialized then set new path point directly
	if( bIsInitialized )
	{
		SetPathPoint( NewPathPoint );
	}
	else
	{
		InitialPathPoint = NewPathPoint;
	}
}

void UDASComponent::Reset()
{
	if( UDASComponent* ComponentCDO = Cast<UDASComponent>( UDASBPLibrary::FindDefaultComponentByClass( GetOwner()->GetClass(), GetClass() ) ) )
	{
		// reset data
		SetIsInitialized( false );
		ClearActionPointsQueue();
		SetActionPoint( nullptr );
		SetPathPoint( nullptr );
		SetRunMode( ComponentCDO->RunMode );
		CurrentGoalLocation = FVector::ZeroVector;
		CurrentGoalRotation = FRotator::ZeroRotator;
		SetIsReturningToPathPoint( false );
		SetHasNewPathPoint( false );
		SetIsMovingForwardAlongPath( ComponentCDO->bIsMovingForwardAlongPath );
		SetPathBehavior( EDASPathBehavior::Undefined );
		ActionSelector = nullptr;
		InitialPathPoint = nullptr;
		OwnerAIController = nullptr;


		// unbind all events
		OnPathPointChanged.Clear();
		OnActionPointChanged.Clear();
		OnPathBehaviorChanged.Clear();
		OnRunModeChanged.Clear();
		OnInitialized.Clear();
	}
}



FDASComponentSnapshot UDASComponent::GetSnapshot()
{
	FDASComponentSnapshot ret;

	ret.bWasMovingForward = bIsMovingForwardAlongPath;
	ret.bHasNewPathPoint = bHasNewPathPoint;
	ret.bWasReturningToPathPoint = bIsReturningToPathPoint;
	ret.RunMode = RunMode;
	ret.OwnerLocation = GetOwner()->GetActorLocation();
	ret.OwnerRotation = GetOwner()->GetActorRotation();

	ret.ActionPointsIdQueue.Reset( ActionPointsQueue.Num() );
	for( ADASActionPoint* actionPoint : ActionPointsQueue )
	{
		if( actionPoint )
			ret.ActionPointsIdQueue.Add( actionPoint->PointId );
	}

	if( ActivePathPoint ) ret.ActivePathPointId = ActivePathPoint->PointId;

	// action selector is an object, which is not placed on the world
	// its class & data is needed to be saved to be able to recreate it on loading
	if( ActionSelector )
	{
		ret.ActionSelectorClass = ActionSelector->GetClass();

		FMemoryWriter memoryWriter( ret.ActionSelectorData, true );
		FDASSaveGameArchive Ar( memoryWriter );
		ActionSelector->Serialize( Ar );
	}

	return ret;
}

void UDASComponent::LoadFromSnapshot( const FDASComponentSnapshot& Snapshot, bool bLoadOwnerTransform /*= true */ )
{
	if( UWorld* world = GetWorld() )
	{
		if( UDASWorldSubsystem* DASSubsystem = world->GetSubsystem<UDASWorldSubsystem>() )
		{
			// Load mode
			SetRunMode( Snapshot.RunMode ); // should call directly to not clear action points

			// Recreate action selector
			UDASActionSelector* selector = nullptr; // this could also clear action points
			if( Snapshot.ActionSelectorClass )
			{
				selector = NewObject<UDASActionSelector>( this, Snapshot.ActionSelectorClass );
				FMemoryReader memoryReader( Snapshot.ActionSelectorData, true );
				FDASSaveGameArchive Ar( memoryReader );
				selector->Serialize( Ar );
			}
			SetActionSelector( selector );

			// Load active path point
			ADASPathPoint* activePathPoint = DASSubsystem->FindPathPointById( Snapshot.ActivePathPointId );
			SetPathPoint( activePathPoint ); // this could also clear action points

			// Reset action point, it will be taken from ActionPointsQueue
			SetActionPoint( nullptr );

			// Reset as it no longer will be needed because data was loaded
			InitialPathPoint = nullptr;

			// Load actions queue
			TArray<ADASActionPoint*> loadedQueue;
			loadedQueue.Reserve( Snapshot.ActionPointsIdQueue.Num() );

			for( const FGuid& actionPointId : Snapshot.ActionPointsIdQueue )
			{
				ADASActionPoint* actionPoint = DASSubsystem->FindActionPointById( actionPointId );
				loadedQueue.Add( actionPoint );
			}
			SetActionPointsQueue( loadedQueue );

			SetIsMovingForwardAlongPath( Snapshot.bWasMovingForward );
			SetIsReturningToPathPoint( Snapshot.bWasReturningToPathPoint );
			SetHasNewPathPoint( Snapshot.bHasNewPathPoint );

			// load transform
			if( bLoadOwnerTransform )
			{
				if( AActor* owner = GetOwner() )
				{
					owner->SetActorLocationAndRotation( Snapshot.OwnerLocation, Snapshot.OwnerRotation );
				}
			}

			// restart logic if is already running ( behavior tree )
			if( GetOwnerAIController() )
			{
				if( UBrainComponent* brain = GetOwnerAIController()->BrainComponent )
				{
					brain->RestartLogic();
				}
			}
		}
	}
}


void UDASComponent::SetPathBehavior( EDASPathBehavior NewPathBehavior )
{
	if( PathBehavior != NewPathBehavior )
	{
		EDASPathBehavior previousBehavior = PathBehavior;
		PathBehavior = NewPathBehavior;
		PathBehaviorChanged( previousBehavior, PathBehavior );
		OnPathBehaviorChanged.Broadcast( previousBehavior, PathBehavior );
	}
}


bool UDASComponent::RequestPathPointSpot( FVector& OutLocation, FRotator& OutRotation )
{
	if( IsValid( ActivePathPoint ) )
	{
		ActivePathPoint->GetPointLocationAndRotation( OutLocation, OutRotation, GetOwner() );

		// cache results
		SetCurrentGoalLocation( OutLocation );
		SetCurrentGoalRotation( OutRotation );

		// update OutLocation after setting goal location because it was projected to nav mesh
		OutLocation = CurrentGoalLocation;

		return true;
	}

	return false;
}

void UDASComponent::ReleasePathPointSpot()
{
	// release path point spot
	if( IsValid( ActivePathPoint ) )
	{
		ActivePathPoint->ReleaseSpot( GetOwner() );
	}
}

void UDASComponent::SetCurrentGoalLocation( const FVector& Location )
{
	CurrentGoalLocation = Location;

	// project location to navigation mesh
	if( GetOwnerAsPawn() )
	{
		FNavLocation projectedNavLocation;
		const FNavAgentProperties& navAgentProps = GetOwnerAsPawn()->GetNavAgentPropertiesRef();
		UNavigationSystemV1* navSystem = Cast<UNavigationSystemV1>( GetWorld()->GetNavigationSystem() );
		if( navSystem->ProjectPointToNavigation( CurrentGoalLocation, projectedNavLocation, INVALID_NAVEXTENT, &navAgentProps ) )
		{
			CurrentGoalLocation = projectedNavLocation.Location;
		}
	}

	if( GetOwnerAIController() )
	{
		// update value in blackboard
		if( UBlackboardComponent* blackboard = GetOwnerAIController()->GetBlackboardComponent() )
		{
			blackboard->SetValueAsVector( UDASBPLibrary::GetBBKeyName_GoalLocation(), CurrentGoalLocation );
		}
	}
}

void UDASComponent::SetCurrentGoalRotation( const FRotator& Rotation )
{
	CurrentGoalRotation = Rotation;

	if( GetOwnerAIController() )
	{
		// update value in blackboard
		if( UBlackboardComponent* blackboard = GetOwnerAIController()->GetBlackboardComponent() )
		{
			blackboard->SetValueAsRotator( UDASBPLibrary::GetBBKeyName_GoalRotation(), CurrentGoalRotation );
		}
	}
}

bool UDASComponent::IsOwnerAtLocation( const FVector& LocationToCheck )
{
	float acceptanceRadius = GetMoveFromPointDistanceTolerance();

	// calculate current distance to given location
	float distanceToLocation = UKismetMathLibrary::Vector_Distance2D( GetOwner()->GetActorLocation(), LocationToCheck );
	return distanceToLocation <= acceptanceRadius;
}

float UDASComponent::GetMoveFromPointDistanceTolerance()
{
	float ret = MoveFromPointDistanceTolerance;

	// try to auto calculate distance tolerance using AI capsule radius
	if( bAutoCalculateMoveFromPointDistanceTolerance && GetOwnerAsCharacter() && GetOwnerAsCharacter()->GetCapsuleComponent() )
	{
		ret = GetOwnerAsCharacter()->GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.5f;
	}

	return ret;
}

void UDASComponent::SetPathPoint( ADASPathPoint* NewPathPoint )
{
	if( NewPathPoint != ActivePathPoint )
	{
		// release path point spot for previous path point before we update it
		ReleasePathPointSpot();

		// cache previous path point and set new one
		ADASPathPoint* previousPathPoint = ActivePathPoint;
		ActivePathPoint = NewPathPoint;

		if( RunMode == EDASRunMode::ExecutePathPoints )
		{
			// reset action points on starting new path point
			ClearActionPointsQueue();
			SetActionPoint( nullptr );
		}

		// reset flag is returning to path
		SetIsReturningToPathPoint( false );

		SetHasNewPathPoint( true );

		// update bb key
		UpdatePathPointBBKey();

		// clear connections to previous point
		if( IsValid( previousPathPoint ) )
		{
			// stop observing condition of previous point
			if( previousPathPoint->ConditionQuery.IsValid() )
			{
				previousPathPoint->ConditionQuery.Instance->OnConditionResultChanged.RemoveDynamic( this, &UDASComponent::UpdateCanRunPathPointBBKey );
			}
		}

		// make connections to new point if its valid
		if( IsValid( NewPathPoint ) )
		{
			// start observing condition of new point
			if( NewPathPoint->ConditionQuery.IsValid() )
			{
				NewPathPoint->ConditionQuery.Instance->OnConditionResultChanged.AddUniqueDynamic( this, &UDASComponent::UpdateCanRunPathPointBBKey );
				UpdateCanRunPathPointBBKey( NewPathPoint->ConditionQuery.Instance->IsConditionFulfilled() );
			}
			// if new point doesn't have any condition then set can run bb key to true
			else
			{
				UpdateCanRunPathPointBBKey( true );
			}

		}
		else
		{
			// reset bb key of can run point, because its not valid
			UpdateCanRunPathPointBBKey( false );
		}

		// broadcast the changes
		PathPointChanged( previousPathPoint, ActivePathPoint );
		OnPathPointChanged.Broadcast( previousPathPoint, ActivePathPoint );
	}
}


bool UDASComponent::UpdatePathPoint()
{
	if( IsValid( ActivePathPoint ) )
	{
		// cache previous path point
		ADASPathPoint* previousPathPoint = ActivePathPoint;

		// get next path point for current moving direction
		ADASPathPoint* newPathPoint = bIsMovingForwardAlongPath ?
			ActivePathPoint->GetNextPathPoint( this ) :
			ActivePathPoint->GetPreviousPathPoint( this );

		// if was found, set new path point
		if( newPathPoint )
		{
			SetPathPoint( newPathPoint );
			return true;
		}

		// otherwise get next path point for reversed direction
		newPathPoint = !bIsMovingForwardAlongPath ?
			ActivePathPoint->GetNextPathPoint( this ) :
			ActivePathPoint->GetPreviousPathPoint( this );

		// if was found, switch direction and set new path point
		if( newPathPoint )
		{
			SetIsMovingForwardAlongPath( !bIsMovingForwardAlongPath );
			SetPathPoint( newPathPoint );
			return true;
		}
	}
	return false;
}

bool UDASComponent::FetchPathActionPoints()
{
	// is path point and path action valid?
	if( IsValid( ActivePathPoint ) && ActivePathPoint->ActionSelector )
	{
		if( ActivePathPoint->PathActionExecutionMethod == EDASPathExecuteMethod::BothWays ||								// both ways actions allowed
			ActivePathPoint->PathActionExecutionMethod == EDASPathExecuteMethod::Forward && bIsMovingForwardAlongPath ||	// only forward actions allowed
			ActivePathPoint->PathActionExecutionMethod == EDASPathExecuteMethod::Backward && !bIsMovingForwardAlongPath )	// only backward actions allowed
		{
			TArray<ADASActionPoint*> actionPoints;
			ActivePathPoint->ActionSelector->GetActionPointsToExecute( actionPoints, this );

			SetActionPointsQueue( actionPoints );
			return actionPoints.Num() > 0;
		}
	}

	// if function got there, it means that getting action points from current path point failed
	ClearActionPointsQueue();
	return false;
}



void UDASComponent::SetActionPoint( ADASActionPoint* NewActionPoint )
{
	if( ActiveActionPoint != NewActionPoint )
	{
		ADASActionPoint* previousActionPoint = ActiveActionPoint;
		ActiveActionPoint = NewActionPoint;

		// update bb key
		UpdateActionPointBBKey();

		// clear connections to previous point
		if( IsValid( previousActionPoint ) )
		{
			// stop observing is taken flag of previous point
			previousActionPoint->OnIsTakenChanged.RemoveDynamic( this, &UDASComponent::UpdateIsActionPointTakenBBKey );

			// stop observing condition of previous point
			if( previousActionPoint->ConditionQuery.IsValid() )
			{
				previousActionPoint->ConditionQuery.Instance->OnConditionResultChanged.RemoveDynamic( this, &UDASComponent::UpdateCanRunActionPointBBKey );
			}

		}

		// make connections to new point if its valid
		if( IsValid( NewActionPoint ) )
		{
			// start observing is taken flag of new point
			NewActionPoint->OnIsTakenChanged.AddUniqueDynamic( this, &UDASComponent::UpdateIsActionPointTakenBBKey );
			UpdateIsActionPointTakenBBKey( NewActionPoint->IsTaken() );

			// start observing condition of new point
			if( NewActionPoint->ConditionQuery.IsValid() )
			{
				NewActionPoint->ConditionQuery.Instance->OnConditionResultChanged.AddUniqueDynamic( this, &UDASComponent::UpdateCanRunActionPointBBKey );
				UpdateCanRunActionPointBBKey( NewActionPoint->ConditionQuery.Instance->IsConditionFulfilled() );
			}
			// if new point doesn't have any condition then set can run bb key to true
			else
			{
				UpdateCanRunActionPointBBKey( true );
			}
		}
		else
		{
			// reset bb key of can run point, because its not valid
			UpdateCanRunActionPointBBKey( false );
			UpdateIsActionPointTakenBBKey( false );
		}


		ActionPointChanged( previousActionPoint, ActiveActionPoint );
		OnActionPointChanged.Broadcast( previousActionPoint, ActiveActionPoint );
	}
}


ADASActionPoint* UDASComponent::SetFirstActionPointFromQueue()
{
	ADASActionPoint* selectedActionPoint = nullptr;

	// loop through actions queue as long as valid and runnable point is found
	while( selectedActionPoint == nullptr && ActionPointsQueue.Num() > 0 )
	{
		ADASActionPoint* checkedActionPoint = ActionPointsQueue[ 0 ];
		if( checkedActionPoint &&
			checkedActionPoint->CanRun() &&
			!checkedActionPoint->IsTaken() )
		{
			// found available action point, cache it
			selectedActionPoint = checkedActionPoint;
			break;
		}
		else
		{
			// action point at this index is not valid or can't run, remove it to check next one
			ActionPointsQueue.RemoveAt( 0 );
		}
	}

	// update action point with found one ( or null if none was found )
	SetActionPoint( selectedActionPoint );
	return selectedActionPoint;
}



AAIController* UDASComponent::GetOwnerAIController()
{
	if( IsValid( OwnerAIController ) )
		return OwnerAIController;

	OwnerAIController = UAIBlueprintHelperLibrary::GetAIController( GetOwner() );
	return OwnerAIController;
}

ACharacter* UDASComponent::GetOwnerAsCharacter()
{
	if( IsValid( OwnerAsCharacter ) )
		return OwnerAsCharacter;

	OwnerAsCharacter = Cast<ACharacter>( GetOwner() );
	return OwnerAsCharacter;
}

APawn* UDASComponent::GetOwnerAsPawn()
{
	if( IsValid( OwnerAsPawn ) )
		return OwnerAsPawn;

	OwnerAsPawn = Cast<APawn>( GetOwner() );
	return OwnerAsPawn;
}


/************************************************************************/
/*                         BLACKBOARD KEYS		                        */
/************************************************************************/

void UDASComponent::RefreshBlackboardKeys()
{
	UpdateRunModeBBKey();
	UpdateActionSelectorBBKey();
	UpdatePathPointBBKey();
	UpdateActionPointBBKey();

	// if action point is valid, get its data and pass to blackboard
	if( IsValid( ActiveActionPoint ) )
	{
		UpdateCanRunActionPointBBKey( ActiveActionPoint->CanRun() );
		UpdateIsActionPointTakenBBKey( ActiveActionPoint->IsTaken() );
	}
	// if action point is NOT valid, then set bb keys to false
	else
	{
		UpdateCanRunActionPointBBKey( false );
		UpdateIsActionPointTakenBBKey( false );
	}

	// if path point is valid, get its data and pass to blackboard
	if( IsValid( ActivePathPoint ) )
	{
		UpdateCanRunPathPointBBKey( ActivePathPoint->CanRun() );
	}
	// if path point is NOT valid, then set bb keys to false
	else
	{
		UpdateCanRunPathPointBBKey( false );
	}
}

void UDASComponent::UpdatePathPointBBKey()
{
	if( OwnerAIController )
	{
		if( UBlackboardComponent* bb = OwnerAIController->GetBlackboardComponent() )
		{
			bb->SetValueAsObject( UDASBPLibrary::GetBBKeyName_PathPoint(), ActivePathPoint );
		}
	}
}

void UDASComponent::UpdateActionPointBBKey()
{
	if( OwnerAIController )
	{
		if( UBlackboardComponent* bb = OwnerAIController->GetBlackboardComponent() )
		{
			bb->SetValueAsObject( UDASBPLibrary::GetBBKeyName_ActionPoint(), ActiveActionPoint );
		}
	}
}

void UDASComponent::UpdateRunModeBBKey()
{
	if( OwnerAIController )
	{
		if( UBlackboardComponent* bb = OwnerAIController->GetBlackboardComponent() )
		{
			bb->SetValueAsEnum( UDASBPLibrary::GetBBKeyName_RunMode(), ( uint8 )RunMode );
		}
	}
}

void UDASComponent::UpdateActionSelectorBBKey()
{
	if( OwnerAIController )
	{
		if( UBlackboardComponent* bb = OwnerAIController->GetBlackboardComponent() )
		{
			bb->SetValueAsObject( UDASBPLibrary::GetBBKeyName_ActionSelector(), ActionSelector );
		}
	}
}

void UDASComponent::UpdateCanRunActionPointBBKey( bool bCanRunActionPoint )
{
	if( OwnerAIController )
	{
		if( UBlackboardComponent* bb = OwnerAIController->GetBlackboardComponent() )
		{
			bb->SetValueAsBool( UDASBPLibrary::GetBBKeyName_CanRunActionPoint(), bCanRunActionPoint );
		}
	}
}

void UDASComponent::UpdateCanRunPathPointBBKey( bool bCanRunPathPoint )
{
	if( OwnerAIController )
	{
		if( UBlackboardComponent* bb = OwnerAIController->GetBlackboardComponent() )
		{
			bb->SetValueAsBool( UDASBPLibrary::GetBBKeyName_CanRunPathPoint(), bCanRunPathPoint );
		}
	}
}

void UDASComponent::UpdateIsActionPointTakenBBKey( bool bIsActionPointTaken )
{
	if( OwnerAIController )
	{
		if( UBlackboardComponent* bb = OwnerAIController->GetBlackboardComponent() )
		{
			bb->SetValueAsBool( UDASBPLibrary::GetBBKeyName_IsActionPointTaken(), bIsActionPointTaken );
		}
	}
}







/************************************************************************/
/*                           DEBUG LOGIC				                */
/************************************************************************/

void UDASComponent::DrawDebug_Implementation( float DeltaTime, bool bIsInEditor, bool bIsSelected )
{
#if WITH_EDITORONLY_DATA
	UWorld* world = GetWorld();
	if( !world )
		return;

	AActor* owner = GetOwner();
	if( !owner )
		return;

	// EDITOR
	if( bIsInEditor )
	{
		if( RunMode == EDASRunMode::ExecutePathPoints )
		{
			// draw line to initial path point
			if( IsValid( InitialPathPoint ) )
			{
				FColor color = UDASDeveloperSettings::Get()->PathPointsDebugColor;
				DrawDebugDirectionalArrow( world, owner->GetActorLocation(), InitialPathPoint->GetActorLocation(), 150.f, color, false, DeltaTime, 0, 1.f );;
			}
		}
		else if( RunMode == EDASRunMode::ExecuteActionsFromSelector )
		{
			// pass debug call to action selector so it can handle it by itself
			if( IsValid( ActionSelector ) )
			{
				ActionSelector->DrawDebug( DeltaTime, owner, bIsInEditor );
			}
		}
	}
	// RUNTIME
	else
	{
		const bool bAnimateArrows = UDASDeveloperSettings::Get()->bAnimatePathArrows && !bIsInEditor;
		float arrowAlpha = 0.5f;
		if( bAnimateArrows )
		{
			arrowAlpha = UKismetMathLibrary::Fraction( world->GetTimeSeconds() / 2.f );
		}

		// draw debug line to active path point
		if( IsValid( ActivePathPoint ) && RunMode == EDASRunMode::ExecutePathPoints )
		{
			FVector pathPointStartLocation = owner->GetActorLocation();
			FVector endLoc = ( PathBehavior == EDASPathBehavior::MovingToPathPoint ) ? CurrentGoalLocation : ActivePathPoint->GetActorLocation();
			FVector arrowEndLoc = UKismetMathLibrary::VLerp( pathPointStartLocation, endLoc, arrowAlpha );
			FColor color = UDASDeveloperSettings::Get()->PathPointsDebugColor;

			DrawDebugLine( world, owner->GetActorLocation(), endLoc, UDASDeveloperSettings::Get()->PathPointsDebugColor, false, DeltaTime, 0, 1.f );
			DrawDebugDirectionalArrow( world, pathPointStartLocation, arrowEndLoc, 150.f, color, false, DeltaTime, 0, 1.f );
		}

		// draw debug line to active action point
		if( IsValid( ActiveActionPoint ) )
		{
			FVector startLoc = owner->GetActorLocation();
			FVector endLoc = ActiveActionPoint->GetActorLocation();
			FVector arrowEndLoc = UKismetMathLibrary::VLerp( startLoc, endLoc, arrowAlpha );
			FColor color = UDASDeveloperSettings::Get()->ActionPointsDebugColor;

			DrawDebugLine( world, startLoc, endLoc, UDASDeveloperSettings::Get()->ActionPointsDebugColor, false, DeltaTime, 0, 1.f );
			DrawDebugDirectionalArrow( world, startLoc, arrowEndLoc, 150.f, color, false, DeltaTime, 0, 1.f );
		}

		// draw text near owner with basic info, like his name & what is he doing
		FColor textColor = FLinearColor( 0.f, 220.f, 224.f, 1.f ).ToFColor( false );
		FVector textLocation = FVector( 0.f, 0.f, 50.f ) + owner->GetActorLocation();

		FString debugOwnerInfo = owner->GetActorNameOrLabel();
		debugOwnerInfo += LINE_TERMINATOR;
		debugOwnerInfo += UDASBPLibrary::EnumToString( TEXT( "EDASPathBehavior" ), ( uint32 )PathBehavior );

		if( PathBehavior == EDASPathBehavior::MovingToPathPoint || PathBehavior == EDASPathBehavior::ReturningToPathPoint )
		{
			debugOwnerInfo += ( bIsMovingForwardAlongPath ) ? TEXT( " (Fwd)" ) : TEXT( " (Bwd)" );
		}

		// add current action index if AI is executing action points
		if( PathBehavior == EDASPathBehavior::ExecutingActionPoint && ActiveActionPoint )
		{
			int32 activeActionIndex = ActiveActionPoint->GetCurrentActionIndex();
			debugOwnerInfo += LINE_TERMINATOR;
			FString actionName = ( ActiveActionPoint->Actions.IsValidIndex( activeActionIndex ) ) ?
				ActiveActionPoint->Actions[ activeActionIndex ]->GetClass()->GetName() : TEXT( "Invalid Action" );

			debugOwnerInfo += FString::Printf( TEXT( "[%d] %s" ), activeActionIndex, *actionName );
		}

		DrawDebugString( world, textLocation, debugOwnerInfo, nullptr, textColor, DeltaTime );
	}
#endif
}


UDASVisComponent::UDASVisComponent()
{
	bTickInEditor = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = UDASDeveloperSettings::Get()->DebugTickInterval;
}

void UDASVisComponent::TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

#if WITH_EDITORONLY_DATA
	if( TickType == ELevelTick::LEVELTICK_ViewportsOnly || DAS_Debug.GetValueOnGameThread() )
	{
		if( DASComponent )
		{
			if( AActor* owner = GetOwner() )
			{
				if( owner->IsTemplate( RF_ArchetypeObject | RF_ClassDefaultObject ) )
					return;

				if( UWorld* world = GetWorld() )
				{
					// show only for selected actors, unless game is running
					if( !owner->IsSelected() && !world->IsGameWorld() )
						return;

					// ignore clients, show debug only on server
					if( world->IsGameWorld() && !owner->HasAuthority() )
						return;

					if( world->ViewLocationsRenderedLastFrame.Num() > 0 )
					{
						// show debug only if player is within view range
						float distanceToCamera = UKismetMathLibrary::Vector_Distance( owner->GetActorLocation(), world->ViewLocationsRenderedLastFrame[ 0 ] );
						if( distanceToCamera < UDASDeveloperSettings::Get()->DrawDebugMaxDistance )
						{
							DASComponent->DrawDebug( PrimaryComponentTick.TickInterval, !world->IsGameWorld(), owner->IsSelected() );
						}
					}
				}
			}
		}
	}
#endif
}

void UDASComponent::ValidateData()
{
	if( ActionSelector )
	{
		ActionSelector->ValidateData();
	}
}

void UDASComponent::RefreshInstancedObjects()
{
	if( ActionSelector )
	{
		ActionSelector = DuplicateObject( ActionSelector, this );
	}
}