// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Objects/DASAction.h"
#include "Utils/DASBPLibrary.h"
#include "Points/DASActionPoint.h"


UDASAction::UDASAction()
{
	bIsExecuting = false;
}


#if WITH_ENGINE
class UWorld* UDASAction::GetWorld() const
{
	if( GetOuter() == nullptr )
	{
		return nullptr;
	}

#if WITH_EDITOR
	// Special case for the editor
	if( Cast<UPackage>( GetOuter() ) != nullptr )
	{
		// GetOuter should return a UPackage and its Outer is a UWorld
		return Cast<UWorld>( GetOuter()->GetOuter() );
	}
#endif

	// In all other cases use outer
	return GetOuter()->GetWorld();
}
#endif


ADASActionPoint* UDASAction::GetOuterAsActionPoint()
{
	if( UObject* outer = GetOuter() )
	{
		return Cast<ADASActionPoint>( outer );
	}
	return nullptr;
}


void UDASAction::AsyncExecute( UDASComponent* DASComponent, FAsyncExecutionFinishedWithResultDelegate FinishExecutionDelegate )
{
	if( bIsExecuting )
	{
		if( FinishExecutionDelegate.IsBound() )
			FinishExecutionDelegate.ExecuteIfBound( EDASExecutionResult::Failed );
	}
	else
	{
		ActiveFinishExecutionDelegate = FinishExecutionDelegate;
		Execute( DASComponent );
	}
}

void UDASAction::Execute( UDASComponent* DASComponent )
{
	if( bIsExecuting == false )
	{
		ActiveDASComponent = DASComponent;
		bIsExecuting = true;
		ReceiveExecute( DASComponent );
	}
}

void UDASAction::FinishExecute(bool bSuccess /*= true */)
{
	if( bIsExecuting )
	{
		bIsExecuting = false;

		EDASExecutionResult result = bSuccess ? EDASExecutionResult::Success : EDASExecutionResult::Failed;

		// stop all delays/timers running on this action point
		UDASBPLibrary::ClearTimersAndDelays( this );
		
		ReceiveFinishExecute( ActiveDASComponent, result );

		ActiveDASComponent = nullptr;

		if( ActiveFinishExecutionDelegate.IsBound() )
			ActiveFinishExecutionDelegate.ExecuteIfBound( result );
	}
}
