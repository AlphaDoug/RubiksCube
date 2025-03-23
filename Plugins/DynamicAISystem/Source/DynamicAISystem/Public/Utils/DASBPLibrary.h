// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "DynamicAISystem.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Objects/DASConditionQuery.h"
#include "DASBPLibrary.generated.h"


class ADASPathPoint;
class ADASActionPoint;

/**
 * Library which includes bunch of helpful functions used across the system
 */
UCLASS()
class DYNAMICAISYSTEM_API UDASBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * Returns default component of CDO
	 * This will work on blueprint actors as well while FindComponentByClass doesn't
	 */
    static UActorComponent* FindDefaultComponentByClass( const TSubclassOf<AActor> InActorClass, const TSubclassOf<UActorComponent> InComponentClass );
    
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASLibrary )
	static FLinearColor GetPathPointsDebugColor();

	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASLibrary )
	static FLinearColor GetActionPointsDebugColor();

	/** Clears timers or stops delay nodes that are active on given object */
	UFUNCTION( BlueprintCallable, Category = DASLibrary )
	static void ClearTimersAndDelays( UObject* Object, bool bClearTimers = true, bool bClearDelays = true );

	/** Calls function with given name on object */
	UFUNCTION( BlueprintCallable, Category = DASLibrary )
	static bool CallFunctionByName( UObject* Object, FString FunctionName );

	/**
	 * Returns enum from given string
	 * Example of use - GetStringFromEnum( TEXT( "ESizeType" ), ( int32 )MediumSize )
	 * @param EnumType - Enum type string, for example TEXT("ESizeType")
	 * @param EnumValue - Enum value, for example (int32)MediumSize
	 */
	static FString EnumToString( const TCHAR* EnumType, int32 EnumValue );



	/************************************************************************/
	/*							PATH & ACTION POINTS                        */
	/************************************************************************/
public:
	/**
	 * Removes all actions points that are currently taken from given array
	 * and returns only free ones in new filtered array
	 */
	UFUNCTION( BlueprintCallable, Category = DASLibrary )
	static void FilterOutTakenActionPoints( TArray<ADASActionPoint*>& FilteredActionPoints, const TArray<ADASActionPoint*>& ActionPoints );

	/**
	 * Sorts array of given points by distance to given location
	 * By default sorts array ascending - from lowest to highest
	 * @param bInverseSort - changes direction of sorting, from highest to lowest
	 */
	UFUNCTION( BlueprintCallable, Category = DASLibrary )
	static void SortActionPointsByDistance( const TArray<ADASActionPoint*>& ArrayToSort, TArray<ADASActionPoint*>& SortedArray, FVector SourceLocation, bool bInverse );

	/**
	 * Sorts array of given points by distance to given location
	 * By default sorts array ascending - from lowest to highest
	 * @param bInverseSort - changes direction of sorting, from highest to lowest
	 */
	UFUNCTION( BlueprintCallable, Category = DASLibrary )
	static void SortPathPointsByDistance( const TArray<ADASPathPoint*>& ArrayToSort, TArray<ADASPathPoint*>& SortedArray, FVector SourceLocation, bool bInverse );
	/************************************************************************/





	/************************************************************************/
	/*							CONDITION QUERY                             */
	/************************************************************************/
public:
	/** Returns success if initialization was successful, false if not - when condition query is not valid */
	UFUNCTION( BlueprintCallable, Category = "DASLibrary|Condition", meta = ( ReturnDisplayName = "Success" ) )
	static bool InitializeConditionQuery( UPARAM(ref)FDASConditionQueryWrapper& ConditionQuery, AActor* QueryOwner ) { return ConditionQuery.Initialize( QueryOwner ); }

	/** Returns true if condition instance in query is valid */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|Condition", meta = ( ReturnDisplayName = "IsValid" ))
	static bool IsConditionQueryValid( const FDASConditionQueryWrapper& ConditionQuery ) { return ConditionQuery.IsValid(); }

	/** Returns true if condition is fullfiled or if condition is not valid ( not valid means no conditions to check ) */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|Condition", meta = ( ReturnDisplayName = "Is Fulfilled" ) )
	static bool IsConditionQueryFulfilled( UPARAM( ref )FDASConditionQueryWrapper& ConditionQuery ) { return ConditionQuery.IsConditionFulfilled(); }
	/************************************************************************/






	/************************************************************************/
	/*							BLACKBOARD KEYS                             */
	/************************************************************************/
protected:
	static const FName BBKeyName_GoalLocation;
	static const FName BBKeyName_GoalRotation;
	static const FName BBKeyName_PathPoint;
	static const FName BBKeyName_ActionPoint;
	static const FName BBKeyName_IsActionPointTaken;
	static const FName BBKeyName_CanRunActionPoint;
	static const FName BBKeyName_CanRunPathPoint;
	static const FName BBKeyName_IsActionPointTakenByMe;
	static const FName BBKeyName_RunMode;
	static const FName BBKeyName_ActionSelector;

public:
	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_GoalLocation() { return BBKeyName_GoalLocation; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_GoalRotation() { return BBKeyName_GoalRotation; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_PathPoint() { return BBKeyName_PathPoint; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_ActionPoint() { return BBKeyName_ActionPoint; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_IsActionPointTaken() { return BBKeyName_IsActionPointTaken; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_CanRunActionPoint() { return BBKeyName_CanRunActionPoint; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_CanRunPathPoint() { return BBKeyName_CanRunPathPoint; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_IsActionPointTakenByMe() { return BBKeyName_IsActionPointTakenByMe; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_RunMode() { return BBKeyName_RunMode; }

	/** Returns blackboard key name */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "DASLibrary|AI" )
	static const FName& GetBBKeyName_ActionSelector() { return BBKeyName_ActionSelector; }
	/************************************************************************/
};
