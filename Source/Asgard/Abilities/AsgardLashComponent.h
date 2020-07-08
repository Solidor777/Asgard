// Copyright © 2020 Justin Camden All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AsgardLashComponent.generated.h"

// Stats group
DECLARE_STATS_GROUP(TEXT("AsgardLash"), STATGROUP_ASGARD_Lash, STATCAT_Advanced);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ASGARD_API UAsgardLashComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAsgardLashComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	* Reduces velocity by this percentage per second.
	* Does not affect gravity.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardLashComponent)
	FVector Damping;

	/**
	* The amount of gravity to apply to each lash segment velocity per second.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardLashComponent)
	FVector Gravity;

	/**
	* The number of physics simulations per second.
	* Higher numbers will result in more accurate simulation but will be more costly.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardLashComponent, meta = (ClampMin = "30.0"))
	int32 PhysicsStepsPerSecond;

	/**
	* How much of a correction is apply to the 'child' point further down down the lash when solving constraints, on a scale of 0-1.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardLashComponent, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChildPointCorrectionWeight;

	/**
	* Whether the lash is currently extended.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardLashComponent)
	uint32 bLashExtended : 1;

#if WITH_EDITORONLY_DATA
	/**
	* Whether to debug draw the lash.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AsgardLashComponent)
	uint32 bDebugDrawLash : 1;
#endif

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	/**
	* Remainder from the last time physics were simulated.
	*/
	float PhysicsStepRemainder;

	/**
	* The points in the lash at the end of the last frame.
	*/
	TArray<FVector> LashPointLastLocations;

	/**
	* The points after calculations in the current frame.
	*/
	TArray<FVector> LashPointCurrentLocations;

	/**
	* The current number of lash segments.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AsgardLashComponent, meta = (AllowPrivateAccess = "true"))
	int32 NumLashSegments;

	/**
	* The maximum number of lash points.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AsgardLashComponent, meta = (AllowPrivateAccess = "true"))
	int32 MaxLashSegments;

	/**
	* The maximum length of each lash segment.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AsgardLashComponent, meta = (AllowPrivateAccess = "true"))
	float LashSegmentMaxLength;

	/**
	* How much the lash should shorten every frame while shrinking.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AsgardLashComponent, meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float LashShrinkSpeed;

	/**
	* When shrinking the lash, the length of the first segment is stored here and used to cap the length of the last segment during the constraints step.
	* This prevents velocity from re-lengthening the lash while it is shrinking.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AsgardLashComponent, meta = (AllowPrivateAccess = "true"))
	float LashShrinFirstSegmentLastLength;

	/**
	* The minimum length of the last lash segment before the lash can 'grow' by adding another segment at the end.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AsgardLashComponent, meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float LashGrowthMinSegmentLength;

	/**
	* The maximum length of the first lash segment before the lash can 'shrink' by removing it.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AsgardLashComponent, meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float LashShrinkMaxSegmentLength;

	/**
	* Adds a point at the end of the lash.
	*/
	void AddLashSegmentAtEnd();

	/**
	* Removes a point at the end of the lash.
	*/
	void RemoveLashSegmentFromFront();

	/**
	* Updates the length of the lash depending on whether it is active.
	* Returns whether the lash has fully shrunk.
	*/
	bool ShrinkLash(float DeltaTime);

	/**
	* Calculates the new current positions for each point, based on velocity from the previous frame.
	*/
	void ApplyVelocityToLashPoints(float DeltaTime);

	/**
	* Applies constraints to individual points in the lash.
	*/
	void ApplyConstraintsToLashPoints();
		
};
