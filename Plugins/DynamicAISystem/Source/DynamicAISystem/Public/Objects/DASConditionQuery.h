// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DASCondition.h"
#include "DASConditionQuery.generated.h"


/**
 * Struct wrapper of condition query instance
 * Allows for ease of use in blueprints
 * Because Instanced object variables can't be created in blueprints
 */
USTRUCT( BlueprintType ) 
struct DYNAMICAISYSTEM_API FDASConditionQueryWrapper
{
	GENERATED_BODY()

public:
	FDASConditionQueryWrapper() :
		Instance( nullptr )
	{}

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Instanced, Category = DASConditionQueryWrapper )
	UDASConditionQuery* Instance;

	bool Initialize( AActor* QueryOwner );
	bool IsConditionFulfilled();
	bool IsValid() const { return Instance != nullptr; }
};
/************************************************************************/


/**
 * Wraps array of conditions
 * observers their state and calls events whenever total result of all conditions is changing
 */
UCLASS( BlueprintType, EditInlineNew, DefaultToInstanced )
class DYNAMICAISYSTEM_API UDASConditionQuery : public UObject
{
	GENERATED_BODY()

public:
	UDASConditionQuery();

#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override;
#endif
	virtual void PostInitProperties() override;

	/** Array of conditions */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DASConditionQuery )
	TArray< FDASConditionWrapper > Conditions;

	/** Event that is triggered when result of all conditions is changed */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnConditionResultChanged OnConditionResultChanged;

	/** Returns true if total result of conditions array is fulfilled, false if not */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASConditionQuery )
	bool IsConditionFulfilled();

	/** Initializes condition query and all conditions within it */
	UFUNCTION( BlueprintCallable, Category = DASConditionQuery )
	void Initialize( AActor* Owner );

	/** Uninitializes condition query and all conditions within it */
	UFUNCTION( BlueprintCallable, Category = DASConditionQuery )
	void Uninitialize();

	/** Draws debug info about query, propagates to all conditions */
	UFUNCTION( BlueprintCallable, Category = DASConditionQuery )
	void DrawDebug( float DeltaTime, AActor* Caller, bool bIsInEditor );

	UFUNCTION( BlueprintCallable, Category = DASConditionQuery )
	void ValidateData();

	UFUNCTION( BlueprintCallable, Category = DASConditionQuery )
	FString GetQueryDescription() const;

protected:
	/**
	 * Checks what is total result of all conditions
	 * doesn't update cached condition result
	 * This is internal function that shouldn't be called manually
	 * only within IsConditionFulfilled
	 */
	virtual bool IsConditionFulfilled_Internal();

	/** flag telling if condition is initialized */
	uint32 bIsInitialized : 1;

	/** Cached owner of condition, who initialized it */
	TWeakObjectPtr<AActor> ConditionOwner;

	/** Cached result of condition, used to compare with current result and calling OnChange events only when it really changes */
	ECachedConditionResult CachedConditionResult;

	/** Function called when any of condition from array has changed its state */
	UFUNCTION()
	void OnInnerConditionResultChanged( bool bResult ) { IsConditionFulfilled(); }

	/** Convert bool to enumECachedConditionResult and store it in variable */
	void SetCachedConditionResult( bool bNewValue )
	{
		if( bNewValue ) CachedConditionResult = ECachedConditionResult::True;
		else CachedConditionResult = ECachedConditionResult::False;
	}
};
