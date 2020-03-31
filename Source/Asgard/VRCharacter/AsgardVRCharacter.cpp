// Copyright © 2020 Justin Camden All Rights Reserved


#include "AsgardVRCharacter.h"
#include "AsgardVRMovementComponent.h"
#include "Asgard/Core/AsgardInputBindings.h"
#include "Asgard/Core/AsgardCollisionProfiles.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/Engine/Public/EngineUtils.h"


AAsgardVRCharacter::AAsgardVRCharacter(const FObjectInitializer& ObjectInitializer)
:Super(ObjectInitializer.SetDefaultSubobjectClass<UAsgardVRMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Navigation settings
	NavQueryExtent = FVector(150.f, 150.f, 150.f);

	// Walking Settings
	bIsWalkingEnabled = true;
	WalkingInputDeadZone = 0.15f;
	WalkingInputMaxZone = 0.9f;
	WalkingInputForwardMultiplier = 1.0f;
	WalkingInputStrafeMultiplier = 1.0f;
	WalkingInputBackwardMultiplier = 1.0f;
	WalkingOrientationMaxAbsPitchRadians = 0.9f;

	// Flight settings
	bIsFlightEnabled = true;
	FlightAnalogInputDeadZone = 0.15f;
	FlightAnalogInputMaxZone = 0.9f;
	FlightAnalogInputForwardMultiplier = 1.0f;
	FlightAnalogInputStrafeMultiplier = 1.0f;
	FlightAnalogInputBackwardMultiplier = 1.0f;

	// Movement state
	bIsWalking = false;

	// Component references
	if (UPawnMovementComponent* MoveComp = GetMovementComponent())
	{
		AsgardVRMovementRef = Cast<UAsgardVRMovementComponent>(MoveComp);
	}

	// Collision profiles
	GetCapsuleComponent()->SetCollisionProfileName(UAsgardCollisionProfiles::VRRoot());
}

void AAsgardVRCharacter::BeginPlay()
{
	// Initialize input settings
	SetWalkingOrientationMode(WalkingOrientationMode);
	SetFlightAnalogOrientationMode(FlightAnalogOrientationMode);

	// Cache the navdata
	CacheNavData();

	Super::BeginPlay();
}

void AAsgardVRCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateMovement();

	return;
}

void AAsgardVRCharacter::OnMoveForwardAxis(float Value)
{
	MoveForwardAxis = Value;

	return;
}

void AAsgardVRCharacter::OnMoveRightAxis(float Value)
{
	MoveRightAxis = Value;

	return;
}

void AAsgardVRCharacter::OnFlightThrusterLeftActionPressed()
{
	bIsFlightThrusterLeftActionPressed = true;
	return;
}

void AAsgardVRCharacter::OnFlightThrusterLeftActionReleased()
{
	bIsFlightThrusterLeftActionPressed = false;
	return;
}

void AAsgardVRCharacter::OnFlightThrusterRightActionPressed()
{
	bIsFlightThrusterRightActionPressed = true;
	return;
}

void AAsgardVRCharacter::OnFlightThrusterRightActionReleased()
{
	bIsFlightThrusterRightActionPressed = true;
}

void AAsgardVRCharacter::OnToggleFlightActionPressed()
{
	switch (AsgardVRMovementRef->GetAsgardMovementMode())
	{
		case EAsgardMovementMode::C_MOVE_Walking:
		{
			if (CanFly())
			{
				AsgardVRMovementRef->SetAsgardMovementMode(EAsgardMovementMode::C_MOVE_Flying);
			}
			break;
		}
		case EAsgardMovementMode::C_MOVE_Falling:
		{
			if (CanFly())
			{
				AsgardVRMovementRef->SetAsgardMovementMode(EAsgardMovementMode::C_MOVE_Flying);
			}
			break;
		}
		case EAsgardMovementMode::C_MOVE_Flying:
		{
			AsgardVRMovementRef->SetAsgardMovementMode(EAsgardMovementMode::C_MOVE_Falling);
			break;
		}
		default:
		{
			break;
		}
	}

	return;
}

