// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Utils/DASTypes.h"
#include "DASAction.generated.h"



/**
 * Action is basically a behavior executed by AI, that lives in Action Point.
 * There are few types of actions ready to be used in the system:
 * Play AnimMontage / Wait/ ExecuteFunction etc.
 */
UCLASS( Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable )
class DYNAMICAISYSTEM_API UDASAction : public UObject
{
	GENERATED_BODY()
	
public:
	UDASAction();

#if WITH_ENGINE
	/** Overriding this function gives access to all function requiring WorldContext */
	virtual class UWorld* GetWorld() const override;
#endif

	/**
	 * Starts executing this action
	 * Not exposed to blueprints on purpose because action can be executed through async node ExecuteAction
	 */
	void Execute( UDASComponent* DASComponent );

	/** Async version of execute which is used by blueprint async node */
	void AsyncExecute( UDASComponent* DASComponent, FAsyncExecutionFinishedWithResultDelegate FinishExecutionDelegate );

	/** Ends execution of this action */
	UFUNCTION( BlueprintCallable, Category = DASAction )
	void FinishExecute( bool bSuccess = true );

	/** Returns true if this action is currently executed by any AI */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASAction )
	FORCEINLINE bool IsExecuting() const { return bIsExecuting; }

	/**
	 * Returns outer (owner) of this object as ActionPoint
	 * This will only work if action lives within ActionPoint which it always should
	 */
	UFUNCTION( BlueprintCallable, Category = PathAction )
	class ADASActionPoint* GetOuterAsActionPoint();

protected:
	/** Delegate used by async node executing this action point */
	FAsyncExecutionFinishedWithResultDelegate ActiveFinishExecutionDelegate;

	/** Flag which is set when action starts/ends executing */
	uint32 bIsExecuting : 1;

	/** Event called when action starts executing */
	UFUNCTION( BlueprintImplementableEvent, Category = DASAction )
	void ReceiveExecute( UDASComponent* DASComponent );
	
	/** Event called when action finished executing */
	UFUNCTION( BlueprintImplementableEvent, Category = DASAction )
	void ReceiveFinishExecute( UDASComponent* DASComponent, EDASExecutionResult Result );

	/**
	 * DAS Component of AI that is currently executing this action
	 * only valid if any AI is executing this action
	 */
	UPROPERTY( BlueprintReadOnly, Category = DASAction )
	UDASComponent* ActiveDASComponent = nullptr;
};
