// Copyright © 2020 Justin Camden All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "VRCharacter.h"
#include "AsgardVRCharacter.generated.h"

// Delegates
DECLARE_DELEGATE(FOnTeleportFadeOutFinished)
DECLARE_DELEGATE(FOnTeleportFadeInFinished)
DECLARE_MULTICAST_DELEGATE(FOnTeleportToRotationFinished)
DECLARE_MULTICAST_DELEGATE(FOnTeleportToLocationFinished)

// Stats
DECLARE_STATS_GROUP(TEXT("AsgardVRCharacter"), STATGROUP_ASGARD_VRCharacter, STATCAT_Advanced);

UENUM(BlueprintType)
enum class EAsgardTeleportMode: uint8
{
	Fade,
	Instant,
	SmoothRate,
	SmoothTime
};

UENUM(BlueprintType)
enum class EAsgardGroundMovementMode : uint8
{
	PrecisionTeleportToLocation,
	TeleportWalk,
	SmoothWalk
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

UENUM(BlueprintType)
enum class EAsgardInputAxisWeightMode: uint8
{
	Circular,
	Cross
};

UENUM(BlueprintType)
enum class EAsgardInputSource : uint8
{
	Dedicated,
	Inferred1,
	Inferred2
};

UENUM(BlueprintType)
enum class EAsgard2DAxisState : uint8
{
	Neutral,
	Up,
	Down,
	Left,
	Right
};

// Forward declarations
class UAsgardVRMovementComponent;


/**
 * Base class for the player avatar
 */
UCLASS()
class ASGARD_API AAsgardVRCharacter : public AVRCharacter
{
	GENERATED_BODY()

public:
	// Overrides
	AAsgardVRCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// ---------------------------------------------------------
	//	Input

	// Axis events
	void OnMoveForwardAxis(float Value);
	void OnMoveRightAxis(float Value);
	void OnTurnRightAxis(float Value);

	// Action events
	void OnFlightThrusterLeftActionPressed();
	void OnFlightThrusterLeftActionReleased();
	void OnFlightThrusterRightActionPressed();
	void OnFlightThrusterRightActionReleased();
	void OnToggleFlightActionPressed();
	void OnPrecisionTeleportActionPressed();
	void OnPrecisionTeleportActionReleased();

	// Axis actions
	void OnTurnRightActionPressed();
	void OnTurnRightActionReleased();
	void OnTurnLeftActionPressed();
	void OnTurnLeftActionReleased();
	void OnMoveActionPressed();
	void OnMoveActionReleased();

	/**
	* Deferred teleport functions for binding to delegates.
	*/
	UFUNCTION()
	void DeferredTeleportToLocation();
	UFUNCTION()
	void DeferredTeleportToLocationFinished();
	UFUNCTION()
	void DeferredTeleportToRotation();
	UFUNCTION()
	void DeferredTeleportToRotationFinished();
	UFUNCTION()
	void DeferredTeleportToLocationAndRotation();
	UFUNCTION()
	void DeferredTeleportToLocationAndRotationFinished();
	UFUNCTION()
	void DeferredTeleportTurnRightFinished();
	UFUNCTION()
	void DeferredTeleportTurnLeftFinished();
	UFUNCTION()
	void DeferredTeleportWalkFinished();

	/**
	* The minimum absolute value of an axis to trigger a pressed angle.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Input", meta = (ClampMin = "0.01"))
	float AxisPressedMinAbsValue;

	/**
	* The maximum angle from an axis an analog stick can be pressed to trigger a pressed action.
	* Ignored if <= 0.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Input", meta = (ClampMax = "22.5"))
	float AxisPressedMaxAngle;

	/**
	* Sets the PrecisionTeleportOrientationComponent to match the orientation mode.
	*/
	UFUNCTION(BlueprintCallable, Category = "AsgardVRCharacter|Movement|PrecisionTeleport")
	void SetPrecisionTeleportOrientationComponent(EAsgardBinaryHand NewPrecisionTeleportOrientationMode);

	/**
	* Setter for WalkOrientationMode.
	* Sets the WalkOrientationComponent to match orientation mode.
	* Sets the component to null for Character mode.
	*/
	UFUNCTION(BlueprintCallable, Category = "AsgardVRCharacter|Movement")
	void SetWalkOrientationMode(const EAsgardOrientationMode NewWalkOrientationMode);

