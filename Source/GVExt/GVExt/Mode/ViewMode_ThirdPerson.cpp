// Copyright (C) 2024 owoDra

#include "ViewMode_ThirdPerson.h"

#include "ViewAssistInterface.h"

#include "Curves/CurveVector.h"
#include "Engine/Canvas.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ViewMode_ThirdPerson)


UViewMode_ThirdPerson::UViewMode_ThirdPerson(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +00.0f, 0.0f), 1.00f, 1.00f, 14.f, 0));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, -16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, -32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+20.0f, +00.0f, 0.0f), 1.00f, 1.00f, 00.f, 4));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(-20.0f, +00.0f, 0.0f), 0.50f, 0.50f, 00.f, 4));

}


void UViewMode_ThirdPerson::UpdateView(float DeltaTime)
{
	UpdateForTarget(DeltaTime);
	UpdateCrouchOffset(DeltaTime);

	auto PivotLocation{ GetPivotLocation() + CurrentCrouchOffset };
	auto PivotRotation{ GetPivotRotation() };

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;

	// Apply third person offset using pitch.

	auto TargetOffset{ FVector(0.0f) };

	TargetOffset.X = TargetOffsetX.GetRichCurveConst()->Eval(PivotRotation.Pitch);
	TargetOffset.Y = TargetOffsetY.GetRichCurveConst()->Eval(PivotRotation.Pitch);
	TargetOffset.Z = TargetOffsetZ.GetRichCurveConst()->Eval(PivotRotation.Pitch);

	View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);

	// Adjust final desired camera location to prevent any penetration

	UpdatePreventPenetration(DeltaTime);
}

void UViewMode_ThirdPerson::UpdateForTarget(float DeltaTime)
{
	if (const auto* TargetCharacter{ GetTargetPawn<ACharacter>() })
	{
		if (TargetCharacter->bIsCrouched)
		{
			const auto* TargetCharacterCDO{ TargetCharacter->GetClass()->GetDefaultObject<ACharacter>() };
			const auto CrouchedHeightAdjustment{ TargetCharacterCDO->CrouchedEyeHeight - TargetCharacterCDO->BaseEyeHeight };

			SetTargetCrouchOffset(FVector(0.f, 0.f, CrouchedHeightAdjustment));

			return;
		}
	}

	SetTargetCrouchOffset(FVector::ZeroVector);
}

