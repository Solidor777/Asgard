// Copyright © 2020 Justin Camden All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Asgard/Core/AsgardOptionsTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AsgardOptionsSubsystem.generated.h"

// Forward declarations
class UAsgardOptions;

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBinaryHandSignature, EAsgardBinaryHand, BinaryHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroundMovementModeSignature, EAsgardGroundMovementMode, GroundMovementMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrientationModeSignature, EAsgardOrientationMode, OrientationMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportModeSignature, EAsgardTeleportMode, TeleportMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportTurnAngleIntervalSignature, float, AngleInterval);

/**
 * System for central management of game settings.
 */
UCLASS()
class ASGARD_API UAsgardOptionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---------------------------------------------------------
	//	Delegates for updating other blueprint

	UPROPERTY(BlueprintAssignable, Category = "Asgard|OptionsSubSystem|Controls")
	FOnBinaryHandSignature OnHandednessChanged;

	UPROPERTY(BlueprintAssignable, Category = "Asgard|OptionsSubSystem|Controls")
	FOnGroundMovementModeSignature OnDefaultGroundMovementModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Asgard|OptionsSubSystem|Controls")
	FOnOrientationModeSignature OnWalkOrientationModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Asgard|OptionsSubSystem|Controls")
	FOnTeleportModeSignature OnTeleportToLocationDefaultModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Asgard|OptionsSubSystem|Controls")
	FOnTeleportModeSignature OnTeleportToRotationDefaultModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Asgard|OptionsSubSystem|Controls")
	FOnTeleportTurnAngleIntervalSignature OnTeleportTurnAngleIntervalChanged;


	// ---------------------------------------------------------
	//	Getters

	/** Whether the controls should be right or left handed. */
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	const EAsgardBinaryHand GetHandedness() const;

	/** Whether the move axis should start a precision teleport, perform short, repeated teleports, or smoothly walk using the analog stick. */
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	const EAsgardGroundMovementMode GetDefaultGroundMovementMode() const;

	/**
	* The orientation mode for the character when walking.
	* Used with both smooth and teleport walk.
	*/
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	const EAsgardOrientationMode GetWalkOrientationMode() const;

	/**
	* The method of teleportation used for teleporting to a location.
	* Fade: The camera will fade out, the player will be placed at the goal location, and then the camera will fade back in.
	* Instant: Character is instantly placed at their goal location.
	* SmoothRate: Character is smoothly interpolated to their goal location at a given rate.
	* SmoothTime: Character is smoothly interpolated to their goal location over a given time.
	*/
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	const EAsgardTeleportMode GetTeleportToLocationDefaultMode() const;

	/**
	* The method of teleportation used for teleporting to a specific Rotation.
	* Fade: The camera will fade out, the player will be placed at the goal rotation, and then the camera will fade back in.
	* Instant: Character is instantly placed at their goal Rotation.
	* SmoothRate: Character is smoothly interpolated to their goal Rotation at a given rate.
	* SmoothTime: Character is smoothly interpolated to their goal Rotation over a given time.
	*/
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	const EAsgardTeleportMode GetTeleportToRotationDefaultMode() const;

	/** How many degrees to turn when performing a teleport turn. */
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	const float GetTeleportTurnAngleInterval() const;


	// ---------------------------------------------------------
	//	Setters

	/** Whether the controls should be right or left handed. */
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	void SetHandedness(const EAsgardBinaryHand NewHandedness);

	/** Whether the move axis should start a precision teleport, perform short, repeated teleports, or smoothly walk using the analog stick. */
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	void SetDefaultGroundMovementMode(const EAsgardGroundMovementMode NewDefaultGroundMovementMode);

	/**
	* The orientation mode for the character when walking.
	* Used with both smooth and teleport walk.
	*/
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	void SetWalkOrientationMode(const EAsgardOrientationMode NewWalkOrientationMode);

	/**
	* The default method of teleportation used for teleporting to a location.
	* Fade: The camera will fade out, the player will be placed at the goal location, and then the camera will fade back in.
	* Instant: Character is instantly placed at their goal location.
	* Smooth : Character is smoothly interpolated to their goal rotation at a given rate (or time).
	*/
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	void SetTeleportToLocationDefaultMode(const EAsgardTeleportMode NewTeleportToLocationDefaultMode);

	/**
	* The default method of teleportation used for teleporting to a rotation.
	* Fade: The camera will fade out, the player will be placed at the goal location, and then the camera will fade back in.
	* Instant: Character is instantly placed at their goal location.
	* Smooth : Character is smoothly interpolated to their goal rotation at a given rate (or time).
	*/
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	void SetTeleportToRotationDefaultMode(const EAsgardTeleportMode NewTeleportToRotationDefaultMode);
	
	/** How many degrees to turn when performing a teleport turn. */
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem|Controls")
	void SetTeleportTurnAngleInterval(const float NewTeleportTurnAngleInterval);


	// ---------------------------------------------------------
	//	Loading and saving

	/** 
	* Loads the saved player options.
	* If no options exist, will create and save default options.
	*/
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem")
	void LoadCreateOptions();

	/** Gets the default settings.*/
	void GetDefaultOptions(
		EAsgardBinaryHand& Handedness,
		EAsgardGroundMovementMode& WalkMode,
		EAsgardOrientationMode& WalkOrientationMode,
		EAsgardTeleportMode& TeleportToLocationDefaultMode,
		EAsgardTeleportMode& TeleportToRotationDefaultMode,
		float& TeleportTurnAngleInterval);
	
	/** Resets the player options to default settings. */
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem")
	void ResetOptions();

	/** Saves the player options. */
	UFUNCTION(BlueprintCallable, Category = "Asgard|OptionsSubSystem")
	void SaveOptions();

private:
	/** Loaded player options reference*/
	UPROPERTY()
	UAsgardOptions* LoadedOptions;
};