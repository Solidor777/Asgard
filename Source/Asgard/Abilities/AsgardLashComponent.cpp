// Copyright © 2020 Justin Camden All Rights Reserved


#include "AsgardLashComponent.h"

// Stat cycles
DECLARE_CYCLE_STAT(TEXT("AsgardLash SimulateLash"), STAT_ASGARD_LashSimulateLash, STATGROUP_ASGARD_Lash);
DECLARE_CYCLE_STAT(TEXT("AsgardLash Velocity"), STAT_ASGARD_LashVelocity, STATGROUP_ASGARD_Lash);
DECLARE_CYCLE_STAT(TEXT("AsgardLash Constraints"), STAT_ASGARD_LashConstraints, STATGROUP_ASGARD_Lash);

// Console variable setup so we can enable and disable debugging from the console
// Draw detection debug
static TAutoConsoleVariable<int32> CVarAsgardLashDrawDebug(
	TEXT("Asgard.LashDrawDebug"),
	0,
	TEXT("Whether to enable Lash debug drawing.\n")
	TEXT("0: Disabled, 1: Enabled"),
	ECVF_Scalability | ECVF_RenderThreadSafe);
static const auto LashDrawDebug = IConsoleManager::Get().FindConsoleVariable(TEXT("Asgard.LashDrawDebug"));

// Macros for debug builds
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "DrawDebugHelpers.h"
#define DRAW_LASH()	if (LashDrawDebug->GetInt() || bDebugDrawLash) { for (FVector& CurrPoint : LashPointCurrentLocations) { DrawDebugSphere(GetWorld(), CurrPoint, 2.5f, 16, FColor::Magenta, false, -1.0f, 0, 0.5f); } }
#else
#define DRAW_LASH()	/* nothing */
#endif


// Sets default values for this component's properties
UAsgardLashComponent::UAsgardLashComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	MaxLashSegments = 20;
	LashSegmentMaxLength = 10.0f;
	LashGrowthMinSegmentLength = 7.5f;
	LashShrinkMaxSegmentLength = 0.1f;
	LashShrinkSpeed = 30.0f;
	PhysicsStepsPerSecond = 300.0f;
	ChildPointCorrectionWeight = 0.4875f;
	Damping = FVector(5.0f, 5.0f, 5.0f);
	Gravity = FVector(0.0f, 0.0f, -20.0f);
}

// Called when the game starts
void UAsgardLashComponent::BeginPlay()
{
	Super::BeginPlay();

	// Reserve space for the lash points
	LashPointLastLocations.Reserve(MaxLashSegments);
	LashPointCurrentLocations.Reserve(MaxLashSegments);

	// Set the first point to be equal to the location of the component
	FVector ComponentLocation = GetComponentLocation();
	LashPointLastLocations.Add(ComponentLocation);
	LashPointCurrentLocations.Add(ComponentLocation);
}

void UAsgardLashComponent::AddLashSegmentAtEnd()
{
	FVector NewPoint = LashPointLastLocations.Last();
	LashPointLastLocations.Emplace(NewPoint);
	NewPoint = LashPointCurrentLocations.Last();
	LashPointCurrentLocations.Emplace(NewPoint);
	NumLashSegments++;

	return;
}

void UAsgardLashComponent::RemoveLashSegmentFromFront()
{
	checkf(NumLashSegments > 0, TEXT("ERROR: NumLashPoints was <= 0. (AsgardLashComponent, RemoveLashPointFromEnd, %s"), * GetNameSafe(this));
	LashPointLastLocations.RemoveAt(1, 1, false);
	LashPointCurrentLocations.RemoveAt(1, 1, false);
	NumLashSegments--;

	return;
}

