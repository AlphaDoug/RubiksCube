// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Points/DASPathPoint.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/DASDeveloperSettings.h"
#include "Objects/DASActionSelector.h"
#include "Utils/DASWorldSubsystem.h"
#include "Objects/DASPathSolver.h"

#if WITH_EDITORONLY_DATA
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#endif




ADASPathPoint::ADASPathPoint()
{
	// settings
	bCanMoveForward = true;
	bCanMoveBackward = true;
	bReturnToPathPointAfterExecutingActions = false;
}

void ADASPathPoint::PostInitializeComponents()
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
				DASSubsystem->AddPathPoint( this );
			}
		}
	}
}


void ADASPathPoint::RefreshInstancedObjects()
{
	Super::RefreshInstancedObjects();

	// refresh action selector object
	if( ActionSelector )
	{
		ActionSelector = DuplicateObject( ActionSelector, this );
	}

	// refresh path solver objects
	if( NextPathPointSolver )
	{
		NextPathPointSolver = DuplicateObject( NextPathPointSolver, this );
	}
	if( PreviousPathPointSolver )
	{
		PreviousPathPointSolver = DuplicateObject( PreviousPathPointSolver, this );
	}

}




void ADASPathPoint::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	if( EndPlayReason == EEndPlayReason::Destroyed || EndPlayReason == EEndPlayReason::RemovedFromWorld )
	{
		// Unregister this path point from DASSubsystem
		if( UWorld* world = GetWorld() )
		{
			if( UDASWorldSubsystem* DASSubsystem = world->GetSubsystem<UDASWorldSubsystem>() )
			{
				DASSubsystem->RemovePathPoint( this );
			}
		}
	}

	Super::EndPlay( EndPlayReason );
}


ADASPathPoint* ADASPathPoint::GetNextPathPoint( UDASComponent* DASComponent )
{
	// check if there are any next path points and if are enabled
	if( bCanMoveForward && NextPathPoints.Num() > 0 )
	{
		// if solver obj is valid, use it to select next path point
		if( NextPathPointSolver )
		{
			return NextPathPointSolver->SelectPathPoint( DASComponent, NextPathPoints );
		}
		// if solver is not valid, then pick next point randomly from those that can run
		else
		{
			// filter only those points that can run
			TArray< ADASPathPoint* > availablePathPoints;
			availablePathPoints.Reserve( NextPathPoints.Num() );

			for( ADASPathPoint* pathPoint : NextPathPoints )
			{
				if( pathPoint )
				{
					if( pathPoint->CanRun() )
					{
						availablePathPoints.Add( pathPoint );
					}
				}
			}

			// if there are any available points, pick one of them randomly
			if( availablePathPoints.Num() > 0 )
			{
				int32 randomIdx = UKismetMathLibrary::RandomIntegerInRange( 0, availablePathPoints.Num() - 1 );
				return availablePathPoints[ randomIdx ];
			}
		}
	}

	return nullptr;
}

ADASPathPoint* ADASPathPoint::GetPreviousPathPoint( UDASComponent* DASComponent )
{
	// check if there are any previous path points and if are enabled
	if( bCanMoveBackward && PreviousPathPoints.Num() > 0 )
	{
		// if solver obj is valid, use it to select previous path point
		if( PreviousPathPointSolver )
		{
			return PreviousPathPointSolver->SelectPathPoint( DASComponent, PreviousPathPoints );
		}
		// if solver is not valid, then pick previous point randomly from those that can run
		else
		{
			// filter only those points that can run
			TArray< ADASPathPoint* > availablePathPoints;
			availablePathPoints.Reserve( PreviousPathPoints.Num() );

			for( ADASPathPoint* pathPoint : PreviousPathPoints )
			{
				if( pathPoint )
				{
					if( pathPoint->CanRun() )
					{
						availablePathPoints.Add( pathPoint );
					}
				}
			}

			// if there are any available points, pick one of them randomly
			if( availablePathPoints.Num() > 0 )
			{
				int32 randomIdx = UKismetMathLibrary::RandomIntegerInRange( 0, availablePathPoints.Num() - 1 );
				return availablePathPoints[ randomIdx ];
			}
		}
	}

	return nullptr;
}