void AAsgardVRCharacter::SetWalkingOrientationMode(const EAsgardWalkingOrientationMode NewWalkingOrientationMode)
{
	WalkingOrientationMode = NewWalkingOrientationMode;
	switch (WalkingOrientationMode)
	{
		case EAsgardWalkingOrientationMode::LeftController:
		{
			WalkingOrientationComponent = LeftMotionController;
			break;
		}
		case EAsgardWalkingOrientationMode::RightController:
		{
			WalkingOrientationComponent = RightMotionController;
			break;
		}
		case EAsgardWalkingOrientationMode::VRCamera:
		{
			WalkingOrientationComponent = VRReplicatedCamera;
			break;
		}
		case EAsgardWalkingOrientationMode::CharacterCapsule:
		{
			WalkingOrientationComponent = VRRootReference;
			break;
		}
		default:
		{
			ensureMsgf(false, TEXT("Invalid orientation mode! (AsgardVRCharacter, Actor %s, SetWalkingOrientationMode)"), *GetNameSafe(this));
			break;
		}
	}

	return;
}

void AAsgardVRCharacter::SetFlightAnalogOrientationMode(const EAsgardFlightAnalogOrientationMode NewFlyingOrientationMode)
{
	FlightAnalogOrientationMode = NewFlyingOrientationMode;
	switch (FlightAnalogOrientationMode)
	{
		case EAsgardFlightAnalogOrientationMode::LeftController:
		{
			FlightAnalogOrientationComponent = LeftMotionController;
			break;
		}
		case EAsgardFlightAnalogOrientationMode::RightController:
		{
			FlightAnalogOrientationComponent = RightMotionController;
			break;
		}
		case EAsgardFlightAnalogOrientationMode::VRCamera:
		{
			FlightAnalogOrientationComponent = VRReplicatedCamera;
			break;
		}
		default:
		{
			ensureMsgf(false, TEXT("Invalid orientation mode! (AsgardVRCharacter, Actor %s, SetFlyingAnalogOrientationMode)"), *GetNameSafe(this));
			break;
		}
	}

	return;
}

void AAsgardVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Action bindings
	PlayerInputComponent->BindAction(UAsgardInputBindings::FlightThrusterLeftAction(), IE_Pressed, this,&AAsgardVRCharacter::OnFlightThrusterLeftActionPressed);
	PlayerInputComponent->BindAction(UAsgardInputBindings::FlightThrusterLeftAction(), IE_Released, this, &AAsgardVRCharacter::OnFlightThrusterLeftActionReleased);
	PlayerInputComponent->BindAction(UAsgardInputBindings::FlightThrusterRightAction(), IE_Pressed, this, &AAsgardVRCharacter::OnFlightThrusterRightActionPressed);
	PlayerInputComponent->BindAction(UAsgardInputBindings::FlightThrusterRightAction(), IE_Released, this, &AAsgardVRCharacter::OnFlightThrusterRightActionReleased);
	PlayerInputComponent->BindAction(UAsgardInputBindings::ToggleFlightAction(), IE_Pressed, this, &AAsgardVRCharacter::OnToggleFlightActionPressed);

	// Axis bindings
	PlayerInputComponent->BindAxis(UAsgardInputBindings::MoveForwardAxis(), this, &AAsgardVRCharacter::OnMoveForwardAxis);
	PlayerInputComponent->BindAxis(UAsgardInputBindings::MoveRightAxis(), this, &AAsgardVRCharacter::OnMoveRightAxis);

	return;
}

bool AAsgardVRCharacter::CanWalk_Implementation()
{
	return bIsWalkingEnabled;
}

bool AAsgardVRCharacter::CanFly_Implementation()
{
	return bIsFlightEnabled;
}

bool AAsgardVRCharacter::IsAboveNavMesh(float MaxHeight, FVector& OutProjectedPoint)
{
	// Return true if we are moving on the ground
	if (VRMovementReference->IsMovingOnGround())
	{
		return true;
	}

	// Otherwise, perform a trace from the bottom of the capsule collider towards the ground
	FVector TraceOrigin = GetCapsuleComponent()->GetComponentLocation();
	FVector TraceEnd = TraceOrigin;
	TraceEnd.Z -= (MaxHeight + GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult TraceHitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("VRCharacterAboveNavMeshTrace")), false, this);
	if (GetWorld()->LineTraceSingleByProfile(TraceHitResult, TraceOrigin, TraceEnd, UAsgardCollisionProfiles::VRRoot(), TraceParams))
	{
		// If we hit an object, check to see if the point can be projected onto a navmesh;
		return ProjectPointToVRNavigation(TraceHitResult.ImpactPoint, OutProjectedPoint, false);
	}

	return false;
}

