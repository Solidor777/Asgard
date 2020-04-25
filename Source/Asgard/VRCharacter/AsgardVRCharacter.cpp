// Copyright © 2020 Justin Camden All Rights Reserved


#include "AsgardVRCharacter.h"
#include "AsgardVRMovementComponent.h"
#include "Asgard/Core/AsgardInputBindings.h"
#include "Asgard/Core/AsgardCollisionProfiles.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/Engine/Public/TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogAsgardVRCharacter, Log, All);


AAsgardVRCharacter::AAsgardVRCharacter(const FObjectInitializer& ObjectInitializer)
:Super(ObjectInitializer.SetDefaultSubobjectClass<UAsgardVRMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Navigation settings
	NavQueryExtent = FVector(150.f, 150.f, 150.f);

	// Walk Settings
	bWalkEnabled = true;
	WalkInputDeadZone = 0.15f;
	WalkInputMaxZone = 0.9f;
	WalkInputForwardMultiplier = 1.0f;
	WalkInputStrafeMultiplier = 1.0f;
	WalkInputBackwardMultiplier = 1.0f;
	WalkOrientationMaxAbsPitchPiRadians = 0.9f;

	// Flight settings
	bFlightEnabled = true;
	bFlightAutoLandingEnabled = true;
	FlightAutoLandingMaxHeight = 50.0f;
	FlightAutoLandingMaxAnglePiRadians = 0.15f;
	bIsFlightAnalogMovementEnabled = true;
	FlightAnalogInputDeadZone = 0.15f;
	FlightAnalogInputMaxZone = 0.9f;
	FlightAnalogInputForwardMultiplier = 1.0f;
	FlightAnalogInputStrafeMultiplier = 1.0f;
	FlightAnalogInputBackwardMultiplier = 1.0f;
	bShouldFlightControllerThrustersAutoStartFlight = true;
	FlightControllerThrusterInputMultiplier = 1.0f;

	// Movement state
	bIsWalk = false;

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
	SetWalkOrientationMode(WalkOrientationMode);
	SetFlightAnalogOrientationMode(FlightAnalogOrientationMode);

	// Cache the navdata
	CacheNavData();

	Super::BeginPlay();
}

void AAsgardVRCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bIsTeleportingToLocation)
	{
		if (CurrentTeleportToLocationMode >= EAsgardTeleportMode::ContinuousRate)
		{
			UpdateContinuousTeleportToLocation(DeltaSeconds);
		}
	}
	else if (bIsTeleportingToRotation)
	{
		if (CurrentTeleportToRotationMode >= EAsgardTeleportMode::ContinuousRate)
		{
			UpdateContinuousTeleportToRotation(DeltaSeconds);
		}
	}
	else
	{
		UpdateMovement();
	}

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

void AAsgardVRCharacter::SetWalkOrientationMode(const EAsgardWalkOrientationMode NewWalkOrientationMode)
{
	WalkOrientationMode = NewWalkOrientationMode;
	switch (WalkOrientationMode)
	{
		case EAsgardWalkOrientationMode::LeftController:
		{
			WalkOrientationComponent = LeftMotionController;
			break;
		}
		case EAsgardWalkOrientationMode::RightController:
		{
			WalkOrientationComponent = RightMotionController;
			break;
		}
		case EAsgardWalkOrientationMode::VRCamera:
		{
			WalkOrientationComponent = VRReplicatedCamera;
			break;
		}
		case EAsgardWalkOrientationMode::CharacterCapsule:
		{
			WalkOrientationComponent = VRRootReference;
			break;
		}
		default:
		{
			break;
		}
	}

	return;
}

void AAsgardVRCharacter::SetFlightAnalogOrientationMode(const EAsgardFlightAnalogOrientationMode NewFlightOrientationMode)
{
	FlightAnalogOrientationMode = NewFlightOrientationMode;
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
			break;
		}
	}

	return;
}