void UViewMode_ThirdPerson::UpdatePreventPenetration(float DeltaTime)
{
	if (!bPreventPenetration)
	{
		return;
	}

	auto* TargetPawn{ GetTargetPawn() };
	auto* TargetController{ TargetPawn ? TargetPawn->GetController() : nullptr };

	auto* TargetControllerAssist{ Cast<IViewAssistInterface>(TargetController) };
	auto* TargetPawnAssist{ Cast<IViewAssistInterface>(TargetPawn) };

	auto OptionalPPTarget{ TargetPawnAssist ? TargetPawnAssist->GetCameraPreventPenetrationTarget() : TOptional<AActor*>() };
	auto* PPActor{ OptionalPPTarget.IsSet() ? OptionalPPTarget.GetValue() : TargetPawn };

	auto* PPActorAssist{ OptionalPPTarget.IsSet() ? Cast<IViewAssistInterface>(PPActor) : nullptr };

	if (const auto* PPActorRootComponent{ Cast<UPrimitiveComponent>(PPActor->GetRootComponent()) })
	{
		// Attempt at picking SafeLocation automatically, so we reduce camera translation when aiming.
		// Our camera is our reticle, so we want to preserve our aim and keep that as steady and smooth as possible.
		// Pick closest point on capsule to our aim line.

		auto ClosestPointOnLineToCapsuleCenter{ FVector::ZeroVector };
		auto SafeLocation{ PPActor->GetActorLocation() };

		FMath::PointDistToLine(SafeLocation, View.Rotation.Vector(), View.Location, ClosestPointOnLineToCapsuleCenter);

		// Adjust Safe distance height to be same as aim line, but within capsule.

		const auto PushInDistance{ PenetrationAvoidanceFeelers[0].Extent + CollisionPushOutDistance };
		const auto MaxHalfHeight{ PPActor->GetSimpleCollisionHalfHeight() - PushInDistance };

		SafeLocation.Z = FMath::Clamp(ClosestPointOnLineToCapsuleCenter.Z, SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

		auto DistanceSqr{ 0.0f };
		PPActorRootComponent->GetSquaredDistanceToCollision(ClosestPointOnLineToCapsuleCenter, DistanceSqr, SafeLocation);

		// Push back inside capsule to avoid initial penetration when doing line checks.

		if (PenetrationAvoidanceFeelers.Num() > 0)
		{
			SafeLocation += (SafeLocation - ClosestPointOnLineToCapsuleCenter).GetSafeNormal() * PushInDistance;
		}

		// Then aim line to desired camera position

		const bool bSingleRayPenetrationCheck{ !bDoPredictiveAvoidance };
		PreventCameraPenetration(*PPActor, SafeLocation, View.Location, DeltaTime, AimLineToDesiredPosBlockedPct, bSingleRayPenetrationCheck);

		auto AssistArray{ TArray<IViewAssistInterface*>({ TargetControllerAssist, TargetPawnAssist, PPActorAssist }) };

		if (AimLineToDesiredPosBlockedPct < ReportPenetrationPercent)
		{
			for (const auto& Assist : AssistArray)
			{
				if (Assist)
				{
					// camera is too close, tell the assists

					Assist->OnCameraPenetratingTarget();
				}
			}
		}
	}
}

void UViewMode_ThirdPerson::PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
	auto HardBlockedPct{ DistBlockedPct };
	auto SoftBlockedPct{ DistBlockedPct };

	auto BaseRay{ CameraLoc - SafeLoc };
	auto BaseRayMatrix{ FRotationMatrix(BaseRay.Rotation()) };

	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;
	BaseRayMatrix.GetScaledAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

	auto DistBlockedPctThisFrame{ 1.0f };

	const auto NumRaysToShoot{ bSingleRayOnly ? FMath::Min(1, PenetrationAvoidanceFeelers.Num()) : PenetrationAvoidanceFeelers.Num() };
	auto SphereParams{ FCollisionQueryParams(SCENE_QUERY_STAT(CameraPen), false, nullptr/*PlayerCamera*/) };

	SphereParams.AddIgnoredActor(&ViewTarget);

	auto SphereShape{ FCollisionShape::MakeSphere(0.f) };
	auto* World{ GetWorld() };

	for (auto RayIdx{ 0 }; RayIdx < NumRaysToShoot; ++RayIdx)
	{
		auto& Feeler{ PenetrationAvoidanceFeelers[RayIdx] };
		if (Feeler.FramesUntilNextTrace <= 0)
		{
			// calc ray target

			auto RotatedRay{ BaseRay.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp) };
			RotatedRay = RotatedRay.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);

			auto RayTarget{ SafeLoc + RotatedRay };

			// cast for world and pawn hits separately.  this is so we can safely ignore the 
			// camera's target pawn

			SphereShape.Sphere.Radius = Feeler.Extent;
			auto TraceChannel{ ECC_Camera };		//(Feeler.PawnWeight > 0.f) ? ECC_Pawn : ECC_Camera;

			// do multi-line check to make sure the hits we throw out aren't
			// masking real hits behind (these are important rays).

			// MT-> passing camera as actor so that camerablockingvolumes know when it's the camera doing traces

			FHitResult Hit;
			const auto bHit{ World->SweepSingleByChannel(Hit, SafeLoc, RayTarget, FQuat::Identity, TraceChannel, SphereShape, SphereParams) };

			Feeler.FramesUntilNextTrace = Feeler.TraceInterval;

			const auto* HitActor{ Hit.GetActor() };

			if (bHit && HitActor)
			{
				auto bIgnoreHit{ false };

				// Ignore CameraBlockingVolume hits that occur in front of the ViewTarget.

				if (!bIgnoreHit && HitActor->IsA<ACameraBlockingVolume>())
				{
					const auto ViewTargetForwardXY{ ViewTarget.GetActorForwardVector().GetSafeNormal2D() };
					const auto ViewTargetLocation{ ViewTarget.GetActorLocation() };
					const auto HitOffset{ Hit.Location - ViewTargetLocation };
					const auto HitDirectionXY{ HitOffset.GetSafeNormal2D() };
					const auto DotHitDirection{ FVector::DotProduct(ViewTargetForwardXY, HitDirectionXY) };

					if (DotHitDirection > 0.0f)
					{
						bIgnoreHit = true;

						// Ignore this CameraBlockingVolume on the remaining sweeps.

						SphereParams.AddIgnoredActor(HitActor);
					}
				}
				
				if (!bIgnoreHit)
				{
					const auto Weight{ Cast<APawn>(Hit.GetActor()) ? Feeler.PawnWeight : Feeler.WorldWeight };
					auto NewBlockPct{ Hit.Time };
					NewBlockPct += (1.f - NewBlockPct) * (1.f - Weight);

					// Recompute blocked pct taking into account pushout distance.

					NewBlockPct = ((Hit.Location - SafeLoc).Size() - CollisionPushOutDistance) / (RayTarget - SafeLoc).Size();
					DistBlockedPctThisFrame = FMath::Min(NewBlockPct, DistBlockedPctThisFrame);

					// This feeler got a hit, so do another trace next frame

					Feeler.FramesUntilNextTrace = 0;
				}
			}

			if (RayIdx == 0)
			{
				// don't interpolate toward this one, snap to it
				// assumes ray 0 is the center/main ray 

				HardBlockedPct = DistBlockedPctThisFrame;
			}
			else
			{
				SoftBlockedPct = DistBlockedPctThisFrame;
			}
		}
		else
		{
			--Feeler.FramesUntilNextTrace;
		}
	}

	if (bResetInterpolation)
	{
		DistBlockedPct = DistBlockedPctThisFrame;
	}
	else if (DistBlockedPct < DistBlockedPctThisFrame)
	{
		// interpolate smoothly out

		if (PenetrationBlendOutTime > DeltaTime)
		{
			DistBlockedPct = DistBlockedPct + DeltaTime / PenetrationBlendOutTime * (DistBlockedPctThisFrame - DistBlockedPct);
		}
		else
		{
			DistBlockedPct = DistBlockedPctThisFrame;
		}
	}
	else
	{
		if (DistBlockedPct > HardBlockedPct)
		{
			DistBlockedPct = HardBlockedPct;
		}
		else if (DistBlockedPct > SoftBlockedPct)
		{
			// interpolate smoothly in

			if (PenetrationBlendInTime > DeltaTime)
			{
				DistBlockedPct = DistBlockedPct - DeltaTime / PenetrationBlendInTime * (DistBlockedPct - SoftBlockedPct);
			}
			else
			{
				DistBlockedPct = SoftBlockedPct;
			}
		}
	}

	DistBlockedPct = FMath::Clamp<float>(DistBlockedPct, 0.f, 1.f);
	if (DistBlockedPct < (1.f - ZERO_ANIMWEIGHT_THRESH))
	{
		CameraLoc = SafeLoc + (CameraLoc - SafeLoc) * DistBlockedPct;
	}
}


void UViewMode_ThirdPerson::SetTargetCrouchOffset(FVector NewTargetOffset)
{
	CrouchOffsetBlendPct = 0.0f;
	InitialCrouchOffset = CurrentCrouchOffset;
	TargetCrouchOffset = NewTargetOffset;
}

void UViewMode_ThirdPerson::UpdateCrouchOffset(float DeltaTime)
{
	if (CrouchOffsetBlendPct < 1.0f)
	{
		CrouchOffsetBlendPct = FMath::Min(CrouchOffsetBlendPct + DeltaTime * CrouchOffsetBlendMultiplier, 1.0f);
		CurrentCrouchOffset = FMath::InterpEaseInOut(InitialCrouchOffset, TargetCrouchOffset, CrouchOffsetBlendPct, 1.0f);
	}
	else
	{
		CurrentCrouchOffset = TargetCrouchOffset;
		CrouchOffsetBlendPct = 1.0f;
	}
}