void AAsgardVRCharacter::CacheNavData()
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (ANavigationData* CurrNavData : TActorRange<ANavigationData>(World))
		{
			if (GetNameSafe(CurrNavData) == "RecastNavMesh-VRCharacter")
			{
				NavData = CurrNavData;
				break;
			}
		}
	}

	return;
}

bool AAsgardVRCharacter::ProjectPointToVRNavigation(const FVector& Point, FVector& OutProjectedPoint, bool bCheckIfIsOnGround)
{
	// Project the to the navigation
	UNavigationSystemV1* const NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedNavLoc(Point);
		bool const bRetVal = NavSys->ProjectPointToNavigation(Point,
			ProjectedNavLoc,
			(NavQueryExtent.IsNearlyZero() ? INVALID_NAVEXTENT : NavQueryExtent),
			NavData,
			UNavigationQueryFilter::GetQueryFilter(*NavData, this, NavFilterClass));
		OutProjectedPoint = ProjectedNavLoc.Location;

		// If point projected successfully and we want to ensure the point is on the ground
		if (bRetVal && bCheckIfIsOnGround)
		{
			// Trace downwards and see if we hit something
			FHitResult GroundTraceHitResult;
			const FVector GroundTraceOrigin = OutProjectedPoint;
			const FVector GroundTraceEnd = GroundTraceOrigin + FVector(0.0f, 0.0f, -200.0f);
			FCollisionQueryParams GroundTraceParams(FName(TEXT("VRCharacterGroundTrace")), false, this);
			if (GetWorld()->LineTraceSingleByProfile(GroundTraceHitResult, GroundTraceOrigin, GroundTraceEnd, UAsgardCollisionProfiles::VRRoot(), GroundTraceParams))
			{
				// If so, return the point of impact
				OutProjectedPoint = GroundTraceHitResult.ImpactPoint;
				return true;
			}
			else
			{
				// Otherwise, return false
				return false;
			}
		}

		// Otherwise return the projection result
		else 
		{
			return bRetVal;
		}
	}

	return false;
}