	/**
	* Setter for FlightOrientationMode.
	* Sets the FlightOrientationComponent to match orientation mode.
	* This is only used with analog stick style flying.
	*/
	UFUNCTION(BlueprintCallable, Category = "AsgardVRCharacter|Movement|SmoothFlight")
	void SetSmoothFlightOrientationMode(EAsgardFlightOrientationMode NewSmoothFlightOrientationMode);

	// ---------------------------------------------------------
	//	General movement settings

	/**
	* Whether movement using the default ground movement mode is enabled for this character.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement")
	uint32 bDefaultGroundMovementEnabled : 1;

	/**
	* Whether the move axis should start a precision teleport, perform short, repeated teleports, or smoothly walk using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement")
	EAsgardGroundMovementMode DefaultGroundMovementMode;

	/**
	* The absolute world pitch angle of forward vector of the movement orientation component must be less than or equal to this many degrees to walk.
	* Otherwise, input will be applied.
	* Only used when orientation mode is not Character
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|SmoothWalk", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float WalkForwardMaxAbsPitch;

	/**
	* The filter class when searching for a place on the navmesh this character can stand on or pass through.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement")
	TSubclassOf<UNavigationQueryFilter> NavQueryFilter;

	/**
	* The extent of the query when searching for a place on the navmesh this character can stand on.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement")
	FVector NavQueryExtent;


	// ---------------------------------------------------------
	//	Universal teleport settings
	/**
	* Whether teleporting is enabled for the character.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport")
	uint32 bTeleportEnabled : 1;

	/** 
	* How long  it takes to fade out when teleporting using Fade teleport mode.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport")
	float TeleportFadeOutTime;

	/**
	* How long  it takes to fade in when teleporting using Fade teleport mode.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport")
	float TeleportFadeInTime;

	/**
	* The method of teleportation used for teleporting to a location.
	* Instant: Character is instantly placed at their goal location.
	* Blink: Brief delay, before the character is placed at their goal location.
	* SmoothRate: Character is smoothly interpolated to their goal location at a given rate.
	* SmoothTime: Character is smoothly interpolated to their goal location over a given time.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport")
	EAsgardTeleportMode TeleportToLocationDefaultMode;

	/**
	* The method of teleportation used for teleporting to a specific Rotation.
	* Instant: Character is instantly placed at their goal Rotation.
	* Blink: Brief delay, before the character is placed at their goal Rotation.
	* SmoothRate: Character is smoothly interpolated to their goal Rotation at a given rate.
	* SmoothTime: Character is smoothly interpolated to their goal Rotation over a given time.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport")
	EAsgardTeleportMode TeleportToRotationDefaultMode;

	/**
	* Radius of the trace when searching for a teleport location.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport")
	float TeleportTraceRadius;

	/**
	* Maximum simulation time for tracing a teleport location.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport")
	float TeleportTraceMaxSimTime;

	/**
	* The speed of the character when teleporting to a location in a Smooth teleport mode.
	* SmoothRate: Treated as centimeters per second.
	* SmoothTime: Treated treated as how long each teleport will take.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport", meta = (ClampMin = "0.01"))
	float SmoothTeleportToLocationSpeed;

	/**
	* The speed of the character when teleporting to a rotation in a Smooth teleport mode.
	* SmoothRate: Treated as degrees per second.
	* SmoothTime: Treated treated as how long each teleport will take.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Teleport", meta = (ClampMin = "0.01"))
	float SmoothTeleportToRotationSpeed;


	// ---------------------------------------------------------
	//	Precision Teleport settings

	/**
	* Whether precision teleport requires navmesh path to the the target location.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|PrecisionTeleport")
	uint32 bPrecisionTeleportLocationRequiresNavmeshPath : 1;

	/**
	* The trace direction of a Precision Teleport will be lerped from the last Direction towards the forward of the Orientation Component.
	* This will be the maximum angle the trace direction can be forward the Orientation Components forward.
	* If set to 0, the trace direction will not lerp and will be instantly set to the Orientation Component's forward.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PrecisionTeleportTraceDirectionMaxErrorAngle;

	/**
	* The trace direction of a Precision Teleport will be lerped from the last Direction towards the forward of the Orientation Component.
	* This is how many angles per second the trace direction will lerp.
	* If set to 0, the trace direction will not lerp and will be instantly set to the Orientation Component's forward.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|PrecisionTeleport")
	float PrecisionTeleportTraceDirectionAngleLerpSpeed;

	/**
	* The maximum pitch angle of a Precision Teleport.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	float PrecisionTeleportTraceDirectionMaxPitch;

	/**
	* The trace magnitude of Precision Teleport.
	* The Precision Teleport Trace Direction multiplied by this number to calculate the arc of the trace.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (ClampMin = "0.01"))
	float PrecisionTeleportTraceMagnitude;

	/**
	* Trace channel use for Precision Teleport traces.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|PrecisionTeleport")
	TEnumAsByte<ECollisionChannel> PrecisionTeleportTraceChannel;


	// ---------------------------------------------------------
	//	Teleport turn settings

	/**
	* How many degrees to turn when performing a teleport turn.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|TeleportTurn", meta = (ClampMin = "0.01"))
	float TeleportTurnAngleInterval;

	/**
	* Interval between teleport turn events.
	* If <= 0 an event will not be triggered.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|TeleportTurn")
	float TeleportTurnHeldInterval;


	// ---------------------------------------------------------
	//	Teleport Walk settings
	
	/**
	* The minimum value of the combined MoveForwardAxis and MoveRightAxis before the Teleport Walk action can be considered pressed.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|TeleportWalk")
	float TeleportWalkActionPressedThreshold;

	/**
	* Interval between Teleport Walk events.
	* If <= 0 an event will not be triggered.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|TeleportWalk")
	float TeleportWalkHeldInterval;

	/**
    * The magnitude of velocity of a Teleport Walk.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|TeleportWalk")
	float TeleportWalkMagnitude;

	/** 
	* If the trace portion of the teleport walk fails, it will be attempted again this number of times, 
	* reducing the magnitude of the trace each time.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|TeleportWalk")
	int32 TeleportWalkMaxFallbackTraces;

	/**
	* If the trace portion of the teleport walk fails and a fallback trace is initiated, 
	* the magnitude of the fallback trace will be reduced by this magnitude each time.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|TeleportWalk")
	int32 TeleportWalkFallbackTraceMagnitudeInterval;

	/**
	* Trace channel use for Teleport Walk traces.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|TeleportWalk")
	TEnumAsByte<ECollisionChannel> TeleportWalkTraceChannel;

		
	// ---------------------------------------------------------
	//	Smooth Walk settings

	/**
	* The axis weight mode for the character when walking using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|SmoothWalk")
	EAsgardInputAxisWeightMode SmoothWalkInputAxisWeightMode;

	/**
	* The dead zone of the input vector when walking with using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|SmoothWalk")
	float SmoothWalkInputDeadZone;

	/**
	* The max zone of the input vector when walking with using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|SmoothWalk")
	float SmoothWalkInputMaxZone;

	/**
	* Multiplier that will be applied to the forward or back input vector when walking using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|SmoothWalk", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SmoothWalkInputForwardMultiplier;

	/**
	* Multiplier that will be applied to the forward or back input vector when walking using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|SmoothWalk", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SmoothWalkInputBackwardMultiplier;

	/**
	* Multiplier that will be applied to the right or left input vector when strafing using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|SmoothWalk", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SmoothWalkInputStrafeMultiplier;


	// ---------------------------------------------------------
	//	Flight settings

	/**
	* Whether flying is enabled for the character.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight")
	uint32 bFlightEnabled : 1;

	/**
	* Whether controller thruster movement is enabled for Flight.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight")
	uint32 bFlightControllerThrustersEnabled : 1;

	/**
	* Whether to the player should automatically take off and start flying by engaging the thrusters when walking on the ground.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight")
	uint32 bShouldFlightControllerThrustersAutoStartFlight : 1;

	/**
	* Multiplier applied to controller thruster movement input when flying.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight")
	float FlightControllerThrusterInputMultiplier;


	// ---------------------------------------------------------
	//	Flight auto landing settings

	/**
	* Whether the player should automatically land and stop flying when attempting to move into the ground.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|AutoLanding")
	uint32 bFlightAutoLandingEnabled : 1;

	/**
	* Whether auto landing requires velocity as well as desired input to be facing the ground.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|AutoLanding")
	uint32 bFlightAutoLandingRequiresDownwardVelocity : 1;

	/**
	* The maximum height above navigable ground the player can be to automatically land by flying into the ground.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|AutoLanding")
	float FlightAutoLandingMaxHeight;

	/**
	* The minimum pitch of the players motion vector to automatically land by flying into the ground.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|AutoLanding")
	float FlightAutoLandingMinPitch;


	// ---------------------------------------------------------
	//	Smooth Flight control  settings

	/**
	* Whether analog stick movement is enabled for Flight.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|SmoothFlight")
	uint32 bIsSmoothFlightMovementEnabled : 1;

	/**
	* The axis weight mode for the character when flying and moving using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|SmoothFlight")
	EAsgardInputAxisWeightMode SmoothFlightAxisWeightMode;

	/**
	* The dead zone of the input vector when flying and moving using the analog stick
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|SmoothFlight")
	float SmoothFlightInputDeadZone;

	/**
	* The max zone of the input vector when flying and moving using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|SmoothFlight")
	float SmoothFlightInputMaxZone;

	/**
	* Multiplier that will be applied to the forward / backward input vector when flying and moving forward using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|SmoothFlight")
	float SmoothFlightInputForwardMultiplier;

	/**
	* Multiplier that will be applied to the forward / backward input vector when flying and moving backward using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|SmoothFlight")
	float SmoothFlightInputBackwardMultiplier;

	/**
	* Multiplier that will be applied to the right / left input vector when flying and moving using the analog stick.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AsgardVRCharacter|Movement|Flight|SmoothFlight")
	float SmoothFlightInputStrafeMultiplier;


	// ---------------------------------------------------------
	//	Teleport functions

	/**
	* Overiddable function for performing the fade out portion of a teleport.
	* If overriden, be sure to call TeleportFadeOutFinished at the end.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "AsgardVRCharacter|Movement|Teleport")
	void TeleportFadeOut();

	/**
	* Called when the fade out portion of a teleport is finished.
	*/
	UFUNCTION(BlueprintCallable, Category = "AsgardVRCharacter|Movement|Teleport")
	void TeleportFadeOutFinished();

