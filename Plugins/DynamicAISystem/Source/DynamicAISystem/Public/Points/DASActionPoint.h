// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Utils/DASTypes.h"
#include "DASBasePoint.h"
#include "GameplayTagContainer.h"
#include "Objects/DASAction.h"
#include "DASActionPoint.generated.h"


class UDASComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnIsTakenChanged, bool, bNewIsTaken );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnIsExecutingChanged, bool, bNewIsExecuting );


/**
 * Point which is used to execute certain actions by AI
 * For example - PlayAnimation / Wait / ExecuteFunction etc.
 * 
 * Automatically stops all times/delays when action is finished/interrupted
 */
UCLASS( Abstract, Blueprintable, BlueprintType )
class DYNAMICAISYSTEM_API ADASActionPoint : public ADASBasePoint
{
	GENERATED_BODY()
	
public:
	ADASActionPoint();





	/************************************************************************/
	/*							    PARENT OVERRIDES				        */
	/************************************************************************/
protected:
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

public:
	virtual bool CanRun_Implementation() override;
	virtual void DrawDebug( float DeltaTime, bool bIsInEditor ) override;
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;
	virtual void PostInitializeComponents() override;
	virtual void RefreshInstancedObjects() override;
	/************************************************************************/






	/************************************************************************/
	/*							EXECUTION FLOW LOGIC		                */
	/************************************************************************/
public:
	/** Event called when this action starts/stops being executed by AI */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnIsExecutingChanged OnIsExecutingChanged;

	/**
	 * Actions that will be executed when AI will use this action point
	 * Executed sequentially, one by one
	 */
	UPROPERTY( EditInstanceOnly, BlueprintReadOnly, Instanced, Category = "Settings" )
	TArray<UDASAction*> Actions;

	/**
	 * DAS Component of AI that is currently executing this action point
	 * only valid if any AI is executing this action point
	 */
	UPROPERTY( BlueprintReadOnly, Category = ActionPoint )
	UDASComponent* ActiveDASComponent = nullptr;

	/** Returns current index of executed action */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = ActionPoint )
	FORCEINLINE int32 GetCurrentActionIndex() { return CurrentActionIndex; }

	/** Ends execution of this action point */
	UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = ActionPoint )
	void FinishExecute( bool bSuccess = true );

	/** Returns true if any AI is currently executing this Action Point */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = ActionPoint )
	FORCEINLINE bool IsExecuting() const { return bIsExecuting; }

	/**
	 * Starts executing this action point
	 * Not exposed to blueprints on purpose because action point can be executed through async node ExecuteActionPoint
	 */
	void Execute( UDASComponent* DASComponent );

	/** Async version of execute which is used by blueprint async node */
	void AsyncExecute( UDASComponent* DASComponent, FAsyncExecutionFinishedWithResultDelegate FinishExecutionDelegate );

protected:
	/**
	 * Current index of executed action
	 */
	UPROPERTY( BlueprintReadWrite, Category = "Settings|Flow", meta = ( ClampMin = 1 ) )
	int32 CurrentActionIndex = 0;

	/** Flag which is set when action point starts/ends executing */
	UPROPERTY( ReplicatedUsing = "OnRep_IsExecuting" )
	uint32 bIsExecuting : 1;

	/** Delegate used by async node executing this action point */
	FAsyncExecutionFinishedWithResultDelegate ActiveFinishExecutionDelegate;

	/** Changes value of flag bIsExecuting, includes replication update */
	void SetIsExecuting( bool bNewExecuting );

	/** OnRep event that is called whenever value of bIsExecuting is changed */
	UFUNCTION()
	virtual void OnRep_IsExecuting();

	/** Event called when value of obIsExecuting is changed */
	UFUNCTION( BlueprintImplementableEvent )
	void IsExecutingValueChanged( bool bNewIsExecuting );

	/** Event called when action point starts executing */
	UFUNCTION( BlueprintImplementableEvent, BlueprintAuthorityOnly, Category = ActionPoint )
	void ReceiveExecute( UDASComponent* DASComponent );

	/** Event called when action point finished executing */
	UFUNCTION( BlueprintImplementableEvent, BlueprintAuthorityOnly, Category = ActionPoint )
	void ReceiveFinishExecute( EDASExecutionResult Result );
	/************************************************************************/





	/************************************************************************/
	/*							    IS POINT TAKEN LOGIC				    */
	/************************************************************************/
