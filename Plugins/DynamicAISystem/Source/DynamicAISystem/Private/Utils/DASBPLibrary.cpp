// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Utils/DASBPLibrary.h"
#include "Engine/SCS_Node.h"
#include "DrawDebugHelpers.h"
#include "Utils/DASDeveloperSettings.h"
#include "Animation/AnimSequenceBase.h"
#include "Points/DASActionPoint.h"
#include "Points/DASPathPoint.h"
#include "TimerManager.h"


const FName UDASBPLibrary::BBKeyName_GoalLocation = FName( "GoalLocation" );
const FName UDASBPLibrary::BBKeyName_GoalRotation = FName( "GoalRotation" );
const FName UDASBPLibrary::BBKeyName_PathPoint = FName( "PathPoint" );
const FName UDASBPLibrary::BBKeyName_ActionPoint = FName( "ActionPoint" );
const FName UDASBPLibrary::BBKeyName_IsActionPointTaken = FName( "IsActionPointTaken" );
const FName UDASBPLibrary::BBKeyName_CanRunActionPoint = FName( "CanRunActionPoint" );
const FName UDASBPLibrary::BBKeyName_CanRunPathPoint = FName( "CanRunPathPoint" );
const FName UDASBPLibrary::BBKeyName_IsActionPointTakenByMe = FName( "IsActionPointTakenByMe" );
const FName UDASBPLibrary::BBKeyName_RunMode = FName( "RunMode" );
const FName UDASBPLibrary::BBKeyName_ActionSelector = FName( "ActionSelector" );


UActorComponent* UDASBPLibrary::FindDefaultComponentByClass( const TSubclassOf<AActor> InActorClass, const TSubclassOf<UActorComponent> InComponentClass )
{
	if( !IsValid( InActorClass ) )
	{
		return nullptr;
	}

	// Check CDO.
	AActor* ActorCDO = InActorClass->GetDefaultObject<AActor>();
	UActorComponent* FoundComponent = ActorCDO->FindComponentByClass( InComponentClass );

	if( FoundComponent != nullptr )
	{
		return FoundComponent;
	}

	// Check blueprint nodes. Components added in blueprint editor only (and not in code) are not available from CDO
	UBlueprintGeneratedClass* RootBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>( InActorClass );
	UClass* ActorClass = InActorClass;

	// Go down the inheritance tree to find nodes that were added to parent blueprints of our blueprint graph.
	do
	{
		UBlueprintGeneratedClass* ActorBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>( ActorClass );
		if( !ActorBlueprintGeneratedClass )
		{
			return nullptr;
		}

		const TArray<USCS_Node*>& ActorBlueprintNodes =
			ActorBlueprintGeneratedClass->SimpleConstructionScript->GetAllNodes();

		for( USCS_Node* Node : ActorBlueprintNodes )
		{
			if( Node->ComponentClass->IsChildOf( InComponentClass ) )
			{
				return Node->GetActualComponentTemplate( RootBlueprintGeneratedClass );
			}
		}

		ActorClass = Cast<UClass>( ActorClass->GetSuperStruct() );

	} while( ActorClass != AActor::StaticClass() );

	return nullptr;
}

FLinearColor UDASBPLibrary::GetPathPointsDebugColor()
{
	return FLinearColor::FromSRGBColor( UDASDeveloperSettings::Get()->PathPointsDebugColor );
}

FLinearColor UDASBPLibrary::GetActionPointsDebugColor()
{
	return FLinearColor::FromSRGBColor( UDASDeveloperSettings::Get()->ActionPointsDebugColor );
}

void UDASBPLibrary::ClearTimersAndDelays( UObject* Object, bool bClearTimers /*= true*/, bool bClearDelays /*= true */ )
{
	if( IsValid( Object ) )
	{
		if( UWorld* world = Object->GetWorld() )
		{
			// clear all timers
			if( bClearTimers )
			{
				world->GetTimerManager().ClearAllTimersForObject( Object );
			}

			// clear all delay nodes ( Delay / RetriggableDelay etc. )
			if( bClearDelays )
			{
				world->GetLatentActionManager().RemoveActionsForObject( Object );
			}
		}
	}
}

