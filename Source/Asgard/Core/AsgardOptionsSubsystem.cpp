// Copyright © 2020 Justin Camden All Rights Reserved


#include "AsgardOptionsSubsystem.h"
#include "Asgard/Core/AsgardOptions.h"
#include "Engine/Classes/Kismet/GameplayStatics.h"

void UAsgardOptionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadCreateOptions();
}

void UAsgardOptionsSubsystem::SetHandedness(const EAsgardBinaryHand NewHandedness)
{
	if (NewHandedness != LoadedOptions->Handedness)
	{
		LoadedOptions->Handedness = NewHandedness;
		OnHandednessChanged.Broadcast(NewHandedness);
	}

	return;
}

void UAsgardOptionsSubsystem::SetDefaultGroundMovementMode(const EAsgardGroundMovementMode NewDefaultGroundMovementMode)
{
	if (NewDefaultGroundMovementMode != LoadedOptions->DefaultGroundMovementMode)
	{
		LoadedOptions->DefaultGroundMovementMode = NewDefaultGroundMovementMode;
		OnDefaultGroundMovementModeChanged.Broadcast(NewDefaultGroundMovementMode);
	}

	return;
}

void UAsgardOptionsSubsystem::SetWalkOrientationMode(const EAsgardOrientationMode NewWalkOrientationMode)
{
	if (NewWalkOrientationMode != LoadedOptions->WalkOrientationMode)
	{
		LoadedOptions->WalkOrientationMode = NewWalkOrientationMode;
		OnWalkOrientationModeChanged.Broadcast(NewWalkOrientationMode);
	}

	return;
}

void UAsgardOptionsSubsystem::SetTeleportToLocationDefaultMode(const EAsgardTeleportMode NewTeleportToLocationDefaultMode)
{
	if (NewTeleportToLocationDefaultMode != LoadedOptions->TeleportToLocationDefaultMode)
	{
		LoadedOptions->TeleportToLocationDefaultMode = NewTeleportToLocationDefaultMode;
		OnTeleportToLocationDefaultModeChanged.Broadcast(NewTeleportToLocationDefaultMode);
	}

	return;
}

void UAsgardOptionsSubsystem::SetTeleportToRotationDefaultMode(const EAsgardTeleportMode NewTeleportToRotationDefaultMode)
{
	if (NewTeleportToRotationDefaultMode != LoadedOptions->TeleportToLocationDefaultMode)
	{
		LoadedOptions->TeleportToRotationDefaultMode = NewTeleportToRotationDefaultMode;
		OnTeleportToRotationDefaultModeChanged.Broadcast(NewTeleportToRotationDefaultMode);
	}

	return;
}

void UAsgardOptionsSubsystem::SetTeleportTurnAngleInterval(const float NewTeleportTurnAngleInterval)
{
	float ProposedTurnAngle = FMath::Clamp(NewTeleportTurnAngleInterval, 0.0f, 180.0f);
	if (ProposedTurnAngle != LoadedOptions->TeleportTurnAngleInterval)
	{
		LoadedOptions->TeleportTurnAngleInterval = ProposedTurnAngle;
		OnTeleportTurnAngleIntervalChanged.Broadcast(ProposedTurnAngle);
	}

	return;
}

void UAsgardOptionsSubsystem::LoadCreateOptions()
{
	const FString OptionsSlot = "AsgardOptions";
	if (UGameplayStatics::DoesSaveGameExist(OptionsSlot, 0))
	{
		LoadedOptions = Cast<UAsgardOptions>(UGameplayStatics::LoadGameFromSlot(OptionsSlot, 0));
		UE_LOG(LogTemp, Error, TEXT("TeleportToLocation failed! TeleportTo returned false."));
	}
	else
	{
		LoadedOptions = Cast<UAsgardOptions>(UGameplayStatics::CreateSaveGameObject(UAsgardOptions::StaticClass()));
		ResetOptions();
		SaveOptions();
	}

	return;
}

void UAsgardOptionsSubsystem::GetDefaultOptions(
	EAsgardBinaryHand& Handedness, 
	EAsgardGroundMovementMode& WalkMode, 
	EAsgardOrientationMode& WalkOrientationMode, 
	EAsgardTeleportMode& TeleportToLocationDefaultMode, 
	EAsgardTeleportMode& TeleportToRotationDefaultMode, 
	float& TeleportTurnAngleInterval)
{
	Handedness = EAsgardBinaryHand::RightHand;
	WalkMode = EAsgardGroundMovementMode::PrecisionTeleportToLocation;
	WalkOrientationMode = EAsgardOrientationMode::Character;
	TeleportToLocationDefaultMode = EAsgardTeleportMode::Fade;
	TeleportToRotationDefaultMode = EAsgardTeleportMode::Fade;
	TeleportTurnAngleInterval = 45.0f;

	return;
}

void UAsgardOptionsSubsystem::ResetOptions()
{
	SetHandedness(EAsgardBinaryHand::RightHand);
	SetDefaultGroundMovementMode(EAsgardGroundMovementMode::PrecisionTeleportToLocation);
	SetWalkOrientationMode(EAsgardOrientationMode::LeftController);
	SetTeleportToLocationDefaultMode(EAsgardTeleportMode::Fade);
	SetTeleportToRotationDefaultMode(EAsgardTeleportMode::Fade);
	SetTeleportTurnAngleInterval(45.0f);
}

void UAsgardOptionsSubsystem::SaveOptions()
{
	UGameplayStatics::SaveGameToSlot(LoadedOptions, "AsgardOptions", 0);
}

const EAsgardBinaryHand UAsgardOptionsSubsystem::GetHandedness() const
{
	return LoadedOptions->Handedness;
}

const EAsgardGroundMovementMode UAsgardOptionsSubsystem::GetDefaultGroundMovementMode() const
{
	return LoadedOptions->DefaultGroundMovementMode;
}

const EAsgardOrientationMode UAsgardOptionsSubsystem::GetWalkOrientationMode() const
{
	return LoadedOptions->WalkOrientationMode;
}

const EAsgardTeleportMode UAsgardOptionsSubsystem::GetTeleportToLocationDefaultMode() const
{
	return LoadedOptions->TeleportToLocationDefaultMode;
}

const EAsgardTeleportMode UAsgardOptionsSubsystem::GetTeleportToRotationDefaultMode() const
{
	return LoadedOptions->TeleportToRotationDefaultMode;
}

const float UAsgardOptionsSubsystem::GetTeleportTurnAngleInterval() const
{
	return LoadedOptions->TeleportTurnAngleInterval;
}