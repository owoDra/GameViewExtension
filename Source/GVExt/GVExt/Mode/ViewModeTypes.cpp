// Copyright (C) 2023 owoDra

#include "ViewModeTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ViewModeTypes)


FViewModeInfo::FViewModeInfo()
	: Location(ForceInit)
	, Rotation(ForceInit)
	, ControlRotation(ForceInit)
	, FieldOfView(90.0f)
{
}

void FViewModeInfo::Blend(const FViewModeInfo& Other, float OtherWeight)
{
	if (OtherWeight <= 0.0f)
	{
		return;
	}
	else if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const auto DeltaRotation{ (Other.Rotation - Rotation).GetNormalized() };
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const auto DeltaControlRotation{ (Other.ControlRotation - ControlRotation).GetNormalized() };
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}