void AAsgardVRCharacter::UpdateMovement()
{
	UE_LOG(LogTemp, All, TEXT("Updating input0"));
	// Branch depending on current movement mode
	switch (AsgardVRMovementRef->GetAsgardMovementMode())
	{
		case EAsgardMovementMode::C_MOVE_Walking:
		{
			// If we can walk
			if (CanWalk())
			{
				UE_LOG(LogTemp, All, TEXT("Can walk"));
				// Get the input vector depending on the walking weight mode
				FVector WalkingInput = FVector::ZeroVector;
				if (WalkingInputAxisWeightMode == EAsgardInputAxisWeightMode::Circular)
				{
					WalkingInput = CalculateCircularInputVector(
						MoveForwardAxis,
						MoveRightAxis,
						WalkingInputDeadZone,
						WalkingInputMaxZone);
				}
				else
				{
					WalkingInput = CalculateCrossInputVector(
						MoveForwardAxis,
						MoveRightAxis,
						WalkingInputDeadZone,
						WalkingInputMaxZone);
				}

				// Guard against no input
				if (WalkingInput.IsNearlyZero())
				{
					UE_LOG(LogTemp, All, TEXT("Input vector was 0"));
					StopWalking();
					break;
				}

				UE_LOG(LogTemp, All, TEXT("Orienting Input vector"));

				// Transform the input vector from the forward of the orientation component
				FVector WalkingForward = WalkingOrientationComponent->GetForwardVector();

				// Guard against the forward vector being nearly up or down
				if (FMath::Abs(FVector::DotProduct(FVector::UpVector, WalkingForward)) > (WalkingOrientationMaxAbsPitchRadians))
				{
					StopWalking();
					break;
				}

				// Flatten the input vector
				WalkingForward.Z = 0.0f;
				WalkingForward = WalkingForward.GetSafeNormal();
				FVector WalkingRight = FVector::CrossProduct(FVector::UpVector, WalkingForward);

				// Scale the input vector by the forward, backward, and strafing multipliers
				WalkingInput *= FVector(WalkingInput.X > 0.0f ? WalkingInputForwardMultiplier : WalkingInputBackwardMultiplier, WalkingInputStrafeMultiplier, 0.0f);
				WalkingInput = (WalkingInput.X * WalkingForward) + (WalkingInput.Y * WalkingRight);

				// Guard against the input vector being nullified
				if (FMath::IsNearlyZero(WalkingInput.SizeSquared()))
				{
					UE_LOG(LogTemp, All, TEXT("Input vector was again 0"));
					StopWalking();
					break;
				}

				// Apply the movement input
				StartWalking();
				UE_LOG(LogTemp, All, TEXT("Adding input"));
				AsgardVRMovementRef->AddInputVector(WalkingInput);
			}

			// Else, ensure we are not in a walking state
			else
			{
				StopWalking();
			}

			break;
		}
		case EAsgardMovementMode::C_MOVE_Flying:
		{
			// If we can fly
			if (CanFly())
			{
				switch (FlightControlMode)
				{
					case EAsgardFlightControlMode::AnalogStick:
					{
						// Calculate the flight input vector
						FVector FlightInput = FVector::ZeroVector;
						if (FlightAnalogAxisWeightMode == EAsgardInputAxisWeightMode::Circular)
						{
							FlightInput = CalculateCircularInputVector(
								MoveForwardAxis,
								MoveRightAxis,
								FlightAnalogInputDeadZone,
								FlightAnalogInputMaxZone);
						}
						else
						{
							FlightInput = CalculateCrossInputVector(
								MoveForwardAxis,
								MoveRightAxis,
								FlightAnalogInputDeadZone,
								FlightAnalogInputMaxZone);
						}

						// Guard against no input
						if (FlightInput.IsNearlyZero())
						{
							break;
						}

						// Get the forward and right flight vectors
						FVector FlightForward = FlightAnalogOrientationComponent->GetForwardVector();
						FVector FlightRight = FlightAnalogOrientationComponent->GetRightVector();

						// Scale the input vector by the forward, backward, and strafing multipliers
						FlightInput *= FVector(FlightInput.X > 0.0f ? FlightAnalogInputForwardMultiplier : FlightAnalogInputBackwardMultiplier, FlightAnalogInputStrafeMultiplier, 0.0f);
						FlightInput = (FlightInput.X * FlightForward) + (FlightInput.Y * FlightRight);

						// Apply movement input
						AsgardVRMovementRef->AddInputVector(FlightInput);

						break;
					}

					// Controller thrusters mode
					case EAsgardFlightControlMode::ControllerThrusters:
					{
						FVector ProjectedPoint;

						// If we are in thruster control mode, and auto-stop flight is enabled, stop flying
						if (bShouldThrustersAutoStopFlying
							&& !bIsFlightThrusterLeftActive
							&& !bIsFlightThrusterRightActive
							&& IsAboveNavMesh(MaxAutoStopFlyingHeight, ProjectedPoint))
						{
							AsgardVRMovementRef->SetAsgardMovementMode(EAsgardMovementMode::C_MOVE_Falling);
							break;
						}

						// Otherwise, add thrust from the left and right hand thrusters as appropriate
						if (bIsFlightThrusterLeftActive && bIsFlightThrusterRightActive)
						{
							AsgardVRMovementRef->AddInputVector(LeftMotionController->GetForwardVector() + RightMotionController->GetForwardVector());
						}
						else
						{
							if (bIsFlightThrusterLeftActive)
							{
								AsgardVRMovementRef->AddInputVector(LeftMotionController->GetForwardVector());
							}
							if (bIsFlightThrusterRightActive)
							{
								AsgardVRMovementRef->AddInputVector(RightMotionController->GetForwardVector());
							}
						}
						break;
					}
				}


			}

			// Otherwise, change movement mode to walking
			else
			{
				AsgardVRMovementRef->SetAsgardMovementMode(EAsgardMovementMode::C_MOVE_Falling);
			}

			break;
		}

		default:
		{
			//ensureMsgf(false, TEXT("Invalid movement mode! (AsgardVRCharacter, Actor %s, UpdateMovement)"), *GetNameSafe(this));
			break;
		}
	}

	return;
}

