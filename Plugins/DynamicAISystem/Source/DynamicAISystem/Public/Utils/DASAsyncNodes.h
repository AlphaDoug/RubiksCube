// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DASTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DASAsyncNodes.generated.h"


class ADASActionPoint;
class UDASComponent;
class UDASAction;

/** Event used by async nodes, includes enum result */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FDASAsyncNodeResult, EDASExecutionResult, Result );


/**
 * Execute Action Point Async Node
 */
UCLASS()
class DYNAMICAISYSTEM_API UAsyncActionHandlePointExecution : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	/** Starts executing action point */
	UFUNCTION(Category = "DAS", BlueprintCallable, meta = (BlueprintInternalUseOnly = "true" ) )
	static UAsyncActionHandlePointExecution* ExecuteActionPoint( ADASActionPoint* ActionPoint, UDASComponent* DASComponent );

	virtual void Activate() override;

	/** Node called when action point has finished execution with any result */
	UPROPERTY(Category = "DAS", BlueprintAssignable)
	FDASAsyncNodeResult OnFinished;

protected:
	/** Cached action point that is being executed */
	UPROPERTY()
	ADASActionPoint* ActiveActionPoint;

	/** Cached DAS Component that executes action point */
	UPROPERTY()
	UDASComponent* ActiveDASComponent;

	/** Event called when action point finishes execution */
	void HandleActionPointExecutionFinish( EDASExecutionResult Result );
};


/**
 * Execute Action Async Node
 */
UCLASS()
class DYNAMICAISYSTEM_API UAsyncActionHandleActionExecution : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	/** Starts executing action */
	UFUNCTION(Category = "DAS", BlueprintCallable, meta = (BlueprintInternalUseOnly = "true" ) )
	static UAsyncActionHandleActionExecution* ExecuteAction( UDASAction* Action, UDASComponent* DASComponent );

	virtual void Activate() override;

	/** Node called when action point has finished execution with any result */
	UPROPERTY( Category = "DAS", BlueprintAssignable )
	FDASAsyncNodeResult OnFinished;

protected:
	/** Cached action that is being executed */
	UPROPERTY()
	UDASAction* ActiveAction;

	/** Cached DAS Component that executes action */
	UPROPERTY()
	UDASComponent* ActiveDASComponent;

	/** Event called when action point finishes execution */
	UFUNCTION()
	void HandleActionExecutionFinish( EDASExecutionResult Result );
};








