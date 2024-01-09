// Copyright (C) 2024 owoDra

#include "ViewMode.h"

#include "Mode/ViewModeAction.h"
#include "ViewerComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ViewMode)


UViewMode::UViewMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UViewMode::SetActivationState(EViewModeActivationState NewActivationState)
{
	if (ActivationState == NewActivationState)
	{
		return;
	}

	ActivationState = NewActivationState;

	if (ActivationState == EViewModeActivationState::PreActivate)
	{
		PreActivateMode();
	}
	else if (ActivationState == EViewModeActivationState::Activated)
	{
		PostActivateMode();
	}
	else if (ActivationState == EViewModeActivationState::PreDeactevate)
	{
		PreDeactivateMode();
	}
	else if (ActivationState == EViewModeActivationState::Deactevated)
	{
		PostDeactivateMode();
	}
}

void UViewMode::PreActivateMode()
{
	for (const auto& Action : Actions)
	{
		if (Action)
		{
			Action->PreActivateMode(this);
		}
	}
}

void UViewMode::PostActivateMode()
{
	for (const auto& Action : Actions)
	{
		if (Action)
		{
			Action->PostActivateMode(this);
		}
	}
}

void UViewMode::PreDeactivateMode()
{
	for (const auto& Action : Actions)
	{
		if (Action)
		{
			Action->PreDeactivateMode(this);
		}
	}
}

void UViewMode::PostDeactivateMode()
{
	for (const auto& Action : Actions)
	{
		if (Action)
		{
			Action->PostDeactivateMode(this);
		}
	}
}


FVector UViewMode::GetPivotLocation() const
{
	const auto* TargetPawn{ GetTargetPawnChecked() };

	// Height adjustments for characters to account for crouching.

	if (const auto* TargetCharacter{ Cast<ACharacter>(TargetPawn) })
	{
		const auto* TargetCharacterCDO{ TargetCharacter->GetClass()->GetDefaultObject<ACharacter>() };
		check(TargetCharacterCDO);

		const auto* CapsuleComp{ TargetCharacter->GetCapsuleComponent() };
		check(CapsuleComp);

		const auto* CapsuleCompCDO{ TargetCharacterCDO->GetCapsuleComponent() };
		check(CapsuleCompCDO);

		const auto DefaultHalfHeight{ CapsuleCompCDO->GetUnscaledCapsuleHalfHeight() };
		const auto ActualHalfHeight{ CapsuleComp->GetUnscaledCapsuleHalfHeight() };
		const auto HeightAdjustment{ (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight };

		return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment);
	}

	return TargetPawn->GetPawnViewLocation();
}

FRotator UViewMode::GetPivotRotation() const
{
	const auto* TargetPawn{ GetTargetPawnChecked() };

	return TargetPawn->GetViewRotation();
}


void UViewMode::UpdateView(float DeltaTime)
{
	auto PivotLocation{ GetPivotLocation() };
	auto PivotRotation{ GetPivotRotation() };

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void UViewMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const auto Exponent{ (BlendExponent > 0.0f) ? BlendExponent : 1.0f };

	switch (BlendFunction)
	{
	case EViewModeBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case EViewModeBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EViewModeBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EViewModeBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	default:
		checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UViewMode::UpdateViewMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void UViewMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.

	const auto InvExponent{ (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f };

	switch (BlendFunction)
	{
	case EViewModeBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case EViewModeBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EViewModeBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EViewModeBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	default:
		checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}


UWorld* UViewMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UViewMode::GetTarget() const
{
	auto* Component{ GetViewerComponent() };

	return Component ? Component->GetOwner() : nullptr;
}