	/**
	* Called when the fade out portion of a teleport is finished.
	*/
	FOnTeleportFadeOutFinished OnTeleportFadeOutFinished;

	/**
	* Overiddable function for performing the fade in portion of a teleport.
	* If overriden, be sure to call TeleportFadeInFinished at the end.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = "AsgardVRCharacter|Movement|Teleport")
	void TeleportFadeIn();

	/**
	* Called when the fade in portion of a teleport is finished.
	*/
	UFUNCTION(BlueprintCallable, Category = "AsgardVRCharacter|Movement|Teleport")
	void TeleportFadeInFinished();

	/**
	* Called when the fade in portion of a teleport is finished.
	*/
	FOnTeleportFadeOutFinished OnTeleportFadeInFinished;

	/**
	* Called when a teleport to location is finished.
	*/
	FOnTeleportToLocationFinished OnTeleportToLocationFinished;

	/**
	* Called when a teleport to rotation is finished.
	*/
	FOnTeleportToRotationFinished OnTeleportToRotationFinished;


protected:
	// Overrides
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/**
	* References to the movement component.
	*/
	UPROPERTY(Category = AsgardVRCharacter, VisibleAnywhere, Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAsgardVRMovementComponent* AsgardVRMovementRef;

	/**
	* Whether the character can currently teleport.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = AsgardVRCharacter)
	bool CanTeleport();

	/**
	* Whether the character can currently walk.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = AsgardVRCharacter)
	bool CanSmoothWalk();

	/**
	* Whether the character can currently fly.
	*/
	UFUNCTION(BlueprintNativeEvent, Category = AsgardVRCharacter)
	bool CanFly();

	/**
	* Called when the character starts walking on the ground.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = AsgardVRCharacter)
	void OnSmoothWalkStarted();

	/**
	* Called when the character stops walking on the ground.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = AsgardVRCharacter)
	void OnSmoothWalkStopped();

	/**
	* Returns whether the player is above the ground up to the max height.
	* If they are moving on the ground, returns true.
	* Otherwise, performs a trace down from the center of the capsule component,
	* with a length of the capsule half height + MaxHeight,
	* and attempts to project the impact point onto the NavMesh.
	*/
	bool ProjectCharacterToVRNavigation(float MaxHeight, FVector& OutProjectedPoint);

	/**
	* Called when the character starts walking on the ground.
	*/
	void StartSmoothWalk();

	/**
	* Called when the character stops walking on the ground.
	*/
	void StopSmoothWalk();

private:
	// ---------------------------------------------------------
	//	Axis Input State

	/**
	* The value of MoveForwardAxis.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	float MoveForwardAxis;

	/**
	* The value of MoveRightAxis.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	float MoveRightAxis;

	/**
	* The combined FVector made from MoveForwardAxis and MoveRightAxis.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	FVector MoveInputVector;

	/**
	* State of 2D input actions bound to the move input vector.
	*/
	EAsgard2DAxisState MoveInput2DActionState;

	/**
	* The value of TurnRightAxis.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	float TurnRightAxis;


	// ---------------------------------------------------------
	//	Action Input State

	/**
	* Whether FlightThrusterLeftAction is pressed.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	uint32 bIsFlightThrusterLeftActionPressed : 1;

	/**
	* Whether FlightThrusterRightAction is pressed.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	uint32 bIsFlightThrusterRightActionPressed : 1;

	/**
	* Whether PrecisionTeleportAction is pressed.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	uint32 bIsPrecisionTeleportActionPressed : 1;

	/**
	* Whether TurnRightAxis is pressed enough to the right to trigger a turn right event action.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	uint32 bIsTurnRightActionPressed : 1;

	/**
	* Whether TurnRightAxis is pressed enough to the left to trigger a turn left event action.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	uint32 bIsTurnLeftActionPressed : 1;

	/**
	* Whether the move input vector is pressed far enough to consider the move action pressed.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Input", meta = (AllowPrivateAccess = "true"))
	uint32 bIsMoveActionPressed : 1;

	// ---------------------------------------------------------
	//	Input functions

	/**
	* Calculates an evenly weighted, circular input vector from two directions and a deadzone.
	* Deadzone must always be less than 1 with a minimum of 0.
	* Maxzone must always be less or equal to 1 and greater or equal to 0.
	*/
	const FVector CalculateCircularInputVector(FVector RawInputVector, float DeadZone, float MaxZone) const;

	/**
	* Calculates an input vector weighted to more heavily favor cardinal directions.
	* (up, down, left, right) from two directions, a deadzone, and a maxzone.
	* Deadzone must always be less than 1 with a minimum of 0.
	* Maxzone must always be less or equal to 1 and greater or equal to 0.
	*/
	const FVector CalculateCrossInputVector(float AxisX, float AxisY, float DeadZone, float MaxZone) const;

	/**
	* Finds and caches the VR navigation data.
	*/
	void CacheNavData();

	/**
	* Projects a point to the navmesh according to the query extent and filter classes set on this actor.
	*/
	bool ProjectPointToVRNavigation(const FVector& Point, FVector& OutPoint, bool bCheckIfIsOnGround);

	/** 
	* Checks to see if a path exists to a specified point, according to the navigation settings on this actor.
	*/
	bool DoesPathToPointExistVR(const FVector& GoalLocation);

	/**
	* Updates walking input depending on current settings and player input.
	*/
	void UpdateSmoothWalk();

	/**
	* Updates flying input depending on current settings and player input.
	*/
	void UpdateSmoothFlight();

	/**
	* Setter for MoveInputVector
	*/
	void SetMoveInputVector(const FVector& NewMoveInputVector);

	/** 
	* Calculates the best 2D axis state to match an input vector.
	*/
	EAsgard2DAxisState Calculate2DActionState(const FVector& InputVector);

	/**
	* Setter for MoveInput2DActionState.
	*/
	void SetMoveInput2DActionState(EAsgard2DAxisState NewMoveInput2DActionState);


	// ---------------------------------------------------------
	//	Teleport functions

	/**
	* Updates for Smooth teleports.
	*/
	void UpdateSmoothTeleportToLocation(float DeltaSeconds);
	void UpdateSmoothTeleportToRotation(float DeltaSeconds);

	/**
	* Traces an arc and attempts to obtain a valid teleport location.
	* Returns true if there was a valid location found otherwise returns false.
	*/
	bool TraceForTeleportLocation(
		const FVector& TraceOrigin,
		const FVector& TraceVelocity,
		bool bRequiresNavmeshPath,
		TEnumAsByte<ECollisionChannel> TraceChannel,
		FVector& OutTeleportLocation,
		FVector* OptionalOutImpactPoint = nullptr,
		TArray<FVector>* OptionalOutTracePathPoints = nullptr);

	/**
	* Attempts to teleport to the target location, using the given teleport mode.
	* Returns whether the initial request succeeeded.
	*/
	bool TeleportToLocation(const FVector& GoalLocation, EAsgardTeleportMode TeleportMode);

	/**
	* Teleports to the target rotation, using the given teleport mode.
	*/
	bool TeleportToRotation(const FRotator& GoalRotation, EAsgardTeleportMode TeleportMode);

	/**
	* Attempts to teleport to the target location, then rotation, using the given teleport mode.
	* Returns whether the initial teleport to location request succeeeded.
	*/
	bool TeleportToLocationAndRotation(const FVector& GoalLocation, const FRotator& GoalRotation, EAsgardTeleportMode TeleportMode);

	/**
	* Attempts to teleport TeleportTurnAngleIntervals to the right.
	*/
	void TeleportTurnRight();

	/**
	* Attempts to teleport TeleportTurnAngleIntervals to the left.
	*/
	void TeleportTurnLeft();

	/**
	* Attempts to Teleport Walk in the direction indicated by 
	*/
	void TeleportWalk();

	/**
	* Attempts to Teleport in the indicated direction.
	*/
	bool TeleportInDirection(
		const FVector& Direction, 
		float InitialMagnitude,
		bool bRequiresNavMeshPath,
		TEnumAsByte<ECollisionChannel> TraceChannel,
		EAsgardTeleportMode TeleportMode,
		int32 MaxRetries = 0,
		float FallbackMagnitudeInterval = 0.0f);

	/**
	* Setup for Precision Teleport trace.
	*/
	void StartPrecisionTeleport();

	/**
	* Resets a Precision Teleport trace and performs a Teleport if appropriate.
	*/
	void StopPrecisionTeleport();

	/**
	* Updates a Precision Teleport trace for a teleport location.
	*/
	void UpdatePrecisionTeleport(float DeltaSeconds);

	/** 
	* Retrieves the orientated forward and right vector, flattened onto the X and Y axis according to the orientation mode. 
	* @param OrientationComponent If this is null, function will returns the VR forward and Right vectors.
	* @param MaxAbsPitch If OrientationComponent is not null, and its forward would be greater than the max pitch vector, will return false.
	*/
	bool CalculateOrientedFlattenedDirectionalVectors(FVector& OutForward, FVector& OutRight, USceneComponent* OrientationComponent, float MaxAbsPitch = 90.0f) const;


	// ---------------------------------------------------------
	//	General movement state

	/**
	* The the navmesh data when searching for a place on the navmesh this character can stand on.
	*/
	UPROPERTY()
	ANavigationData* ProjectionNavData;

	/**
	* The the navmesh data when searching for pathable locations.
	* Used to allow us to be more generous with pathfinding checks than with projection tests,
	* such as when teleporting to a location.
	*/
	UPROPERTY()
	ANavigationData* PathfindingNavData;

	/**
	* The the navmesh data when searching for pathable locations.
	* Used to allow us to be more generous with pathfinding checks than with projection tests,
	* such as when teleporting to a location.
	*/
	FNavAgentProperties PathfindingNavAgentProperties;

	/**
	* The orientation mode for the character when walking.
	* Used with both smooth and teleport walk.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement", meta = (AllowPrivateAccess = "true"))
	EAsgardOrientationMode WalkOrientationMode;

	/**
	* Reference to component used to orient walking input.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement", meta = (AllowPrivateAccess = "true"))
	USceneComponent* WalkOrientationComponent;


	// ---------------------------------------------------------
	//	Teleporting state

	/**
	* Whether the character is currently teleporting to a location.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	uint32 bIsTeleportingToLocation : 1;

	/**
	* Whether the character is currently teleporting to a pivot.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	uint32 bIsTeleportingToRotation : 1;

	/**
	* The start location of the character when teleporting to a location.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	FVector TeleportStartLocation;

	/**
	* The goal location of the character when teleporting to a location.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	FVector TeleportGoalLocation;

	/**
	* The Rotation location of the character when teleporting to a location.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	FRotator TeleportStartRotation;

	/**
	* The goal Rotation of the character when teleporting to a Rotation.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	FRotator TeleportGoalRotation;

	/**
	* The mode of the current or last teleport to location performed.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	EAsgardTeleportMode CurrentTeleportToLocationMode;

	/**
	* The mode of the current or last teleport to Rotation performed. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	EAsgardTeleportMode CurrentTeleportToRotationMode;

	/**
	* Used to scale the speed of the teleport to location being performed.
	* Only used with Smooth teleport mode.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	float TeleportToLocationSmoothMultiplier;

	/**
	* The progress of the teleport to location being performed.
	* Only used with Smooth teleport mode.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	float TeleportToLocationSmoothProgress;

	/**
	* Used to scale the speed of the teleport to rotation being performed.
	* Only used with Smooth teleport mode.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	float TeleportToRotationSmoothMultiplier;

	/**
	* The progress of the teleport to rotation being performed.
	* Only used with Smooth teleport mode.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	float TeleportToRotationSmoothProgress;

	/** Whether the most recent teleport was successful.*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Teleport", meta = (AllowPrivateAccess = "true"))
	uint32 bTeleportToLocationSucceeded : 1;


	// ---------------------------------------------------------
	//	Precision Teleport state

	/** 
	* Whether the character is performing a Precision teleport. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	uint32 bIsPrecisionTeleportActive : 1;

	/** 
	* Whether the most recent traced Precision Teleport location was valid. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	uint32 bIsPrecisionTeleportLocationValid : 1;

	/** 
	* The the most recent provoking input source of Precision Teleport. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	EAsgardInputSource PrecisionTeleportInputSource;

	/** 
	* The default orientation mode for the character when performing a Precision Teleport trace. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	EAsgardBinaryHand PrecisionTeleportDefaultOrientationMode;

	/** 
	* Reference to component used to orient a Precision Teleport trace. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	USceneComponent* PrecisionTeleportOrientationComponent;

	/** 
	* The most recent location returned by a Precision Teleport trace. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	FVector PrecisionTeleportLocation;

	/** 
	* The most recent impact point returned by a Precision Teleport trace. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	FVector PrecisionTeleportImpactPoint;

	/** 
	* The most recent trace path returned by a Precision Teleport trace. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	TArray<FVector> PrecisionTeleportTracePath;
	
	/** 
	* Direction of the Precision Teleport. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|PrecisionTeleport", meta = (AllowPrivateAccess = "true"))
	FVector PrecisionTeleportTraceDirection;


	// ---------------------------------------------------------
	//	Teleport turn state

	/** 
	* Delegate handle for completing a Teleport Turn. 
	*/
	FDelegateHandle TeleportTurnFinishedDelegateHandle;

	/** 
	* Used for TurnActionHeld events. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|TeleportTurn", meta = (AllowPrivateAccess = "true"))
	FTimerHandle TeleportTurnHeldTimer;


	// ---------------------------------------------------------
	//	Teleport Walk state

	/** 
	* Delegate handle for completing a Teleport Turn. 
	*/
	FDelegateHandle TeleportWalkFinishedDelegateHandle;

	/** 
	* Used for TeleportWalkHeld events. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|TeleportWalk", meta = (AllowPrivateAccess = "true"))
	FTimerHandle TeleportWalkHeldTimer;


	// ---------------------------------------------------------
	//	Smooth Walk state

	/** Whether the character is currently walking. */
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|SmoothWalk", meta = (AllowPrivateAccess = "true"))
	uint32 bIsSmoothWalking : 1;


	// --------------------------------------s-------------------
	//	Flight state

	/** 
	* Whether the left hand Flight thruster is currently active. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Flight", meta = (AllowPrivateAccess = "true"))
	uint32 bIsFlightThrusterLeftActive : 1;

	/** 
	* Whether the right hand Flight thruster is currently active. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Flight", meta = (AllowPrivateAccess = "true"))
	uint32 bIsFlightThrusterRightActive : 1;


	// --------------------------------------s-------------------
	//	Analog flight state

	/** 
	* Reference to component used to orient flying input when moving using the analog stick. 
	*/
	UPROPERTY(BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Flight|Analog", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SmoothFlightOrientationComponent;

	/** 
	* The orientation mode for the character when flying and using the analog stick. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AsgardVRCharacter|Movement|Flight|Analog", meta = (AllowPrivateAccess = "true"))
	EAsgardFlightOrientationMode SmoothFlightOrientationMode;
};