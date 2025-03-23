// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "BehaviorTree/BTTask_DASRotateToFaceBBEntry.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Rotator.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"


UBTTask_DASRotateToFaceBBEntry::UBTTask_DASRotateToFaceBBEntry()
	: Precision( 10.f )
{
	NodeName = "DAS Rotate to face BB entry";
	bNotifyTick = true;

	// accept only actors and vectors
	BlackboardKey.AddObjectFilter( this, GET_MEMBER_NAME_CHECKED( UBTTask_DASRotateToFaceBBEntry, BlackboardKey ), AActor::StaticClass() );
	BlackboardKey.AddVectorFilter( this, GET_MEMBER_NAME_CHECKED( UBTTask_DASRotateToFaceBBEntry, BlackboardKey ) );
	BlackboardKey.AddRotatorFilter( this, GET_MEMBER_NAME_CHECKED( UBTTask_DASRotateToFaceBBEntry, BlackboardKey ) );
}

void UBTTask_DASRotateToFaceBBEntry::PostInitProperties()
{
	Super::PostInitProperties();

	// clamp precision to be at least 5
	Precision = UKismetMathLibrary::FMax( 5.f, Precision );
	PrecisionDot = FMath::Cos( FMath::DegreesToRadians( Precision ) );
}

void UBTTask_DASRotateToFaceBBEntry::PostLoad()
{
	Super::PostLoad();

	Precision = UKismetMathLibrary::FMax( 5.f, Precision );
	PrecisionDot = FMath::Cos( FMath::DegreesToRadians( Precision ) );
}

namespace
{
	FORCEINLINE_DEBUGGABLE float CalculateAngleDifferenceDot( const FVector& VectorA, const FVector& VectorB )
	{
		return (VectorA.IsNearlyZero() || VectorB.IsNearlyZero())
			? 1.f
			: VectorA.CosineAngle2D( VectorB );
	}
}

void UBTTask_DASRotateToFaceBBEntry::SetControlRotation( bool bUseControlRot, bool bOrientToMovement, ACharacter* Character)
{
	if( IsValid( Character ) && Character->GetCharacterMovement() )
	{
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = bUseControlRot;
		Character->GetCharacterMovement()->bOrientRotationToMovement = bOrientToMovement;
	}
}

EBTNodeResult::Type UBTTask_DASRotateToFaceBBEntry::ExecuteTask( UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory )
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	if( AIController == NULL || AIController->GetPawn() == NULL )
	{
		return EBTNodeResult::Failed;
	}

	FBTRotateMemory* MyMemory = (FBTRotateMemory*)NodeMemory;
	check( MyMemory );
	MyMemory->Reset();

	EBTNodeResult::Type Result = EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();
	const FVector PawnLocation = Pawn->GetActorLocation();
	const UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();

	if( BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass() )
	{
		UObject* KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Object>( BlackboardKey.GetSelectedKeyID() );
		AActor* ActorValue = Cast<AActor>( KeyValue );

		if( ActorValue != NULL )
		{
			const float AngleDifference = CalculateAngleDifferenceDot( Pawn->GetActorForwardVector()
				, (ActorValue->GetActorLocation() - PawnLocation) );

			if( AngleDifference >= PrecisionDot )
			{
				Result = EBTNodeResult::Succeeded;
			}
			else
			{
				ACharacter* character = Cast<ACharacter>( Pawn );
				if( IsValid( character ) && character->GetCharacterMovement() )
				{
					MyMemory->bOrientToMovement = character->GetCharacterMovement()->bOrientRotationToMovement;
					MyMemory->bUseControlDesRot = character->GetCharacterMovement()->bUseControllerDesiredRotation;

					SetControlRotation( true, false, character );
				}

				AIController->SetFocus( ActorValue, EAIFocusPriority::Gameplay );
				MyMemory->FocusActorSet = ActorValue;
				MyMemory->bActorSet = true;
				Result = EBTNodeResult::InProgress;
			}
		}
	}
	else if( BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass() )
	{
		const FVector KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Vector>( BlackboardKey.GetSelectedKeyID() );

		if( FAISystem::IsValidLocation( KeyValue ) )
		{
			const float AngleDifference = CalculateAngleDifferenceDot( Pawn->GetActorForwardVector()
				, (KeyValue - PawnLocation) );

			if( AngleDifference >= PrecisionDot )
			{
				Result = EBTNodeResult::Succeeded;
			}
			else
			{
				ACharacter* character = Cast<ACharacter>( Pawn );
				if( IsValid( character ) && character->GetCharacterMovement() )
				{
					MyMemory->bOrientToMovement = character->GetCharacterMovement()->bOrientRotationToMovement;
					MyMemory->bUseControlDesRot = character->GetCharacterMovement()->bUseControllerDesiredRotation;

					SetControlRotation( true, false, character );
				}

				AIController->SetFocalPoint( KeyValue, EAIFocusPriority::Gameplay );
				MyMemory->FocusLocationSet = KeyValue;
				Result = EBTNodeResult::InProgress;
			}
		}
	}
	else if( BlackboardKey.SelectedKeyType == UBlackboardKeyType_Rotator::StaticClass() )
	{
		const FRotator KeyValue = MyBlackboard->GetValue<UBlackboardKeyType_Rotator>( BlackboardKey.GetSelectedKeyID() );

		if( FAISystem::IsValidRotation( KeyValue ) )
		{
			const FVector DirectionVector = KeyValue.Vector();
			const float AngleDifference = CalculateAngleDifferenceDot( Pawn->GetActorForwardVector(), DirectionVector );

			if( AngleDifference >= PrecisionDot )
			{
				Result = EBTNodeResult::Succeeded;
			}
			else
			{
				ACharacter* character = Cast<ACharacter>( Pawn );
				if( IsValid( character ) && character->GetCharacterMovement() )
				{
					MyMemory->bOrientToMovement = character->GetCharacterMovement()->bOrientRotationToMovement;
					MyMemory->bUseControlDesRot = character->GetCharacterMovement()->bUseControllerDesiredRotation;

					SetControlRotation( true, false, character );
				}

				const FVector FocalPoint = PawnLocation + DirectionVector * 10000.0f;
				// set focal somewhere far in the indicated direction
				AIController->SetFocalPoint( FocalPoint, EAIFocusPriority::Gameplay );
				MyMemory->FocusLocationSet = FocalPoint;
				Result = EBTNodeResult::InProgress;
			}
		}
	}

	return Result;
}