void ADASPathPoint::GetPointLocationAndRotation( FVector& OutLocation, FRotator& OutRotation, AActor* Querier )
{
	// if there are no spots, simply return this point location & rotation
	if( Spots.Num() == 0 )
	{
		OutLocation = GetActorLocation();
		OutRotation = GetActorRotation();
	}
	// otherwise choose one of spots and return its location & rotation
	else
	{
		FDASSpot spot;
		RequestSpot( spot, Querier );

		OutLocation = GetActorTransform().TransformPosition( spot.Transform.GetLocation() );
		OutRotation = spot.Transform.GetRotation().Rotator();
	}
}

void ADASPathPoint::RequestSpot( FDASSpot& OutSpot, AActor* Querier )
{
	// make sure there are any spots
	if( Spots.Num() == 0 )
		return;

	// if this Querier had any spot taken, release it before taking new one
	ReleaseSpot( Querier );

	// get querier location if its valid, or use this point location if its not valid
	FVector querierLocation = IsValid( Querier ) ? Querier->GetActorLocation() : GetActorLocation();

	FDASSpot* closestFreeSpot = nullptr;
	float closestDistance = FLT_MAX;

	// find free closest spot
	for( FDASSpot& spot : Spots )
	{
		if( spot.IsFree() )
		{
			// convert spot to world location, cuz they are stored in array as local locations
			FVector spotWorldLoc = GetActorTransform().TransformPosition( spot.Transform.GetLocation() );

			float currentDistance = UKismetMathLibrary::Vector_DistanceSquared( spotWorldLoc, querierLocation );
			if( currentDistance < closestDistance )
			{
				// update closest spot
				closestDistance = currentDistance;
				closestFreeSpot = &spot;
			}
		}
	}

	// take found spot
	if( closestFreeSpot )
	{
		closestFreeSpot->TakeSpot( Querier );
		OutSpot = *closestFreeSpot;
	}
	// if not free spot was found then return 
	else
	{
		OutSpot = Spots[ 0 ];
	}
}



void ADASPathPoint::ReleaseSpot( AActor* Querier )
{
	// check if there is any point taken by given Querier and release it
	if( FDASSpot* foundSpot = Spots.FindByKey( Querier ) )
	{
		foundSpot->FreeSpot();
	}
}







/************************************************************************/
/*                      EDITOR ONLY FUNCTIONS                           */
/************************************************************************/
void ADASPathPoint::GenerateSpots()
{
#if WITH_EDITORONLY_DATA
	// make transaction to be able to undo that change with ctrl+Z
	UKismetSystemLibrary::BeginTransaction( TEXT( "GenerateSpots" ), FText(), nullptr );
	UKismetSystemLibrary::TransactObject( this );

	// clear old spots before generating new ones
	Spots.Empty();

	// create new spots
	for( int32 i = 0; i < NumOfSpotsToGenerate; i++ )
	{
		FDASSpot spot;

		spot.Transform.SetRotation( GetActorRotation().Quaternion() );

		// put first spot on actor location
		if( i == 0 )
		{
			spot.Transform.SetLocation( FVector( 0.f ) );
		}
		// 2nd spot on right side
		else if( i == 1 )
		{
			FVector dir = GetActorRightVector();
			FVector newLocation = dir * 125.f;
			spot.Transform.SetLocation( newLocation );
		}
		// 3rd spot on left side
		else if( i == 2 )
		{
			FVector dir = GetActorRightVector() * -1.f;
			FVector newLocation = dir * 125.f;
			spot.Transform.SetLocation( newLocation );
		}
		// next spots randomly
		else
		{
			// random 2D direction
			FVector dir = UKismetMathLibrary::RandomUnitVector().GetSafeNormal2D();
			float distance = UKismetMathLibrary::RandomFloatInRange( 100.f, 150.f );
			FVector newLocation = dir * distance;
			spot.Transform.SetLocation( newLocation );
		}
		Spots.Add( spot );
	}

	UKismetSystemLibrary::EndTransaction();
#endif
}