bool AAsgardVRCharacter::MoveActionTeleportToLocation(const FVector& GoalLocation, EAsgardTeleportMode Mode)
{
	if (CanTeleport())
	{
		// Cache teleport variables
		TeleportStartLocation = GetVRLocation();
		TeleportStartLocation.Z -= GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		TeleportGoalLocation = GoalLocation;
		CurrentTeleportToLocationMode = Mode;

		// Depending on the teleport mode
		switch (CurrentTeleportToLocationMode)
		{
			case EAsgardTeleportMode::Fade:
			{
				bIsTeleportingToLocation = true;
				OnTeleportFadeOutFinished.BindUFunction(this, FName("DeferredTeleportToLocation"));
				OnTeleportFadeInFinished.BindUFunction(this, FName("DefferedTeleportToLocationFinished"));
				bTeleportToLocationSucceeded = true;
				TeleportFadeOut();
				break;
			}
			case EAsgardTeleportMode::Instant:
			{
				bTeleportToLocationSucceeded = TeleportTo(GetTeleportLocation(TeleportGoalLocation), GetActorRotation());
				if (!bTeleportToLocationSucceeded)
				{
					UE_LOG(LogAsgardVRCharacter, Error, TEXT("MoveActionTeleportToLocation failed! TeleportTo returned false."));
				}
				break;
			}
			// Continuous cases
			default:
			{
				checkf(ContinuousTeleportToRotationSpeed > 0.0f,
					TEXT("Invalid ContinuousTeleportToLocationSpeed! Must be > 0.0! (AsgardVRCharacter, Value %f, MoveActionTeleportToLocation)"),
					ContinuousTeleportToLocationSpeed);
				if (CurrentTeleportToLocationMode == EAsgardTeleportMode::ContinuousRate)
				{
					TeleportToLocationContinuousMultiplier = 1.0f / ((TeleportGoalLocation - TeleportStartLocation).Size() / ContinuousTeleportToLocationSpeed);
				}
				else
				{
					TeleportToLocationContinuousMultiplier = 1.0f / ContinuousTeleportToLocationSpeed;
				}
				TeleportToLocationContinuousProgress = 0.0f;
				SetActorEnableCollision(false);
				bIsTeleportingToLocation = true;
				bTeleportToLocationSucceeded = true;
				break;
			}
		}

		return bTeleportToLocationSucceeded;
	}

	return false;
}

void AAsgardVRCharacter::MoveActionTeleportToRotation(const FRotator& GoalRotation, EAsgardTeleportMode Mode)
{
	if (CanTeleport())
	{
		// Cache teleport variables
		CurrentTeleportToRotationMode = Mode;
		TeleportStartRotation = GetVRRotation();
		TeleportGoalRotation = GoalRotation;

		// Depending on teleport mode
		switch (CurrentTeleportToRotationMode)
		{
			case EAsgardTeleportMode::Fade:
			{
				bIsTeleportingToRotation = true;
				OnTeleportFadeOutFinished.BindUFunction(this, FName("DeferredTeleportToLocation"));
				OnTeleportFadeInFinished.BindUFunction(this, FName("DefferedTeleportToLocationFinished"));
				TeleportFadeOut();
				break;
			}
			case EAsgardTeleportMode::Instant:
			{
				SetActorRotationVR(TeleportGoalRotation);
				break;
			}
			// Continuous cases
			default:
			{
				checkf(ContinuousTeleportToRotationSpeed > 0.0f,
					TEXT("Invalid ContinuousTeleportToRotationSpeed! Must be > 0.0! (AsgardVRCharacter, Value %f, MoveActionTeleportToRotation)"),
					ContinuousTeleportToRotationSpeed);
				if (CurrentTeleportToRotationMode == EAsgardTeleportMode::ContinuousRate)
				{
					TeleportToRotationContinuousMultiplier = 1.0f / ( (TeleportGoalRotation - TeleportStartRotation).GetNormalized().Yaw / ContinuousTeleportToRotationSpeed);
				}
				else
				{
					TeleportToLocationContinuousMultiplier = 1.0f / ContinuousTeleportToRotationSpeed;
				}
				TeleportToRotationContinuousProgress = 0.0f;
				bIsTeleportingToRotation = true;
				break;
			}
		}
	}

	return;
}

