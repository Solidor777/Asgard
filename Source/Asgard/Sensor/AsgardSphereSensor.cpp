// Copyright © 2020 Justin Camden All Rights Reserved


#include "AsgardSphereSensor.h"
#include "Runtime/Engine/Classes/Engine/World.h"

// Stat cycles
DECLARE_CYCLE_STAT(TEXT("AsgardSphereSensor OverlapTest"), STAT_ASGARD_SphereSensorOverlapTest, STATGROUP_ASGARD_SphereSensor);
DECLARE_CYCLE_STAT(TEXT("AsgardSphereSensor ProcessOverlaps"), STAT_ASGARD_SphereSensorProcessOverlaps, STATGROUP_ASGARD_SphereSensor);

// Console variable setup so we can enable and disable debugging from the console
// Draw detection debug
static TAutoConsoleVariable<int32> CVarAsgardSphereSensorDrawDebug(
	TEXT("Asgard.SphereSensorDrawDebug"),
	0,
	TEXT("Whether to enable SphereSensor detection debug.\n")
	TEXT("0: Disabled, 1: Enabled"),
	ECVF_Scalability | ECVF_RenderThreadSafe);
static const auto SphereSensorDrawDebug = IConsoleManager::Get().FindConsoleVariable(TEXT("Asgard.SphereSensorDrawDebug"));

// Macros for debug builds
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "DrawDebugHelpers.h"
#define DRAW_SENSOR()	if (SphereSensorDrawDebug->GetInt() || bDebugDrawSensor) { DrawDebugSphere(GetWorld(), GetComponentLocation(), Radius, 16, (DetectedComponents.Num() > 0 ? FColor::Red : FColor::Green), false, -1.0f, 0, 0.5f); }
#else
#define DRAW_SENSOR()	/* nothing */
#endif

// Sets default values for this component's properties
UAsgardSphereSensor::UAsgardSphereSensor(const FObjectInitializer & ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Radius = 32.0f;
	DetectionChannel = ECC_Visibility;
	bAutoIgnoreOwner = true;
	bUseAsyncOverlapTests = true;
}


// Called when the game starts
void UAsgardSphereSensor::BeginPlay()
{
	Super::BeginPlay();
	AsyncOverlapTestDelegate.BindUObject(this, &UAsgardSphereSensor::OnAsyncOverlapTestCompleted);
}


// Called every frame
void UAsgardSphereSensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UWorld* World = GetWorld();
	if (World)
	{
		SCOPE_CYCLE_COUNTER(STAT_ASGARD_SphereSensorOverlapTest);

		// Overlap test
		TArray<FOverlapResult> OutOverlaps;
		const FTransform& SelfTransform = GetComponentTransform();
		const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(Radius);
		FCollisionQueryParams Params;
		Params.AddIgnoredActors(IgnoredActors);
		if (bAutoIgnoreOwner)
		{
			Params.AddIgnoredActors(TArray<AActor*> {GetOwner()});
		}
		Params.TraceTag = FName("AsgardSphereSensorOverlapTest");

		if (bUseAsyncOverlapTests)
		{
			AsyncOverlapTestHandle = World->AsyncOverlapByChannel(
				SelfTransform.GetLocation(), 
				SelfTransform.GetRotation(), 
				DetectionChannel, 
				CollisionShape, 
				Params, 
				FCollisionResponseParams::DefaultResponseParam, 
				&AsyncOverlapTestDelegate);
		}
		else
		{
			FCollisionResponseParams ResponseParams;
			bool bBlockingHits = World->OverlapMultiByChannel(
				OutOverlaps,
				SelfTransform.GetLocation(),
				SelfTransform.GetRotation(),
				DetectionChannel,
				CollisionShape,
				Params,
				ResponseParams);
			ProcessOverlaps(OutOverlaps, bBlockingHits);
		}
	}

	return;
}

void UAsgardSphereSensor::OnAsyncOverlapTestCompleted(const FTraceHandle& Handle, FOverlapDatum& Data)
{
	checkf(Handle == AsyncOverlapTestHandle, TEXT("Invalid incoming handle != AsyncOverlapTestHandle (AsgardSphereSensor, Actor %f, Component %f)"), *GetNameSafe(GetOwner()), *GetNameSafe(this));
	ProcessOverlaps(Data.OutOverlaps, true);
	AsyncOverlapTestHandle._Data.FrameNumber = 0;

	return;
}