public:
	/**
	 * True means that AI on reaching point will be moved to exact spot location
	 * because by default due to acceptance radius AI may be a little bit off of real spot location
	 * set to true only if AI needs to be very precisely on point location
	 * because it may cause little 'sliding' effect
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category= "Settings|Movement" )
	uint32 bUsePreciseLocation : 1;

	/**
	 * Should execution of action point be interrupted if AI executing it was moved?
	 * for example stop playing montage when AI was launched away by grenade explosion
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category= "Settings|Movement", meta = ( DisplayName = "Interrupt Action On AI Moved" ) )
	uint32 bInterruptActionOnAIMoved : 1;
	/************************************************************************/

	

	/************************************************************************/
	/*							    IS POINT TAKEN LOGIC				    */
	/************************************************************************/
public:
	/** Event called when action point is taken/free by AI */
	UPROPERTY( BlueprintAssignable, Category = Events )
	FOnIsTakenChanged OnIsTakenChanged;

	/**
	 * Returns true if this action point is taken by any AI
	 * It means whether any AI is currently moving towards it, or is executing it
	 * Used to avoid situation when 2 or more AIs want to use same Action Point
	 */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASActionPoint )
	FORCEINLINE bool IsTaken() const { return bIsTaken; }

	/** Switch IsTaken state of this action point */
	UFUNCTION( BlueprintCallable, Category = DASActionPoint )
	void SetIsTaken( bool bNewIsTaken );

	/** Event called when IsTaken state is changed */
	UFUNCTION( BlueprintImplementableEvent, Category = DASActionPoint )
	void IsTakenChanged( bool bNewIsTaken );

protected:
	/**
	 * Flag telling if this point is taken by any AI
	 * means if any AI is currently moving towards or executing it
	 */
	UPROPERTY( ReplicatedUsing = "OnRep_IsTaken")
	uint32 bIsTaken : 1;

	UFUNCTION()
	virtual void OnRep_IsTaken();
	/************************************************************************/





	/************************************************************************/
	/*						CANCEL ON CONDITION FAIL LOGIC		            */
	/************************************************************************/
public:
	/**
	 * Flag telling how AI should behave if condition query will fail while AI is already executing this action point
	 * This has no effect if AI is moving to Action Point, only when already executing it
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Condition" )
	uint32 bCancelExecutionOnConditionFail : 1;

	/**
	 * How long should it take AI to interrupt this action point if condition query has failed while executing it
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Condition", meta = ( EditCondition = "bCancelExecutionOnConditionFail" ) )
	float DelayToCancelOnConditionFail = 0.5f;
	/************************************************************************/





	/************************************************************************/
	/*							COOLDOWN		                            */
	/************************************************************************/
public:
	/**
	 * How often this Action Point can be executed
	 * Cooldown is applied on successfully finishing action point by any AI
	 * and is applied globally, meaning that when its on cooldown, no one can execute it
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Settings|Condition", meta = ( ClampMin = 0 ) )
	float Cooldown = 0;
	
	/** Applies cooldown to this action point */
	UFUNCTION( BlueprintCallable, Category = DASActionPoint )
	void ApplyCooldown();

	/** Applies custom cooldown value to this action point */
	UFUNCTION( BlueprintCallable, Category = DASActionPoint )
	void ApplyCustomCooldown( float CooldownToApply );

	/** Resets cooldown of this action point ( if there was any ) */
	UFUNCTION( BlueprintCallable, Category = DASActionPoint )
	void ResetCooldown() { CooldownEndWorldTime = 0.f; }
	
	/** Returns true if action point is currently on cooldown */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASActionPoint )
	bool IsOnCooldown() const;

	/** Returns remaining time of cooldown in seconds */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = DASActionPoint )
	float GetCooldownRemainingTime() const;

protected:
	/**
	 * World time telling when this action point cooldown will end
	 */
	float CooldownEndWorldTime = 0.f;
	/************************************************************************/

};