void ADASPathPoint::ClearSpots()
{
#if WITH_EDITORONLY_DATA
	if( Spots.Num() == 0 )
		return;

	// make transaction to be able to undo that change with ctrl+Z
	UKismetSystemLibrary::BeginTransaction( TEXT( "ClearSpots" ), FText(), nullptr );
	UKismetSystemLibrary::TransactObject( this );

	Spots.Empty();

	UKismetSystemLibrary::EndTransaction();
#endif
}


void ADASPathPoint::PostActorCreated()
{
	Super::PostActorCreated();

#if WITH_EDITORONLY_DATA
	// ignore CDO & Archetypes
	if( !IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) && SelfName.IsEmpty() )
	{
		// ignore names with REINST
		if( GetName().Contains( TEXT( "REINST" ) ) == false )
		{
			SelfName = GetName();
		}
	}
#endif
}

void ADASPathPoint::PostRename( UObject* OldOuter, const FName OldName )
{
	Super::PostRename( OldOuter, OldName );

#if WITH_EDITORONLY_DATA
	// ignore CDO & Archetypes
	if( IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) )
	{
		SelfName.Empty();
		return;
	}

	// update cached self name after actor is renamed
	if( SelfName.IsEmpty() || SelfName.Equals( OldName.ToString() ) )
	{
		// ignore names with REINST
		if( GetName().Contains( TEXT( "REINST" ) ) == false )
		{
			SelfName = GetName();
		}
	}
#endif
}


void ADASPathPoint::PostDuplicate( EDuplicateMode::Type DuplicateMode )
{
	Super::PostDuplicate( DuplicateMode );

#if WITH_EDITORONLY_DATA
	// Ignore CDO and Archetype
	if( IsTemplate( RF_ClassDefaultObject | RF_ArchetypeObject ) )
	{
		return;
	}

	if( DuplicateMode == EDuplicateMode::Normal )
	{
		if( SelfName.IsEmpty() == false )
		{
			// if is different it means that this actor was duplicated ( alt-dragged or copy pasted )
			if( SelfName.Equals( GetName() ) == false )
			{
				// find all path points
				TArray<AActor*> foundActors;
				UGameplayStatics::GetAllActorsOfClass( this, GetClass(), foundActors );

				for( AActor* actor : foundActors )
				{
					// find point with old name of this one ( source point of duplication )
					if( actor->GetName().Equals( SelfName ) )
					{
						// if actor was found, then create connection between them
						// this allows to create paths with alt-drag or copy paste
						if( ADASPathPoint* actorAsPathPoint = Cast<ADASPathPoint>( actor ) )
						{
							actorAsPathPoint->NextPathPoints.Add( this );
							PreviousPathPoints.Add( actorAsPathPoint );
						}

						break;
					}
				}
				// update self name after that
				SelfName = GetName();
			}
		}
	}
#endif
}


void ADASPathPoint::ValidateData_Implementation()
{
	Super::ValidateData_Implementation();

	// remove invalid path points from arrays
	for( int i = NextPathPoints.Num() - 1; i >= 0; i-- )
	{
		if( NextPathPoints[ i ] == nullptr )
		{
			NextPathPoints.RemoveAt( i );
		}
	}
	for( int i = PreviousPathPoints.Num() - 1; i >= 0; i-- )
	{
		if( PreviousPathPoints[ i ] == nullptr )
		{
			PreviousPathPoints.RemoveAt( i );
		}
	}

	// call validation on Action Selector
	if( ActionSelector )
	{
		ActionSelector->ValidateData();
	}
}






/************************************************************************/
/*									DEBUG								*/
/************************************************************************/