bool UAsgardLashComponent::ShrinkLash(float DeltaTime)
{
	// Shrink the first segment
	const FVector& FirstPoint = LashPointCurrentLocations[0];
	FVector& SecondPoint = LashPointCurrentLocations[1];
	SecondPoint = FMath::VInterpConstantTo(SecondPoint, FirstPoint, DeltaTime, LashShrinkSpeed);

	// If the first segment is short, enough remove it
	float DeltaMagSquared = (SecondPoint - FirstPoint).SizeSquared();
	if (DeltaMagSquared <= LashShrinkMaxSegmentLength * LashShrinkMaxSegmentLength)
	{
		RemoveLashSegmentFromFront();

		// Update the length of the last segment if more segments remain
		if (NumLashSegments > 0)
		{
			LashShrinFirstSegmentLastLength = (LashPointCurrentLocations[1] - FirstPoint).Size();
			return false;
		}
		else
		{
			return true;
		}
	}

	// Otherwise, update the length of the last segment
	else
	{
		LashShrinFirstSegmentLastLength = FMath::Sqrt(DeltaMagSquared);
		return false;
	}
}

void UAsgardLashComponent::ApplyVelocityToLashPoints(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_ASGARD_LashVelocity);

	for (uint8 Idx = 1; Idx <= NumLashSegments; Idx++)
	{
		// Calculate the velocity
		FVector& CurrentLocation = LashPointCurrentLocations[Idx];
		FVector& LastLocation = LashPointLastLocations[Idx];
		FVector Velocity = CurrentLocation - LastLocation;
		Velocity = FMath::Lerp(Velocity, Velocity - (Damping * Velocity), DeltaTime);

		// Update the last and current locations
		LastLocation = CurrentLocation;
		CurrentLocation = CurrentLocation + Velocity + (Gravity * DeltaTime);
	}

	return;
}

void UAsgardLashComponent::ApplyConstraintsToLashPointsFromFront()
{
	SCOPE_CYCLE_COUNTER(STAT_ASGARD_LashConstraints);

	// Solve constraints from the end to give the lash a weighty feely
	for (int32 Idx = NumLashSegments; Idx > 1; Idx--)
	{
		// Cache variables
		FVector& CurrentPoint = LashPointCurrentLocations[Idx];
		FVector& NextPoint = LashPointCurrentLocations[Idx - 1];
		FVector ToNextPoint = NextPoint - CurrentPoint;
		float DistToNextPoint = ToNextPoint.SizeSquared();

		// If the current point is too far from the next point
		if (DistToNextPoint > LashSegmentMaxLength* LashSegmentMaxLength)
		{
			// Normalize direction to next point
			DistToNextPoint = FMath::Sqrt(DistToNextPoint);
			ToNextPoint = ToNextPoint / DistToNextPoint;

			// Scale the distance according to the error
			DistToNextPoint -= LashSegmentMaxLength;
			ToNextPoint = ToNextPoint * DistToNextPoint;

			// Apply the correction to the current and next points
			CurrentPoint = CurrentPoint + (ToNextPoint * ChildPointCorrectionWeight);
			NextPoint = NextPoint + (ToNextPoint * -(1.0f - ChildPointCorrectionWeight));
		}
	}

	// The first segment is special because it is constrained to the root of the chain, aka the component
	// It can also shrink while the chain is inactive
	// Begin by cashing the variables
	const FVector& FirstPoint = LashPointCurrentLocations[0];
	FVector& SecondPoint = LashPointCurrentLocations[1];
	FVector ToSecondPoint = SecondPoint - FirstPoint;
	float FirstSegmentLength = ToSecondPoint.SizeSquared();

	// If physics and constraint simulation has lengthened the first segment
	if (FirstSegmentLength > LashSegmentMaxLength * LashSegmentMaxLength)
	{
		ToSecondPoint = ToSecondPoint / (FMath::Sqrt(FirstSegmentLength));
		SecondPoint = FirstPoint + (ToSecondPoint * LashSegmentMaxLength);
	}

	return;
}

