// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_RotateToFaceBBEntry.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BehaviorTree/Services/BTService_DefaultFocus.h"
#include "BTTask_DASRotateToFaceBBEntry.generated.h"


struct FBTRotateMemory
{
	AActor* FocusActorSet;
	FVector FocusLocationSet;
	bool bActorSet;

	bool bOrientToMovement = false;
	bool bUseControlDesRot = false;

	void Reset()
	{
		FocusActorSet = nullptr;
		FocusLocationSet = FAISystem::InvalidLocation;
		bActorSet = false;
		bOrientToMovement = false;
		bUseControlDesRot = false;
	}
};


class AAIController;
class ACharacter;

/**
 * This is extension of default UE task UBTTask_RotateToFaceBBEntry
 * but this task also turns on ControllerDesiredRotation if controlled pawn is a character and disables it on finish.
 * It is because if by default character uses OrientRotationToMovement, then it will not work with controller rotation.
 */
UCLASS( config = Game )
class DYNAMICAISYSTEM_API UBTTask_DASRotateToFaceBBEntry : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
protected:
	/** Success condition precision in degrees */
	UPROPERTY( config, Category = Node, EditAnywhere, meta = (ClampMin = "5.0") )
	float Precision;

private:
	/** cached Precision tangent value */
	float PrecisionDot;

public:

	UBTTask_DASRotateToFaceBBEntry();

	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

	virtual EBTNodeResult::Type ExecuteTask( UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory ) override;
	virtual void TickTask( UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds ) override;
	virtual EBTNodeResult::Type AbortTask( UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory ) override;
	virtual void DescribeRuntimeValues( const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values ) const override;
	virtual FString GetStaticDescription() const override;

	virtual uint16 GetInstanceMemorySize() const override { return sizeof( FBTRotateMemory ); }

protected:

	// enables controller desired rotation if owner is character so it can rotate properly
	void SetControlRotation( bool bUseControlRot, bool bOrientToMovement, ACharacter* Character );
	float GetPrecisionDot() const { return PrecisionDot; }
	void CleanUp( AAIController& AIController, uint8* NodeMemory );
};