void ADASPathPoint::DrawDebug( float DeltaTime, bool bIsInEditor )
{
#if WITH_EDITORONLY_DATA
	Super::DrawDebug( DeltaTime, bIsInEditor );

	if( UWorld* world = GetWorld() )
	{
		const FVector ownerLocation = GetActorLocation();

		// call debug function on Action Selector
		if( ActionSelector && PathActionExecutionMethod != EDASPathExecuteMethod::None )
		{
			ActionSelector->DrawDebug( DeltaTime, this, !world->IsGameWorld() );
		}

		// Draw Spots
		if( Spots.Num() > 0 )
		{
			for( const FDASSpot& spot : Spots )
			{
				// get spot location in world space
				FVector worldSpaceLoc = FVector( 0.f, 0.f, 20.f ) + GetTransform().TransformPosition( spot.Transform.GetLocation() );

				// draw line from actor to spot
				DrawDebugLine( world, FVector( 0.f, 0.f, 20.f ) + ownerLocation, worldSpaceLoc, FColor::Cyan, false, DeltaTime, 0, 2.f );

				// draw spot
				DrawDebugPoint( world, worldSpaceLoc, spot.Transform.GetRotation().Rotator(), DeltaTime );
			}
		}
		else
		{
			// draw spot on owner location
			DrawDebugPoint( world, FVector( 0.f, 0.f, 20.f ) + ownerLocation, GetActorRotation(), DeltaTime );
		}

		float pathOffsetDistance = 10.f;
		const bool bAnimateArrows = UDASDeveloperSettings::Get()->bAnimatePathArrows && !bIsInEditor;
		float currentTime = world->GetTimeSeconds();
		float arrowSpeed = UDASDeveloperSettings::Get()->AnimatedArrowsSpeed;
		float arrowAlpha = 0.5f;

		// Draw Forward Path
		if( bCanMoveForward )
		{
			for( ADASPathPoint* pathPoint : NextPathPoints )
			{
				if( pathPoint )
				{
					FVector dirToPath = UKismetMathLibrary::GetDirectionUnitVector( GetActorLocation(), pathPoint->GetActorLocation() );
					FVector rightVec = UKismetMathLibrary::Cross_VectorVector( dirToPath, FVector::UpVector );
					FVector startLoc = FVector( 0.f, 0.f, 20.f ) + GetActorLocation() + rightVec * pathOffsetDistance + dirToPath * pathOffsetDistance * 2.f;
					FVector endLoc = FVector( 0.f, 0.f, 20.f ) + pathPoint->GetActorLocation() + rightVec * pathOffsetDistance - dirToPath * pathOffsetDistance * 2.f;

					if( bAnimateArrows )
					{
						float distSquared = ( startLoc - endLoc ).Size();
						float requiredTime = distSquared / arrowSpeed;
						arrowAlpha = ( FMath::Fmod( currentTime, requiredTime ) * arrowSpeed ) / distSquared;
					}

					FVector arrowLoc = UKismetMathLibrary::VLerp( startLoc, endLoc, arrowAlpha );

					DrawDebugLine( world, startLoc, endLoc, FColor::Green, false, DeltaTime, 0, 2.f );
					DrawDebugDirectionalArrow( world, startLoc, arrowLoc, 250.f, FColor::Green, false, DeltaTime, 0, 2.f );
				}
			}
		}

		// Draw Backward Path
		if( bCanMoveBackward )
		{
			for( ADASPathPoint* pathPoint : PreviousPathPoints )
			{
				if( pathPoint )
				{
					FVector dirToPath = UKismetMathLibrary::GetDirectionUnitVector( GetActorLocation(), pathPoint->GetActorLocation() );
					FVector rightVec = UKismetMathLibrary::Cross_VectorVector( dirToPath, FVector::UpVector );
					FVector startLoc = FVector( 0.f, 0.f, 20.f ) + GetActorLocation() + ( rightVec * pathOffsetDistance ) + dirToPath * pathOffsetDistance * 2.f;;
					FVector endLoc = FVector( 0.f, 0.f, 20.f ) + pathPoint->GetActorLocation() + ( rightVec * pathOffsetDistance ) - dirToPath * pathOffsetDistance * 2.f;

					if( bAnimateArrows )
					{
						float distSquared = ( startLoc - endLoc ).Size();
						float requiredTime = distSquared / arrowSpeed;
						arrowAlpha = ( FMath::Fmod( currentTime, requiredTime ) * arrowSpeed ) / distSquared;
					}

					FVector arrowLoc = UKismetMathLibrary::VLerp( startLoc, endLoc, arrowAlpha );

					DrawDebugLine( world, startLoc, endLoc, FColor::Red, false, DeltaTime, 0, 2.f );
					DrawDebugDirectionalArrow( world, startLoc, arrowLoc, 250.f, FColor::Red, false, DeltaTime, 0, 2.f );

				}
			}
		}

	}
#endif
}