void AAsgardVRCharacter::TeleportFadeOut_Implementation()
{
	if (TeleportFadeOutTime > 0.0f)
	{
		FTimerHandle TempTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, this, &AAsgardVRCharacter::TeleportFadeOutFinished, TeleportFadeOutTime);
	}
	else
	{
		TeleportFadeOutFinished();
	}
	return;
}

void AAsgardVRCharacter::TeleportFadeOutFinished()
{
	if (OnTeleportFadeOutFinished.IsBound())
	{
		OnTeleportFadeOutFinished.Execute();
		OnTeleportFadeOutFinished.Unbind();
	}
	TeleportFadeIn();
	return;
}

void AAsgardVRCharacter::TeleportFadeIn_Implementation()
{
	if (TeleportFadeInTime > 0.0f)
	{
		FTimerHandle TempTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, this, &AAsgardVRCharacter::TeleportFadeInFinished, TeleportFadeOutTime);
	}
	else
	{
		TeleportFadeInFinished();
	}
	return;
}


void AAsgardVRCharacter::TeleportFadeInFinished()
{
	if (OnTeleportFadeInFinished.IsBound())
	{
		OnTeleportFadeInFinished.Execute();
		OnTeleportFadeInFinished.Unbind();
	}
	return;
}

void AAsgardVRCharacter::DeferredTeleportToLocation()
{
	if (!TeleportTo(GetTeleportLocation(TeleportGoalLocation), GetActorRotation()))
	{
		UE_LOG(LogAsgardVRCharacter, Error, TEXT("DeferredTeleportToLocation failed! TeleportTo returned false."));
		bTeleportToLocationSucceeded = false;
	}
	return;
}

void AAsgardVRCharacter::DeferredTeleportToLocationFinished()
{
	bIsTeleportingToLocation = false;
	return;
}

void AAsgardVRCharacter::DeferredTeleportToRotation()
{
	SetActorRotationVR(TeleportGoalRotation);
	return;
}

void AAsgardVRCharacter::DeferredTeleportToRotationFinished()
{
	bIsTeleportingToRotation = false;
	return;
}

void AAsgardVRCharacter::UpdateContinuousTeleportToLocation(float DeltaSeconds)
{
	// Progress interpolation
	TeleportToLocationContinuousProgress = FMath::Min
	(TeleportToLocationContinuousProgress + (TeleportToLocationContinuousMultiplier * DeltaSeconds), 
		1.0f);

	// Continue interpolation
	if (TeleportToLocationContinuousProgress < 1.0f)
	{

		SetActorLocation(FMath::Lerp(
			GetTeleportLocation(TeleportStartLocation),
			GetTeleportLocation(TeleportGoalLocation),
			TeleportToLocationContinuousProgress));
	}

	// Finish interpolation
	else
	{
		SetActorLocation(GetTeleportLocation(TeleportGoalLocation));
		SetActorEnableCollision(true);
		bIsTeleportingToLocation = false;
	}

	return;
}

