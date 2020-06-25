// Copyright © 2020 Justin Camden All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Asgard/Sensor/AsgardSphereSensor.h"
#include "AsgardCameraProximitySensor.generated.h"

/**
 *	Class used to sense primitives around the periphery of the player's camera without triggering overlap or hit events.
 */
UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent) )
class ASGARD_API UAsgardCameraProximitySensor : public UAsgardSphereSensor
{
	GENERATED_BODY()
public:
	// Sets default values for this component's properties
	UAsgardCameraProximitySensor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	* The maximum number of primitives to track. 
	* Ignored if <= 0.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardCameraProximitySensor)
	int32 MaxTrackedPoints;

	/**
	* The minimum angle from the front or back of the camera for an object to be considered in the camera's periphery.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardCameraProximitySensor)
	float Min3DAngleFromCamera;

	/**
	* The minimum distance from the camera for an object to be considered in the camera's periphery.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardCameraProximitySensor)
	float MinDistanceFromCamera;

	/**
	* The minimum 2D angle between the nearest points on tracked primitives, measured across the camera plane.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardCameraProximitySensor)
	float Min2DAngleBetweenTrackedPoints;

private:
	/**
	* Current list of the nearest points on tracked primitives.
	*/
	UPROPERTY(BlueprintReadOnly, Category = AsgardCameraProximitySensor, meta = (AllowPrivateAccess = "true"))
	TArray<float> RollAnglesToNearestTrackedPoints;
};
