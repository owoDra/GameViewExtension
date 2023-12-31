// Copyright (C) 2023 owoDra

#pragma once

#include "ViewMode.h"

#include "PenetrationAvoidanceFeeler.h"

#include "ViewMode_ThirdPerson.generated.h"

class UCurveVector;
struct FRuntimeFloatCurve;


/**
 * ViewMode base class for TPP viewpoint
 */
UCLASS(Abstract, Blueprintable)
class UViewMode_ThirdPerson : public UViewMode
{
	GENERATED_BODY()
public:
	UViewMode_ThirdPerson(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	FRuntimeFloatCurve TargetOffsetX;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	FRuntimeFloatCurve TargetOffsetY;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	FRuntimeFloatCurve TargetOffsetZ;

	//
	// Alters the speed that a crouch offset is blended in or out
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	float CrouchOffsetBlendMultiplier{ 5.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float PenetrationBlendInTime{ 0.1f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float PenetrationBlendOutTime{ 0.15f };

	//
	// If true, does collision checks to keep the camera out of the world.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bPreventPenetration{ true };

	//
	// If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping.
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bDoPredictiveAvoidance{ true };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionPushOutDistance{ 2.0f };

	//
	// When the camera's distance is pushed into this percentage of its full distance due to penetration
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float ReportPenetrationPercent{ 0.0f };

	//
	// These are the feeler rays that are used to find where to place the camera.
	// Index: 0  : This is the normal feeler we use to prevent collisions.
	// Index: 1+ : These feelers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the player
	//             were to rotate towards that direction and primitively collide the camera so that it pulls in before
	//             impacting the occluder.
	//
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FPenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;

protected:
	virtual void UpdateView(float DeltaTime) override;

	void UpdateForTarget(float DeltaTime);
	void UpdatePreventPenetration(float DeltaTime);
	void PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly);


protected:
	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct;

	FVector InitialCrouchOffset{ FVector::ZeroVector };
	FVector TargetCrouchOffset{ FVector::ZeroVector };
	float CrouchOffsetBlendPct{ 1.0f };
	FVector CurrentCrouchOffset{ FVector::ZeroVector };

protected:
	void SetTargetCrouchOffset(FVector NewTargetOffset);
	void UpdateCrouchOffset(float DeltaTime);

};