void AAsgardVRCharacter::UpdateContinuousTeleportToRotation(float DeltaSeconds)
{
	// Progress interpolation
	TeleportToRotationContinuousProgress = FMath::Min
	(TeleportToRotationContinuousProgress + (TeleportToRotationContinuousMultiplier * DeltaSeconds),
		1.0f);

	// Continue interpolation
	if (TeleportToLocationContinuousProgress < 1.0f)
	{
		SetActorRotationVR(FMath::Lerp(
			TeleportStartRotation, 
			TeleportGoalRotation, 
			TeleportToRotationContinuousProgress));
	}

	// Finish interpolation
	else
	{
		SetActorRotationVR(TeleportGoalRotation);
		bIsTeleportingToRotation = false;
	}
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

bool AAsgardVRCharacter::CanTeleport_Implementation()
{
	return bTeleportEnabled && !bIsTeleportingToLocation;
}

bool AAsgardVRCharacter::CanWalk_Implementation()
{
	return bWalkEnabled;
}

bool AAsgardVRCharacter::CanFly_Implementation()
{
	return bFlightEnabled && (bIsFlightAnalogMovementEnabled || bFlightControllerThrustersEnabled);
}

bool AAsgardVRCharacter::ProjectCharacterToVRNavigation(float MaxHeight, FVector& OutProjectedPoint)
{
	// Perform a trace from the bottom of the capsule collider towards the ground
	FVector TraceOrigin = GetCapsuleComponent()->GetComponentLocation();
	FVector TraceEnd = TraceOrigin;
	TraceEnd.Z -= (MaxHeight + GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
	FHitResult TraceHitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("AsgardVRCharacterAboveNavMeshTrace")), false, this);
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

		// If point projected successfully and we want to check the point is on the ground
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
	// Branch depending on current movement mode
	switch (AsgardVRMovementRef->GetAsgardMovementMode())
	{
		case EAsgardMovementMode::C_MOVE_Walking:
		{
			// If we can walk
			if (CanWalk())
			{
				// Get the input vector depending on the walking weight mode
				FVector WalkInput = FVector::ZeroVector;
				if (WalkInputAxisWeightMode == EAsgardInputAxisWeightMode::Circular)
				{
					WalkInput = CalculateCircularInputVector(
						MoveForwardAxis,
						MoveRightAxis,
						WalkInputDeadZone,
						WalkInputMaxZone);
				}
				else
				{
					WalkInput = CalculateCrossInputVector(
						MoveForwardAxis,
						MoveRightAxis,
						WalkInputDeadZone,
						WalkInputMaxZone);
				}

				// Guard against no input
				if (WalkInput.IsNearlyZero())
				{
					StopWalk();
					break;
				}

				// Transform the input vector from the forward of the orientation component
				FVector WalkForward = WalkOrientationComponent->GetForwardVector();

				// Guard against the forward vector being nearly up or down
				if (FMath::Abs(FVector::UpVector | WalkForward) > (WalkOrientationMaxAbsPitchPiRadians))
				{ 
					StopWalk();
					break;
				}

				// Flatten the input vector
				WalkForward.Z = 0.0f;
				WalkForward = WalkForward.GetSafeNormal();
				FVector WalkRight = FVector::CrossProduct(FVector::UpVector, WalkForward);

				// Scale the input vector by the forward, backward, and strafing multipliers
				WalkInput *= FVector(WalkInput.X > 0.0f ? WalkInputForwardMultiplier : WalkInputBackwardMultiplier, WalkInputStrafeMultiplier, 0.0f);
				WalkInput = (WalkInput.X * WalkForward) + (WalkInput.Y * WalkRight);

				// Guard against the input vector being nullified
				if (FMath::IsNearlyZero(WalkInput.SizeSquared()))
				{
					StopWalk();
					break;
				}

				// Apply the movement input
				StartWalk();
				AsgardVRMovementRef->AddInputVector(WalkInput);
			}

			// Else, check we are not in a walking state
			else
			{
				StopWalk();
			}

			break;
		}
		case EAsgardMovementMode::C_MOVE_Flying:
		{
			// If we can fly
			if (CanFly())
			{
				// Cache initial variables
				bool bShouldApplyInput = false;
				FVector FlightInput = FVector::ZeroVector;

				// Analog stick input
				if (bFlightAutoLandingEnabled)
				{
					// Calculate the Flight input vector
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
					if (!FlightInput.IsNearlyZero())
					{
						bShouldApplyInput = true;

						// Get the forward and right Flight vectors
						FVector FlightForward = FlightAnalogOrientationComponent->GetForwardVector();
						FVector FlightRight = FlightAnalogOrientationComponent->GetRightVector();

						// Scale the input vector by the forward, backward, and strafing multipliers
						FlightInput *= FVector(FlightInput.X > 0.0f ? FlightAnalogInputForwardMultiplier : FlightAnalogInputBackwardMultiplier, FlightAnalogInputStrafeMultiplier, 0.0f);
						FlightInput = (FlightInput.X * FlightForward) + (FlightInput.Y * FlightRight);
					}
				}

				// Thruster input
				if (bFlightControllerThrustersEnabled)
				{
					FVector ProjectedPoint;

					// Add thrust from the left and right hand thrusters as appropriate
					if (bIsFlightThrusterLeftActive)
					{
						bShouldApplyInput = true;
						FlightInput += LeftMotionController->GetForwardVector() * FlightControllerThrusterInputMultiplier;
					}
					if (bIsFlightThrusterRightActive)
					{
						bShouldApplyInput = true;
						FlightInput += RightMotionController->GetForwardVector() * FlightControllerThrusterInputMultiplier;
					}
				}

				// Apply input if appropriate
				if (bShouldApplyInput)
				{
					VRMovementReference->AddInputVector(FlightInput);

					// If we are flying towards the ground
					if ((FVector::DownVector | FlightInput.GetSafeNormal()) <= FlightAutoLandingMaxAnglePiRadians
						&& (!bFlightAutoLandingRequiresDownwardVelocity 
							|| (FVector::DownVector | FlightInput.GetSafeNormal()) <= FlightAutoLandingMaxAnglePiRadians))
					{
						// If we are close to the ground
						FVector ProjectedPoint;
						if (ProjectCharacterToVRNavigation(FlightAutoLandingMaxHeight, ProjectedPoint))
						{
							// Stop flying
							AsgardVRMovementRef->SetAsgardMovementMode(EAsgardMovementMode::C_MOVE_Falling);
						}
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
			break;
		}
	}

	return;
}

const FVector AAsgardVRCharacter::CalculateCircularInputVector(float AxisX, float AxisY, float DeadZone, float MaxZone) const
{
	checkf(DeadZone < 1.0f && DeadZone >= 0.0f, 
		TEXT("Invalid Dead Zone for calculating an input vector! (AsgardVRCharacter, Value %f, CalculateCircularInputVector)"), 
		DeadZone);

	checkf(MaxZone <= 1.0f && MaxZone >= 0.0f,
		TEXT("Invalid Max Zone for calculating an input vector! (AsgardVRCharacter, Value %f, CalculateCircularInputVector)"),
		MaxZone);

	checkf(MaxZone > DeadZone,
		TEXT("Max Zone must be greater than Dead Zone for calculating an input vector! (AsgardVRCharacter, Max Zone %f, Dead Zone %f, CalculateCircularInputVector)"),
		MaxZone, DeadZone);

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
	checkf(DeadZone < 1.0f && DeadZone >= 0.0f,
		TEXT("Invalid Dead Zone for calculating an input vector! (AsgardVRCharacter, Value %f, CalculateCrossInputVector)"),
		DeadZone);

	checkf(MaxZone <= 1.0f && MaxZone >= 0.0f,
		TEXT("Invalid Max Zone for calculating an input vector! (AsgardVRCharacter, Value %f, CalculateCrossInputVector)"),
		MaxZone);

	checkf(MaxZone > DeadZone,
		TEXT("Max Zone must be greater than Dead Zone an input vector! (AsgardVRCharacter, Max Zone %f, Dead Zone %f, CalculateCrossInputVector)"),
		MaxZone, DeadZone);

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

void AAsgardVRCharacter::StartWalk()
{
	if (!bIsWalk)
	{
		bIsWalk = true;
		OnWalkStarted();
	}
	return;
}

void AAsgardVRCharacter::StopWalk()
{
	if (bIsWalk)
	{
		bIsWalk = false;
		OnWalkStopped();
	}

	return;
}

