// Copyright © 2020 Justin Camden All Rights Reserved


#include "AsgardCameraProximitySensor.h"

UAsgardCameraProximitySensor::UAsgardCameraProximitySensor(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostPhysics;
	MaxTrackedPoints = 4;
	Min3DAngleFromCamera = 45.0f;
	Min2DAngleBetweenTrackedPoints = 45.0f;
	MinDistanceFromCamera = 5.0f;
}

void UAsgardCameraProximitySensor::BeginPlay()
{
	Super::BeginPlay();
	RollAnglesToNearestTrackedPoints.Reserve(MaxTrackedPoints);
}

void UAsgardCameraProximitySensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RollAnglesToNearestTrackedPoints.Reset();
	// If components are detected
	if (GetDetectedComponents().Num() > 0)
	{
		// Cache variables
		TMap<FVector, float> DistancesByDirection;
		const FTransform& SelfTransform = GetComponentTransform();
		FVector SelfLocation = SelfTransform.GetLocation();
		FVector SelfForward = SelfTransform.GetUnitAxis(EAxis::X);
		float MinForwardAngle = FMath::DegreesToRadians(Min3DAngleFromCamera);

		// For each detected component
		for (UPrimitiveComponent* DetectedComponent : GetDetectedComponents())
		{
			if (DetectedComponent)
			{
				// Get the closest point and distance
				FVector OutPointOnBody;
				float DistanceFromSelf = DetectedComponent->GetClosestPointOnCollision(SelfLocation, OutPointOnBody);

				// If the point is far enough away
				if (DistanceFromSelf > MinDistanceFromCamera)
				{
					// If the point is angled enough away from the center of the camera
					OutPointOnBody = (OutPointOnBody - SelfLocation).GetSafeNormal();
					float AngleFromForward = FMath::Acos(FMath::Abs(FVector::DotProduct(OutPointOnBody, SelfForward)));
					if (AngleFromForward > MinForwardAngle)
					{
						// Add to potential points and distances
						auto ExistingEntry = DistancesByDirection.Find(OutPointOnBody);
						if (ExistingEntry != nullptr)
						{
							if (*ExistingEntry < DistanceFromSelf)
							{
								*ExistingEntry = DistanceFromSelf;
							}
						}
						else
						{
							DistancesByDirection.Add(OutPointOnBody, DistanceFromSelf);
						}
					}
				}
			}
		}

		// If there is at least one potential point
		if (DistancesByDirection.Num() > 0)
		{
			// Sort the points by distance
			DistancesByDirection.ValueSort([](const float A, const float B) {
				return A < B;
				});

			// Cache variables
			TArray<FVector> TrackedPoints;
			TrackedPoints.Reserve(MaxTrackedPoints);
			float MinAngleFromPoint = FMath::DegreesToRadians(Min2DAngleBetweenTrackedPoints);

			// For each potential point
			for (auto& DistanceByPoint : DistancesByDirection)
			{
				// Project and normalize the point
				FVector ProjectedPointNormalized = DistanceByPoint.Key;
				ProjectedPointNormalized = SelfTransform.InverseTransformVectorNoScale(ProjectedPointNormalized);
				ProjectedPointNormalized.X = 0.0f;
				ProjectedPointNormalized = ProjectedPointNormalized.GetSafeNormal();
				
				// Check the angle to the point against the angle from
				bool bTooCloseToTrackedPoint = false;
				for (FVector& CurrPoint : TrackedPoints)
				{
					// If the angle is too close to a currently tracked point, abort the loop
					if (FMath::Acos(FVector::DotProduct(CurrPoint, ProjectedPointNormalized)) < MinAngleFromPoint)
					{
						bTooCloseToTrackedPoint = true;
						break;
					}
				}

				// If the angle was far enough away from any currently tracked points, add it to the list of tracked points
				if (!bTooCloseToTrackedPoint)
				{
					TrackedPoints.Emplace(ProjectedPointNormalized);

					// If we have reached the limited of tracked points, break the loop
					if (MaxTrackedPoints > 0 && TrackedPoints.Num() >= MaxTrackedPoints)
					{
						break;
					}
				}
			}

			// Calculate the roll angle to each tracked point
			for (FVector& TrackedPoint : TrackedPoints)
			{
				RollAnglesToNearestTrackedPoints.Add(FMath::RadiansToDegrees(FMath::Atan2(TrackedPoint.Y, TrackedPoint.X)));
			}
		}
	}

	return;
}
