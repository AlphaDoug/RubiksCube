// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Points/DASActionPoint.h"
#include "Components/DASComponent.h"
#include "Utils/DASWorldSubsystem.h"
#include "Utils/DASBPLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"



ADASActionPoint::ADASActionPoint()
{
	// tick settings
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// network settings
	bReplicates = true;
	NetDormancy = ENetDormancy::DORM_Initial;
	NetUpdateFrequency = 10.f;

	// settings
	bIsExecuting = false;
	bIsTaken = false;
	bCancelExecutionOnConditionFail = true;
	bUsePreciseLocation = true;
	bInterruptActionOnAIMoved = true;
}

void ADASActionPoint::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST( ADASActionPoint, bIsExecuting, SharedParams );
	DOREPLIFETIME_WITH_PARAMS_FAST( ADASActionPoint, bIsTaken, SharedParams );
}

void ADASActionPoint::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Register point to DAS Subsystem right after initializing components
	// to make sure it will be ready to use before actors receive BeginPlay call
	if( UWorld* world = GetWorld() )
	{
		if( world->IsGameWorld() )
		{
			if( UDASWorldSubsystem* DASSubsystem = world->GetSubsystem<UDASWorldSubsystem>() )
			{
				DASSubsystem->AddActionPoint( this );
			}
		}
	}
}


void ADASActionPoint::RefreshInstancedObjects()
{
	Super::RefreshInstancedObjects();

	// refresh actions objects
	for( int32 i = 0; i < Actions.Num() - 1; i++ )
	{
		if( Actions[ i ] )
		{
			Actions[ i ] = DuplicateObject( Actions[i], this );
		}
	}
}

void ADASActionPoint::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	if( EndPlayReason == EEndPlayReason::Destroyed || EndPlayReason == EEndPlayReason::RemovedFromWorld )
	{
		// Unregister action point from DASSubsystem
		if( UWorld* world = GetWorld() )
		{
			if( UDASWorldSubsystem* DASSubsystem = world->GetSubsystem<UDASWorldSubsystem>() )
			{
				DASSubsystem->RemoveActionPoint( this );
			}
		}
	}

	Super::EndPlay( EndPlayReason );
}


bool ADASActionPoint::CanRun_Implementation()
{
	// parent implementation checks only if condition query is fulfilled
	// in action point, before checking condition, firstly check cooldown
	if( IsOnCooldown() )
	{
		return false;
	}

	return Super::CanRun_Implementation();
}


void ADASActionPoint::AsyncExecute( UDASComponent* DASComponent, FAsyncExecutionFinishedWithResultDelegate FinishExecutionDelegate )
{
	// allow execution only on Server
	if( IsExecuting() == false && HasAuthority() )
	{
		ActiveFinishExecutionDelegate = FinishExecutionDelegate;
		Execute( DASComponent );
	}
	// if already executing or for clients always return fail result
	else
	{
		if( FinishExecutionDelegate.IsBound() )
			FinishExecutionDelegate.ExecuteIfBound( EDASExecutionResult::Failed );
	}
}

void ADASActionPoint::Execute( UDASComponent* DASComponent )
{
	// allow execution only on Server
	if( IsExecuting() == false && HasAuthority() )
	{
		ActiveDASComponent = DASComponent;

		SetIsExecuting( true );

		// call OnRep event manually on server, on clients it will be called through replication
		OnRep_IsExecuting();

		ReceiveExecute( ActiveDASComponent );
	}
}

void ADASActionPoint::FinishExecute( bool bSuccess /*= true */ )
{
	if( IsExecuting() )
	{
		EDASExecutionResult result = bSuccess ? EDASExecutionResult::Success : EDASExecutionResult::Failed;

		// stop all delays/timers running on this action point
		UDASBPLibrary::ClearTimersAndDelays( this );

		// reset flag IsExecuting
		SetIsExecuting( false );

		ReceiveFinishExecute( result );

		// apply cooldown if action was finished with success
		if( bSuccess ) ApplyCooldown();

		ActiveDASComponent = nullptr;

		// call OnRep event manually on server, on clients it will be called through replication
		OnRep_IsExecuting();

		if( ActiveFinishExecutionDelegate.IsBound() )
			ActiveFinishExecutionDelegate.ExecuteIfBound( result );
	}
}

void ADASActionPoint::SetIsExecuting( bool bNewExecuting )
{
	if( bIsExecuting != bNewExecuting )
	{
		bIsExecuting = bNewExecuting;
		MARK_PROPERTY_DIRTY_FROM_NAME( ADASActionPoint, bIsExecuting, this );
		ForceNetUpdate();
	}
}

void ADASActionPoint::OnRep_IsExecuting()
{
	IsExecutingValueChanged( bIsExecuting );
	OnIsExecutingChanged.Broadcast( bIsExecuting );
}


void ADASActionPoint::ApplyCooldown()
{
	if( Cooldown > 0.f )
	{
		if( UWorld* world = GetWorld() )
		{
			float currentTime = world->GetTimeSeconds();
			CooldownEndWorldTime = currentTime + Cooldown;
		}
	}
}

void ADASActionPoint::ApplyCustomCooldown( float CooldownToApply )
{
	if( CooldownToApply > 0.f )
	{
		if( UWorld* world = GetWorld() )
		{
			float currentTime = world->GetTimeSeconds();
			CooldownEndWorldTime = currentTime + CooldownToApply;
		}
	}
}

bool ADASActionPoint::IsOnCooldown() const
{
	if( UWorld* world = GetWorld() )
	{
		float currentTime = world->GetTimeSeconds();
		return currentTime < CooldownEndWorldTime;
	}

	return false;
}


float ADASActionPoint::GetCooldownRemainingTime() const
{
	if( const UWorld* world = GetWorld() )
	{
		// calculate remaining cooldown time base on current world time & CooldownEndWorldTime
		float currentTime = world->GetTimeSeconds();
		if( CooldownEndWorldTime > currentTime )
		{
			return CooldownEndWorldTime - currentTime;
		}
	}
	return 0.f;
}


void ADASActionPoint::SetIsTaken( bool bNewIsTaken )
{
	if( bIsTaken != bNewIsTaken )
	{
		bIsTaken = bNewIsTaken;
		MARK_PROPERTY_DIRTY_FROM_NAME( ADASActionPoint, bIsTaken, this );
		OnRep_IsTaken();
		ForceNetUpdate();
	}
}

void ADASActionPoint::OnRep_IsTaken()
{
	IsTakenChanged( bIsTaken );
	OnIsTakenChanged.Broadcast( bIsTaken );
}


void ADASActionPoint::DrawDebug( float DeltaTime, bool bIsInEditor )
{
	Super::DrawDebug( DeltaTime, bIsInEditor );

#if WITH_EDITORONLY_DATA

	if( UWorld* world = GetWorld() )
	{
		DrawDebugPoint( world, FVector( 0.f, 0.f, 20.f ) + GetActorLocation(), GetActorRotation(), DeltaTime );
	}
#endif
}