bool UDASBPLibrary::CallFunctionByName( UObject* Object, FString FunctionName )
{
	FName const FunctionFName( *FunctionName );

	// check if object is valid
	if( !IsValid( Object ) )
	{
		FString ObjectName = Object ? *Object->GetName() : TEXT( "None" );
		UE_LOG( LogDAS, Warning, TEXT( "Invalid object to call function %s" ), *FunctionName );
		return false;
	}

	UFunction* const Function = Object->FindFunction( FunctionFName );

	// check if function was found on given object
	if( !Function )
	{
		FString ObjectName = Object ? *Object->GetName() : TEXT( "None" );
		UE_LOG( LogDAS, Error, TEXT( "Function %s not found on %s" ), *FunctionName, *ObjectName );
		return false;
	}

	// check if function doesn't take any params
	if( Function->ParmsSize > 0 )
	{
		FString ObjectName = Object ? *Object->GetName() : TEXT( "None" );
		UE_LOG( LogDAS, Error, TEXT( "Function %s on %s can't take any params" ), *FunctionName, *ObjectName );
		return false;
	}

	Object->ProcessEvent( Function, nullptr );
	return true;
}

FString UDASBPLibrary::EnumToString( const TCHAR* EnumType, int32 EnumValue )
{
	// find enum and convert given value to string
	UEnum* foundEnum = FindObject<UEnum>( ANY_PACKAGE, EnumType, true );
	if( foundEnum )
	{
		return foundEnum->GetNameStringByIndex( EnumValue );
	}
	return FString( "Invalid - are you sure enum uses UENUM() macro?" );
}


void UDASBPLibrary::FilterOutTakenActionPoints( TArray<ADASActionPoint*>& FilteredActionPoints, const TArray<ADASActionPoint*>& ActionPoints )
{
	FilteredActionPoints.Reserve( ActionPoints.Num() );

	for( ADASActionPoint* actionPoint : ActionPoints )
	{
		if( actionPoint && !actionPoint->IsTaken() )
		{
			FilteredActionPoints.Add( actionPoint );
		}
	}
}

void UDASBPLibrary::SortActionPointsByDistance( const TArray<ADASActionPoint*>& ArrayToSort, TArray<ADASActionPoint*>& SortedArray, FVector SourceLocation, bool bInverse )
{
	SortedArray = ArrayToSort;

	// remove any invalid elements before sorting
	for( int32 i = SortedArray.Num() - 1; i >= 0; i-- )
	{
		if( !IsValid( SortedArray[ i ] ) )
			SortedArray.RemoveAt( i, 1, false );
	}
	SortedArray.Shrink();


	SortedArray.Sort( [&SourceLocation, &bInverse]( const ADASActionPoint& LHS, const ADASActionPoint& RHS ){

		const float distA = FVector::DistSquared( LHS.GetActorLocation(), SourceLocation );
		const float distB = FVector::DistSquared( RHS.GetActorLocation(), SourceLocation );

		if( bInverse )
		{
			return distB < distA;
		}
		else
		{
			return distB > distA;
		}

		} );
}

void UDASBPLibrary::SortPathPointsByDistance( const TArray<ADASPathPoint*>& ArrayToSort, TArray<ADASPathPoint*>& SortedArray, FVector SourceLocation, bool bInverse )
{
	SortedArray = ArrayToSort;

	// remove any invalid elements before sorting
	for( int32 i = SortedArray.Num() - 1; i >= 0; i-- )
	{
		if( !IsValid( SortedArray[ i ] ) )
			SortedArray.RemoveAt( i, 1, false );
	}
	SortedArray.Shrink();


	SortedArray.Sort( [&SourceLocation, &bInverse]( const ADASPathPoint& LHS, const ADASPathPoint& RHS ){

		const float distA = FVector::DistSquared( LHS.GetActorLocation(), SourceLocation );
		const float distB = FVector::DistSquared( RHS.GetActorLocation(), SourceLocation );

		if( bInverse )
		{
			return distB < distA;
		}
		else
		{
			return distB > distA;
		}

		} );
}


