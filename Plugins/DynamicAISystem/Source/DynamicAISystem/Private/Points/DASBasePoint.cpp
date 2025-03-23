// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#include "Points/DASBasePoint.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/DASDeveloperSettings.h"
#include "Components/BillboardComponent.h"

#if WITH_EDITORONLY_DATA
#include "DrawDebugHelpers.h"
#endif


ADASBasePoint::ADASBasePoint()
{
	// by default ticking is disabled
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// create scene component which will be used as a root
	SceneComponent = CreateDefaultSubobject<USceneComponent>( TEXT( "SceneComponent" ) );
	SetRootComponent( SceneComponent );

#if WITH_EDITORONLY_DATA
	// create billboard component that is Editor-Only and is used as visual representation of point in the world
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>( TEXT( "BillboardComponent" ) );
	if( BillboardComponent )
	{
		BillboardComponent->SetupAttachment( GetRootComponent() );
	}

	// create visualizer component, which will run DrawDebug logic but only in Editor
	if( PointVisComponent == nullptr )
	{
		PointVisComponent = CreateDefaultSubobject<UDASPointVisComponent>( TEXT( "VisualizerComponent" ), true );
		PointVisComponent->SetupAttachment( GetRootComponent() );
		PointVisComponent->SetIsVisualizationComponent( true );
	}
#endif
}

void ADASBasePoint::BeginPlay()
{
	Super::BeginPlay();

	// initialize condition query right after game starts
	ConditionQuery.Initialize( this );

	ValidateData();
}


void ADASBasePoint::PostDuplicate( EDuplicateMode::Type DuplicateMode )
{
#if WITH_EDITOR
	// on duplication of point ( alt-drag or copy paste )
	// generate new Id for duplicated point to make sure it will be different than source point that this one was copied from
	if( DuplicateMode == EDuplicateMode::Normal )
	{
		// ignore CDO & Archetypes
		if( !IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) )
		{
			PointId = FGuid::NewGuid();
		}
	}
#endif
	Super::PostDuplicate( DuplicateMode );
}

void ADASBasePoint::PostActorCreated()
{
	Super::PostActorCreated();
#if WITH_EDITOR
	// generate new Id for newly created point ( placed on the level or spawned in runtime )

	// ignore CDO & Archetypes
	if( !IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) )
	{
		// don't change Id if it was already generated before, it must remain the same for entire lifetime of point
		if( PointId.IsValid() == false )
		{
			PointId = FGuid::NewGuid();
		}
	}
#endif
}


bool ADASBasePoint::CanRun_Implementation()
{
	// default implementation of checking if point can be used by AI
	// only checks condition query
	return ConditionQuery.IsConditionFulfilled();
}


void ADASBasePoint::DrawDebug( float DeltaTime, bool bIsInEditor )
{
#if WITH_EDITORONLY_DATA
	if( UWorld* world = GetWorld() )
	{
		// call draw debug on condition query ( only if there is any )
		if( ConditionQuery.IsValid() )
		{
			ConditionQuery.Instance->DrawDebug( DeltaTime, this, !world->IsGameWorld() );
		}

		// call blueprint version of draw debug to allow implement debug logic in blueprints
		K2_DrawDebug( DeltaTime, !world->IsGameWorld() );
	}
#endif
}

void ADASBasePoint::ValidateData_Implementation()
{
	// call validate data on condition so it may also check itself
	// if has any invalid references, bad data etc.
	if( ConditionQuery.IsValid() )
	{
		ConditionQuery.Instance->ValidateData();
	}
}


/************************************************************************/
/*					DEBUG VISUALIZER COMPONENT                          */
/************************************************************************/

UDASPointVisComponent::UDASPointVisComponent()
{
	// allow ticking in editor on visualizer component used by Points
	bTickInEditor = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = UDASDeveloperSettings::Get()->DebugTickInterval;
}

void UDASPointVisComponent::TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

#if WITH_EDITORONLY_DATA
	// always draw debug in editor, in runtime only if console variable DAS_Debug is set to true
	if( TickType == ELevelTick::LEVELTICK_ViewportsOnly || DAS_Debug.GetValueOnGameThread() )
	{
		// get reference to owning point
		if( ADASBasePoint* point = Cast<ADASBasePoint>( GetOwner() ) )
		{
			// don't draw debug in CDO & Archetypes
			if( point->IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) )
				return;

			if( UWorld* world = GetWorld() )
			{
				// check if this is runtime or editor
				const bool bIsRuntime = world->IsGameWorld();

				// ignore clients, show debug only on server, as AI is mostly running only on server side
				if( bIsRuntime && !point->HasAuthority() )
					return;

				// show billboard comp if debugging is enabled
				if( bIsRuntime && point->BillboardComponent )
				{
					point->BillboardComponent->SetHiddenInGame( false, true );
				}

				if( world->ViewLocationsRenderedLastFrame.Num() > 0 )
				{
					// show debug only if player view is in range ( to not draw debug on end of the world when we are not there )
					float distanceToCamera = UKismetMathLibrary::Vector_Distance( point->GetActorLocation(), world->ViewLocationsRenderedLastFrame[ 0 ] );
					if( distanceToCamera < UDASDeveloperSettings::Get()->DrawDebugMaxDistance )
					{
						point->DrawDebug( PrimaryComponentTick.TickInterval, !bIsRuntime );
					}
				}
			}
		}
	}
	// if debugging is disabled, hide billboard comp
	else if( ADASBasePoint* point = Cast<ADASBasePoint>( GetOwner() ) )
	{
		if( point->BillboardComponent )
		{
			point->BillboardComponent->SetHiddenInGame( true, true );
		}
	}
#endif
}

void ADASBasePoint::RefreshInstancedObjects()
{
	// refresh condition query instance object
	if( ConditionQuery.IsValid() )
	{
		ConditionQuery.Instance = DuplicateObject( ConditionQuery.Instance, this );
	}
}

void ADASBasePoint::DrawDebugPoint( UWorld* world, const FVector& SpotLocation, const FRotator& SpotRotation, float DeltaTime )
{
	// helper function which shows point, either an arrow ( if point uses rotation ) or a sphere if it doesn't
#if WITH_EDITORONLY_DATA
	if( bRotateToPoint )
	{
		DrawDebugDirectionalArrow( world, SpotLocation, SpotLocation + ( SpotRotation.Vector() * 50 ), 100.f, FColor::Blue, false, DeltaTime, 0, 2.f );
	}
	else
	{
		DrawDebugSphere( world, SpotLocation, 10.f, 6, UDASDeveloperSettings::Get()->PathPointsDebugColor, false, DeltaTime, 0, 1.f );
	}
#endif
}


