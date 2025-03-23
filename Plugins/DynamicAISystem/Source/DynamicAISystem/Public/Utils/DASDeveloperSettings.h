// Copyright (C) 2022 Grzegorz Szewczyk - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DASDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS( config = Game, defaultconfig, meta = ( DisplayName = "Dynamic AI System" ) )
class DYNAMICAISYSTEM_API UDASDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UDASDeveloperSettings();

	/** Distance to camera/player at which debug info will be displayed */
	UPROPERTY( EditAnywhere, config, Category = "Debug" )
	float DrawDebugMaxDistance = 5000.f;

	/**
	 * How often debug is refreshed
	 * 0 means every frame, may impact frame rate
	 */
	UPROPERTY( EditAnywhere, config, Category = "Debug" )
	float DebugTickInterval = 0.f;

	/** Should arrows between path points animate? or should not move */
	UPROPERTY( EditAnywhere, config, Category = "Debug" )
	uint32 bAnimatePathArrows : 1;

	UPROPERTY( EditAnywhere, config, Category = "Debug" )
	float AnimatedArrowsSpeed = 100.f;

	UPROPERTY( EditAnywhere, config, Category = "Debug" )
	FColor PathPointsDebugColor = FColor::Blue;

	UPROPERTY( EditAnywhere, config, Category = "Debug" )
	FColor ActionPointsDebugColor = FColor::Purple;

	/** Returns default object of this class */
	static const UDASDeveloperSettings* Get() { return GetDefault<UDASDeveloperSettings>(); }
};