void UAsgardLashComponent::ApplyConstraintsToLashPointsFromBack()
{
	SCOPE_CYCLE_COUNTER(STAT_ASGARD_LashConstraints);

	// Solve constraints from the end to give the lash a weighty feely
	for (int32 Idx = NumLashSegments; Idx > 1; Idx--)
	{
		// Cache variables
		FVector& CurrentPoint = LashPointCurrentLocations[Idx];
		FVector& NextPoint = LashPointCurrentLocations[Idx - 1];
		FVector ToNextPoint = NextPoint - CurrentPoint;
		float DistToNextPoint = ToNextPoint.SizeSquared();

		// If the current point is too far from the next point
		if (DistToNextPoint > LashSegmentMaxLength* LashSegmentMaxLength)
		{
			// Normalize direction to next point
			DistToNextPoint = FMath::Sqrt(DistToNextPoint);
			ToNextPoint = ToNextPoint / DistToNextPoint;

			// Scale the distance according to the error
			DistToNextPoint -= LashSegmentMaxLength;
			ToNextPoint = ToNextPoint * DistToNextPoint;

			// Apply the correction to the current and next points
			CurrentPoint = CurrentPoint + (ToNextPoint *  (1.0f - ChildPointCorrectionWeight));
			NextPoint = NextPoint + (ToNextPoint * -ChildPointCorrectionWeight);
		}
	}

	// The first segment is special because it is constrained to the root of the chain, aka the component
	// Don't worry about shrinking while applying constrainst from the back
	// Begin by cashing the variables
	const FVector& FirstPoint = LashPointCurrentLocations[0];
	FVector& SecondPoint = LashPointCurrentLocations[1];
	FVector ToSecondPoint = SecondPoint - FirstPoint;
	float FirstSegmentLength = ToSecondPoint.SizeSquared();

	// If physics and constraint simulation has lengthened the first segment
	if (FirstSegmentLength > LashShrinFirstSegmentLastLength * LashShrinFirstSegmentLastLength)
	{
		ToSecondPoint = ToSecondPoint / (FMath::Sqrt(FirstSegmentLength));
		SecondPoint = FirstPoint + (ToSecondPoint * LashShrinFirstSegmentLastLength);
	}

	return;
}


// Called every frame
void UAsgardLashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Update origin position
	FVector& LashOrigin = LashPointCurrentLocations[0];
	LashPointLastLocations[0] = LashOrigin;
	LashOrigin = GetComponentLocation();

	// If there are lash segments extended
	if (NumLashSegments > 0)
	{
		SCOPE_CYCLE_COUNTER(STAT_ASGARD_LashSimulateLash);

		// Calculate the number of physics steps
		float PhysicsStepTime = 1.0f / PhysicsStepsPerSecond;
		PhysicsStepRemainder += DeltaTime;
		int32 NumPhysicsSteps = (int32)(PhysicsStepRemainder / PhysicsStepTime);
		PhysicsStepRemainder -= (float)NumPhysicsSteps * PhysicsStepTime;

		// If active
		if (bLashExtended)
		{
			// For each physics steps
			for (int32 Idx = 0; Idx < NumPhysicsSteps; Idx++)
			{
				// If more lash segments can be added and the last segment is of the minimum length, then add a segment
				if (NumLashSegments < MaxLashSegments
					&& (LashPointCurrentLocations[NumLashSegments] - LashPointCurrentLocations[NumLashSegments - 1]).SizeSquared() >= LashGrowthMinSegmentLength * LashGrowthMinSegmentLength)
				{
					AddLashSegmentAtEnd();
				}

				// Update point velocities and contrainst
				ApplyVelocityToLashPoints(PhysicsStepTime);
				ApplyConstraintsToLashPointsFromFront();
			}
		}
		// If not active
		else
		{
			// For	` each physic step while the lash has not fully shrunk
			for (int32 Idx = 0; Idx < NumPhysicsSteps; Idx++)
			{
				// Shrink the lash
				if (ShrinkLash(PhysicsStepTime))
				{
					break;
				}
				else
				{
					// If the lash has still not fully shrink, update point velocities and constraints
					ApplyVelocityToLashPoints(PhysicsStepTime);
					ApplyConstraintsToLashPointsFromBack();
				}
			}
		}
	}

	// Otherwise, if the lash is active
	else if (bLashExtended)
	{
		// Add a segment
		AddLashSegmentAtEnd();
	}

	DRAW_LASH();

	return;
}
