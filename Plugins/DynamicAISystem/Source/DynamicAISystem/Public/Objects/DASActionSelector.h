// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DASActionSelector.generated.h"

/**
 * Object which is used for selecting ActionPoints
 */
UCLASS( Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced )
class DYNAMICAISYSTEM_API UDASActionSelector : public UObject
{
	GENERATED_BODY()
public:

#if WITH_ENGINE
	/** Overriding this function gives access to all function requiring WorldContext */
	virtual class UWorld* GetWorld() const override;
#endif
	
	/**
	 * Returns action points that should be executed by this selector
	 */
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = PathAction )
	void GetActionPointsToExecute( TArray< ADASActionPoint* >& OutActionPoints, UDASComponent* DASComponent );
	void GetActionPointsToExecute_Implementation( TArray< ADASActionPoint* >& OutActionPoints, UDASComponent* DASComponent ) {};


	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = Debug, meta = ( DevelopmentOnly ) )
	void DrawDebug( float DeltaTime, AActor* Caller, bool bIsInEditor );
	void DrawDebug_Implementation( float DeltaTime, AActor* Caller, bool bIsInEditor ) {};

	/**
	 * Validates data of point
	 * For example removes all invalid references
	 */
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = PathAction )
	void ValidateData();
	virtual void ValidateData_Implementation() {};

	/** returns outer (owner) of this object as actor */
	UFUNCTION( BlueprintCallable, Category = PathAction )
	AActor* GetOuterAsActor();

	/** returns outer (owner) of this object as PathPoint, this will only work for action selectors placed on path points */
	UFUNCTION( BlueprintCallable, Category = PathAction )
	ADASPathPoint* GetOuterAsPathPoint();
};
