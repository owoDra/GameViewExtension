// Copyright (C) 2023 owoDra

#include "ViewMode_FirstPerson.h"

#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ViewMode_FirstPerson)


UViewMode_FirstPerson::UViewMode_FirstPerson(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UViewMode_FirstPerson::UpdateView(float DeltaTime)
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
}

void UViewMode_FirstPerson::UpdateForTarget(float DeltaTime)
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

void UViewMode_FirstPerson::SetTargetCrouchOffset(FVector NewTargetOffset)
{
	CrouchOffsetBlendPct = 0.0f;
	InitialCrouchOffset = CurrentCrouchOffset;
	TargetCrouchOffset = NewTargetOffset;
}

void UViewMode_FirstPerson::UpdateCrouchOffset(float DeltaTime)
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
