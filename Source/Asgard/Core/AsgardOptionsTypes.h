// Copyright © 2020 Justin Camden All Rights Reserved

#pragma once

#include "CoreMinimal.h"

// Enum declarations
UENUM(BlueprintType)
enum class EAsgardTeleportMode : uint8
{
	Fade,
	Instant,
	Smooth
};

UENUM(BlueprintType)
enum class EAsgardGroundMovementMode : uint8
{
	PrecisionTeleportToLocation,
	SmoothWalk,
	TeleportWalk
};

UENUM(BlueprintType)
enum class EAsgardOrientationMode : uint8
{
	Character,
	LeftController,
	RightController
};

UENUM(BlueprintType)
enum class EAsgardFlightOrientationMode : uint8
{
	VRCamera,
	LeftController,
	RightController
};

UENUM(BlueprintType)
enum class EAsgardBinaryHand : uint8
{
	LeftHand,
	RightHand
};