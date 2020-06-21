// Copyright © 2020 Justin Camden All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AsgardCameraFunctionLibrary.generated.h"

class UCameraComponent;

/**
 * Contains camera specific utility functions.
 */
UCLASS()
class ASGARD_API UAsgardCameraFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	* Returns the aspect ratio of the camera.
	*/
	UFUNCTION(BlueprintCallable, Category = AsgardCameraFunctionLibrary)
	static float GetAspectRatio(UCameraComponent* Camera);

	/**
	* Returns the width of the camera view in world units at a specific distance from the camera.
	*/
	UFUNCTION(BlueprintCallable, Category = AsgardCameraFunctionLibrary)
	static float GetFrustumWidth(UCameraComponent* Camera, float Distance);

	/**
	* Returns the height of the camera view in world units at a specific distance from the camera.
	*/
	UFUNCTION(BlueprintCallable, Category = AsgardCameraFunctionLibrary)
	static float GetFrustumHeight(UCameraComponent* Camera, float Distance);

	/**
	* Returns the width and height of the camera view in world units at a specific distance from the camera.
	*/
	UFUNCTION(BlueprintCallable, Category = AsgardCameraFunctionLibrary)
	static FVector2D GetFrustumSize(UCameraComponent* Camera, float Distance);
};
