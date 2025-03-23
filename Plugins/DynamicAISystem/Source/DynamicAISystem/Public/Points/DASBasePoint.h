// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Objects/DASConditionQuery.h"
#include "Components/PrimitiveComponent.h"
#include "DASBasePoint.generated.h"



class UDASConditionsWrapper;



/**
 * Base class of points - ActionPoint & PathPoint
 * This is abstract class, can not be placed on the level
 * Just contains base logic that Action & Path points are extending
 */
UCLASS( Abstract, BlueprintType, NotBlueprintable )
class DYNAMICAISYSTEM_API ADASBasePoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ADASBasePoint();


	/************************************************************************/
	/*							    PARENT OVERRIDES				        */
	/************************************************************************/
public:
	virtual void PostDuplicate( EDuplicateMode::Type DuplicateMode ) override;
	virtual void PostActorCreated() override;

protected:
	virtual void BeginPlay() override;
	/************************************************************************/

	


	/************************************************************************/
	/*								DATA									*/
	/************************************************************************/
public:
	/** Root component */
	UPROPERTY( VisibleDefaultsOnly, BlueprintReadOnly, Category = Components )
	USceneComponent* SceneComponent;

	/** Condition defining if this point can be used */
	UPROPERTY( EditInstanceOnly, BlueprintReadOnly, Category = "Settings|Condition", meta = ( DisplayPriority = 10 ) )
	FDASConditionQueryWrapper ConditionQuery;

	/**
	 * Movement speed AI will use when moving to this point
	 * 0 means don't override AI speed
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement", meta = ( ClampMin = 0.f ) )
	float MoveSpeed = 0.f;

	/** Should AI rotate to this point rotation on reaching it? */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Movement" )
	uint32 bRotateToPoint : 1;

	/**
	 * Tag defining this Point
	 * Can be used when AI wants to use point with certain tags
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = ( DisplayPriority = 10 ) )
	FGameplayTag PointTag;

	/** Unique ID of this point, auto-generated, used for saving/loading */
	UPROPERTY( VisibleInstanceOnly, BlueprintReadOnly, Category = "Editor" )
	FGuid PointId;
	/************************************************************************/





	/************************************************************************/
	/*								CAN RUN POINT							*/
	/************************************************************************/
public:
	/**
	 * Can this point be used by AI?
	 * For example is condition query fulfilled ( if has any ), is not on cooldown? etc.
	 */
	UFUNCTION( BlueprintNativeEvent, BlueprintPure, BlueprintCallable, Category = DASPoint )
	bool CanRun();
	virtual bool CanRun_Implementation();
	/************************************************************************/





	/************************************************************************/
	/*								EDITOR									*/
	/************************************************************************/
public:
	/** Displays debug info about point, called only in EDITOR */
	UFUNCTION( BlueprintImplementableEvent, Category = Debug, meta = ( DisplayName = "DrawDebug" ) )
	void K2_DrawDebug( float DeltaTime, bool bIsInEditor ) const;
	virtual void DrawDebug( float DeltaTime, bool bIsInEditor );

	/**
	 * Validates data of point
	 * For example if has any invalid references
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, CallInEditor, Category = Settings, meta = ( DevelopmentOnly ) )
	void ValidateData();
	virtual void ValidateData_Implementation();

	/**
	 * Refreshes instanced object to make sure they are unique.
	 * Sometimes when multiple selected Actors with instanced objects are copied at once
	 * It may happen that Instanced Objects will not be re instanced on new copies
	 * but will share same instance with Actor, it was copied from
	 * This function makes sure that all Instanced Objects within this Actor will be safe and unique
	 * Don't use in runtime! only in Editor
	 */
	UFUNCTION( BlueprintCallable, Category = Settings, meta = ( DevelopmentOnly ) )
	virtual void RefreshInstancedObjects();

	/** Helper function to visualize point location */
	void DrawDebugPoint( UWorld* world, const FVector& SpotLocation, const FRotator& SpotRotation, float DeltaTime );

	/** Editor only component, just visual representation of point in world */
	UPROPERTY( VisibleDefaultsOnly, BlueprintReadOnly, Category = Components )
	class UBillboardComponent* BillboardComponent;

#if WITH_EDITORONLY_DATA
	/** Editor only component, used to call DrawDebug function */
	UPROPERTY()
	UDASPointVisComponent* PointVisComponent;
#endif // WITH_EDITORONLY_DATA
	/************************************************************************/
};





/************************************************************************/
/*                      VISUALIZER COMPONENT                            */
/************************************************************************/

UCLASS()
class DYNAMICAISYSTEM_API UDASPointVisComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UDASPointVisComponent();
	virtual void TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
};
/************************************************************************/