// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DASInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDASInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DYNAMICAISYSTEM_API IDASInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/** Returns DASComponent */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = DAS )
	class UDASComponent* GetDASComponent();
};