const FVector AAsgardVRCharacter::CalculateCircularInputVector(float AxisX, float AxisY, float DeadZone, float MaxZone) const
{
	ensureMsgf(DeadZone < 1.0f && DeadZone >= 0.0f, 
		TEXT("Invalid Dead Zone for calculating an input vector! (AsgardVRCharacter, Actor %s, Value %f, CalculateCircularInputVecto)"), 
		*GetNameSafe(this), DeadZone);

	ensureMsgf(MaxZone <= 1.0f && MaxZone >= 0.0f,
		TEXT("Invalid Max Zone for calculating an input vector! (AsgardVRCharacter, Actor %s, Value %f, CalculateCircularInputVector)"),
		*GetNameSafe(this),MaxZone);

	ensureMsgf(MaxZone > DeadZone,
		TEXT("Max Zone must be greater than Dead Zone for calculating an input vector! (AsgardVRCharacter, Actor %s, Max Zone %f, Dead Zone %f, CalculateCircularInputVector)"),
		*GetNameSafe(this), MaxZone, DeadZone);

	// Cache the size of the input vector
	FVector AdjustedInputVector = FVector(AxisX, AxisY, 0.0f);
	float InputVectorLength = AdjustedInputVector.SizeSquared();

	// If the size of the input vector is greater than the deadzone
	if (InputVectorLength > DeadZone * DeadZone)
	{
		// Normalize the size of the vector to the remaining space between the deadzone and the maxzone
		AdjustedInputVector.Normalize();
		if (InputVectorLength < MaxZone * MaxZone)
		{
			InputVectorLength = FMath::Sqrt(InputVectorLength);
			AdjustedInputVector *= (InputVectorLength - DeadZone) / (MaxZone - DeadZone);
		}
		return AdjustedInputVector;
	}
	else
	{
		return FVector::ZeroVector;
	}
}

const FVector AAsgardVRCharacter::CalculateCrossInputVector(float AxisX, float AxisY, float DeadZone, float MaxZone) const
{
	ensureMsgf(DeadZone < 1.0f && DeadZone >= 0.0f,
		TEXT("Invalid Dead Zone for calculating an input vector! (AsgardVRCharacter, Actor %s, Value %f, CalculateCrossInputVector)"),
		*GetNameSafe(this), DeadZone);

	ensureMsgf(MaxZone <= 1.0f && MaxZone >= 0.0f,
		TEXT("Invalid Max Zone for calculating an input vector! (AsgardVRCharacter, Actor %s, Value %f, CalculateCrossInputVector)"),
		*GetNameSafe(this), MaxZone);

	ensureMsgf(MaxZone > DeadZone,
		TEXT("Max Zone must be greater than Dead Zone an input vector! (AsgardVRCharacter, Actor %s, Max Zone %f, Dead Zone %f, CalculateCrossInputVector)"),
		*GetNameSafe(this), MaxZone, DeadZone);

	// If the X axis is greater than the dead zone
	float AdjustedAxisX = FMath::Abs(AxisX);
	if (AdjustedAxisX > DeadZone)
	{
		// Normalize the X axis to the space between the dead zone and max zone
		if (AdjustedAxisX < MaxZone)
		{
			AdjustedAxisX = ((AdjustedAxisX - DeadZone) / (MaxZone - DeadZone)) * FMath::Sign(AxisX);
		}
		else
		{
			AdjustedAxisX = 1.0f;
		}
	}

	// Otherwise, X axis is 0
	else
	{
		AdjustedAxisX = 0.0f;
	}

	// If the Y axis is greater than the dead zone
	float AdjustedAxisY = FMath::Abs(AxisY);
	if (AdjustedAxisY > DeadZone)
	{
		// Normalize the Y axis to the space between the dead zone and max zone
		if (AdjustedAxisY < MaxZone)
		{
			AdjustedAxisY = ((AdjustedAxisY - DeadZone) / (MaxZone - DeadZone)) * FMath::Sign(AxisY);
		}
		else
		{
			AdjustedAxisY = 1.0f;
		}
	}

	// Otherwise, Y axis is 0
	else
	{
		AdjustedAxisY = 0.0f;
	}

	FVector RetVal = FVector(AdjustedAxisX, AdjustedAxisY, 0.0f);
	return RetVal;
}

void AAsgardVRCharacter::StartWalking()
{
	if (!bIsWalking)
	{
		bIsWalking = true;
		OnWalkingStarted();
	}
	return;
}

void AAsgardVRCharacter::StopWalking()
{
	if (bIsWalking)
	{
		bIsWalking = false;
		OnWalkingStopped();
	}

	return;
}

