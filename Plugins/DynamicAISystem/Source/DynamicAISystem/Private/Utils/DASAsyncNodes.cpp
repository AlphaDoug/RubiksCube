// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved


#include "Utils/DASAsyncNodes.h"
#include "Components/DASComponent.h"
#include "Objects/DASAction.h"
#include "Points/DASActionPoint.h"


UAsyncActionHandlePointExecution* UAsyncActionHandlePointExecution::ExecuteActionPoint( ADASActionPoint* ActionPoint, UDASComponent* DASComponent )
{
	// make sure given action point is valid
	if( IsValid( ActionPoint ) )
	{
		UAsyncActionHandlePointExecution* Action = NewObject<UAsyncActionHandlePointExecution>();
		Action->RegisterWithGameInstance( ActionPoint );

		// cache data of executing DAS Component
		Action->ActiveActionPoint = ActionPoint;
		Action->ActiveDASComponent = DASComponent;
		return Action;
	}

	return nullptr;
}

void UAsyncActionHandlePointExecution::Activate()
{
	if( IsValid( ActiveActionPoint ) && IsValid( ActiveDASComponent ) )
	{
		// start action point execution, when it will finish execute, function HandleActionPointExecutionFinish will be called
		ActiveActionPoint->AsyncExecute( ActiveDASComponent, FAsyncExecutionFinishedWithResultDelegate::CreateUObject( this, &UAsyncActionHandlePointExecution::HandleActionPointExecutionFinish ) );
	}
	else
	{
		HandleActionPointExecutionFinish( EDASExecutionResult::Failed );
	}
}


void UAsyncActionHandlePointExecution::HandleActionPointExecutionFinish( EDASExecutionResult Result )
{
	// call on finished node
	OnFinished.Broadcast( Result );

	// reset data
	ActiveActionPoint = nullptr;
	ActiveDASComponent = nullptr;
	SetReadyToDestroy();
}











UAsyncActionHandleActionExecution* UAsyncActionHandleActionExecution::ExecuteAction( UDASAction* Action, UDASComponent* DASComponent )
{
	// make sure given action is valid
	if( IsValid( Action ) )
	{
		UAsyncActionHandleActionExecution* ActionObj = NewObject<UAsyncActionHandleActionExecution>();
		ActionObj->RegisterWithGameInstance( Action );

		// cache data of executing DAS Component
		ActionObj->ActiveAction = Action;
		ActionObj->ActiveDASComponent = DASComponent;
		return ActionObj;
	}

	return nullptr;
}

void UAsyncActionHandleActionExecution::Activate()
{
	if( IsValid( ActiveAction ) && IsValid( ActiveDASComponent ) )
	{
		// start action execution, when it will finish execute, function HandleActionExecutionFinish will be called
		ActiveAction->AsyncExecute( ActiveDASComponent, FAsyncExecutionFinishedWithResultDelegate::CreateUObject( this, &UAsyncActionHandleActionExecution::HandleActionExecutionFinish ) );
	}
	else
	{
		HandleActionExecutionFinish( EDASExecutionResult::Failed );
	}
}



void UAsyncActionHandleActionExecution::HandleActionExecutionFinish(EDASExecutionResult Result)
{
	// call on finished node
	OnFinished.Broadcast( Result );
	
	// reset data
	ActiveAction = nullptr;
	ActiveDASComponent = nullptr;
	SetReadyToDestroy();
}