void UBTTask_DASRotateToFaceBBEntry::TickTask( UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds )
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	if( AIController == NULL || AIController->GetPawn() == NULL )
	{
		FinishLatentTask( OwnerComp, EBTNodeResult::Failed );
	}
	else
	{
		const FVector PawnDirection = AIController->GetPawn()->GetActorForwardVector();
		const FVector FocalPoint = AIController->GetFocalPointForPriority( EAIFocusPriority::Gameplay );

		if( CalculateAngleDifferenceDot( PawnDirection, FocalPoint - AIController->GetPawn()->GetActorLocation() ) >= PrecisionDot )
		{
			CleanUp( *AIController, NodeMemory );
			FinishLatentTask( OwnerComp, EBTNodeResult::Succeeded );
		}
	}
}

void UBTTask_DASRotateToFaceBBEntry::CleanUp( AAIController& AIController, uint8* NodeMemory )
{
	FBTRotateMemory* MyMemory = ( FBTRotateMemory*)NodeMemory;
	check( MyMemory );

	ACharacter* character = AIController.GetPawn<ACharacter>();
	if( IsValid( character ) && character->GetCharacterMovement() )
	{
		MyMemory->bOrientToMovement = character->GetCharacterMovement()->bOrientRotationToMovement;
		MyMemory->bUseControlDesRot = character->GetCharacterMovement()->bUseControllerDesiredRotation;

		SetControlRotation( MyMemory->bUseControlDesRot, MyMemory->bOrientToMovement, character );
	}

	bool bClearFocus = false;
	if( MyMemory->bActorSet )
	{
		bClearFocus = (MyMemory->FocusActorSet == AIController.GetFocusActorForPriority( EAIFocusPriority::Gameplay ));
	}
	else
	{
		bClearFocus = (MyMemory->FocusLocationSet == AIController.GetFocalPointForPriority( EAIFocusPriority::Gameplay ));
	}

	if( bClearFocus )
	{
		AIController.ClearFocus( EAIFocusPriority::Gameplay );
	}
}

EBTNodeResult::Type UBTTask_DASRotateToFaceBBEntry::AbortTask( UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory )
{
	AAIController* AIController = OwnerComp.GetAIOwner();

	if( AIController != NULL )
	{
		CleanUp( *AIController, NodeMemory );
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_DASRotateToFaceBBEntry::DescribeRuntimeValues( const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values ) const
{
	FString KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	Values.Add( FString::Printf( TEXT( "%s: %s" ), *Super::GetStaticDescription(), *KeyDesc ) );

	AAIController* AIController = OwnerComp.GetAIOwner();

	if( AIController != NULL && AIController->GetPawn() != NULL )
	{
		const FVector PawnDirection = AIController->GetPawn()->GetActorForwardVector();
		const FVector FocalPoint = AIController->GetFocalPointForPriority( EAIFocusPriority::Gameplay );

		if( FocalPoint != FAISystem::InvalidLocation )
		{
			const float CurrentAngleRadians = CalculateAngleDifferenceDot( PawnDirection, (FocalPoint - AIController->GetPawn()->GetActorLocation()) );
			Values.Add( FString::Printf( TEXT( "Current angle: %.2f" ), FMath::RadiansToDegrees( FMath::Acos( CurrentAngleRadians ) ) ) );
		}
		else
		{
			Values.Add( TEXT( "FocalPoint is an Invalid Location" ) );
		}
	}
	else
	{
		Values.Add( TEXT( "Controller or Pawn is NULL" ) );
	}
}

FString UBTTask_DASRotateToFaceBBEntry::GetStaticDescription() const
{
	FString KeyDesc = BlackboardKey.SelectedKeyName.ToString();
	return FString::Printf( TEXT( "%s: %s" ), *Super::GetStaticDescription(), *KeyDesc );
}

