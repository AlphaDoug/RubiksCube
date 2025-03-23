// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Utils/DASTypes.h"
#include "DASCondition.generated.h"


/**
 * Struct wrapper of condition instance
 * which is used by ConditionQuery
 */
USTRUCT( BlueprintType )
struct DYNAMICAISYSTEM_API FDASConditionWrapper
{
	GENERATED_BODY()

public:
	/** Instance object of condition */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Instanced, Category = DASConditionWrapper )
	UDASCondition* Instance = nullptr;

#if WITH_EDITORONLY_DATA
	/** Editor-only variable describing condition */
	UPROPERTY( EditAnywhere, meta = ( MultiLine ), Category = DASConditionWrapper )
	FString Description;

	/** Editor-only flag which helps to define whether this condition is at index 0 in array of conditions */
	UPROPERTY( VisibleAnywhere, Category = DASConditionWrapper )
	bool bIsFirstCondition = true;
#endif

	/** Operator connecting this condition with next one in array */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASConditionWrapper )
	EDASOperator Operator = EDASOperator::AND;
};


/**
 * Cached condition result
 * instead of simple bool flag to have Undefined value ( when condition was not yet initialized )
 */
enum class ECachedConditionResult : uint8
{
	Undefined,
	True,
	False
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnConditionResultChanged, bool, bResult );


/**
 * Condition class
 */
UCLASS( Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced )
class DYNAMICAISYSTEM_API UDASCondition : public UObject
{
	GENERATED_BODY()

public:
	UDASCondition();

#if WITH_ENGINE
	/** Overriding this function gives access to all function requiring WorldContext */
	virtual class UWorld* GetWorld() const override;
#endif

	/** Event called when condition result changes */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnConditionResultChanged OnConditionResultChanged;

	/** Returns true if condition is fulfilled, false if not */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASCondition, meta = ( ReturnDisplayName = "Success" ) )
	bool IsConditionFulfilled();

	/** Force condition check to update cached result */
	UFUNCTION( BlueprintCallable, Category = DASCondition )
	void UpdateCondition() { IsConditionFulfilled(); }

	/**
	 * Initialize condition
	 * Starts observing its requirements
	 */
	UFUNCTION( BlueprintCallable, Category = DASCondition )
	virtual void Initialize( AActor* Owner );

	/**
	 * Uninitialize condition
	 * Stops observing its requirements
	 */
	UFUNCTION( BlueprintCallable, Category = DASCondition )
	virtual void Uninitialize();

	/**
	 * This function should start observing requirements
	 * for example if actors are in good state etc.
	 * gets called when condition is initialized
	 */
	UFUNCTION( BlueprintNativeEvent, Category = DASCondition )
	void AddObservers();
	virtual void AddObservers_Implementation() {};

	/**
	 * This function should stop observing requirements
	 * Gets called when condition is uninitialized
	 */
	UFUNCTION( BlueprintNativeEvent, Category = DASCondition )
	void RemoveObservers();
	virtual void RemoveObservers_Implementation() {};

	/** Draws debug info about condition */
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = DASCondition )
	void DrawDebug( float DeltaTime, AActor* Caller, bool bIsInEditor, int32 ConditionIndex = 0 );
	void DrawDebug_Implementation( float DeltaTime, AActor* Caller, bool bIsInEditor, int32 ConditionIndex /*= 0 */ ) {};

	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = DASCondition )
	FString GetConditionDescription();
	virtual FString GetConditionDescription_Implementation() const { return FString(); }

	/** Validate condition data, check if has any invalid references, wrong data etc. */
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = DASCondition )
	void ValidateData();
	virtual void ValidateData_Implementation() {};

protected:
	/**
	 * This function should contain actual condition check
	 * for example: are doors open? does player have below 50% HP? Was main boss killed? etc.
	 */
	UFUNCTION( BlueprintNativeEvent, Category = DASCondition, meta = ( ReturnDisplayName = "Success" ) )
	bool IsConditionFulfilled_Internal();
	virtual bool IsConditionFulfilled_Internal_Implementation() { return true; }

	/** Called when condition result changes */
	UFUNCTION( BlueprintNativeEvent, Category = Condition )
	void ConditionResultChanged( bool bNewResult );
	virtual void ConditionResultChanged_Implementation( bool bNewResult ) {};

	/** Cached owner of condition, who initialized it */
	TWeakObjectPtr<AActor> ConditionOwner;

	/** flag telling if condition is initialized */
	uint32 bIsInitialized : 1;

	/** Cached result of condition, used to compare with current result and calling OnChange events only when it really changes */
	ECachedConditionResult CachedConditionResult;

	/** Returns owner of condition casted as a character */
	class ACharacter* GetOwnerAsCharacter();

	/** Returns owner of condition casted as a AI Controller */
	class AAIController* GetOwnerAsAIController();

	/** Convert bool to enumECachedConditionResult and store it in variable */
	void SetCachedConditionResult( bool bNewValue )
	{
		if( bNewValue ) CachedConditionResult = ECachedConditionResult::True;
		else CachedConditionResult = ECachedConditionResult::False;
	}
};