void UAsgardSphereSensor::ProcessOverlaps(TArray<FOverlapResult>& Overlaps, bool bBlockingHits)
{
	SCOPE_CYCLE_COUNTER(STAT_ASGARD_SphereSensorProcessOverlaps);

	// If we detect nothing, clear the list of detected components
	if (DetectedComponents.Num() && (Overlaps.Num() <= 0 || (!bBlockingHits && bDetectBlockingHitsOnly)))
	{
		for (UPrimitiveComponent* LostComponent : DetectedComponents)
		{
			OnComponentLost.Broadcast(LostComponent);
		}
		for (AActor* LostActor : DetectedActors)
		{
			OnActorLost.Broadcast(LostActor);
		}
		DetectedComponents.Reset();
		DetectedActors.Reset();

	}
	else
	{
		TSet<AActor*> LostActors = DetectedActors;
		TSet<UPrimitiveComponent*> LostComponents = DetectedComponents;
		TMap<AActor*, TArray<UPrimitiveComponent*>> DetectedComponentsByDetectedActor;

		// If we only detect blocking hits, keep only detected blocking hits
		if (bDetectBlockingHitsOnly)
		{
			for (FOverlapResult& Overlap : Overlaps)
			{
				if (Overlap.bBlockingHit)
				{
					UPrimitiveComponent* DetectedComponent = Overlap.Component.Get();
					if (DetectedComponent)
					{
						// Cache the detected component and actor
						AActor* DetectedActor = Overlap.Actor.Get();

						// Remove detected component and actor from lost list if appropriate
						LostActors.Remove(DetectedActor);
						LostComponents.Remove(DetectedComponent);

						// Add component to matching array for its actor, or create an array as appropriate
						TArray<UPrimitiveComponent*>* DetectedActorsDetectedComponents = DetectedComponentsByDetectedActor.Find(DetectedActor);
						if (DetectedActorsDetectedComponents != nullptr)
						{
							DetectedActorsDetectedComponents->Add(DetectedComponent);
						}
						else
						{
							DetectedComponentsByDetectedActor.Emplace(DetectedActor, TArray<UPrimitiveComponent*> {DetectedComponent});
						}

					}
				}
			}
		}

		// Otherwise, keep all overlaps
		else
		{
			for (FOverlapResult& Overlap : Overlaps)
			{
				UPrimitiveComponent* DetectedComponent = Overlap.Component.Get();
				if (DetectedComponent)
				{
					AActor* DetectedActor = Overlap.Actor.Get();

					// Remove detected component and actor from lost list if appropriate
					LostActors.Remove(DetectedActor);
					LostComponents.Remove(DetectedComponent);

					// Add component to matching array for its actor, or create an array as appropriate
					TArray<UPrimitiveComponent*>* DetectedActorsDetectedComponents = DetectedComponentsByDetectedActor.Find(DetectedActor);
					if (DetectedActorsDetectedComponents != nullptr)
					{
						DetectedActorsDetectedComponents->Add(DetectedComponent);
					}
					else
					{
						DetectedComponentsByDetectedActor.Emplace(DetectedActor, TArray<UPrimitiveComponent*> {DetectedComponent});
					}
				}
			}
		}

		// Clear all components and actors that are no longer detected
		for (UPrimitiveComponent* LostComponent : LostComponents)
		{
			DetectedComponents.Remove(LostComponent);
			OnComponentLost.Broadcast(LostComponent);
		}
		for (AActor* LostActor : LostActors)
		{
			DetectedActors.Remove(LostActor);
			OnActorLost.Broadcast(LostActor);
		}

		// For each detected actor
		for (auto& DetectedActorsComponents : DetectedComponentsByDetectedActor)
		{
			// Add the actor to the detected list
			bool bAlreadyDetected = false;
			DetectedActors.Add(DetectedActorsComponents.Key, &bAlreadyDetected);

			// If it's a new actor, broadcast the on detected event for it and all its detected components
			if (!bAlreadyDetected)
			{
				OnActorDetected.Broadcast(DetectedActorsComponents.Key, DetectedActorsComponents.Value[0]);
				for (UPrimitiveComponent* DetectedComponent : DetectedActorsComponents.Value)
				{
					DetectedComponents.Add(DetectedComponent);
					OnComponentDetected.Broadcast(DetectedComponent);
				}
			}

			// Otherwise, check each of its detected components for whether it is already detected
			else
			{
				for (UPrimitiveComponent* DetectedComponent : DetectedActorsComponents.Value)
				{
					DetectedComponents.Add(DetectedComponent, &bAlreadyDetected);
					if (!bAlreadyDetected)
					{
						OnComponentDetected.Broadcast(DetectedComponent);
					}
				}
			}
		}
	}

	DRAW_SENSOR();

	return;
}
