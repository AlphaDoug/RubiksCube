// Copyright (C) 2021 Grzegorz Szewczyk - All Rights Reserved

#include "Utils/DASTypes.h"
#include "GameFramework/Actor.h"


bool FDASSpot::IsTaken() const
{
	return IsValid( SpotOwner );
}