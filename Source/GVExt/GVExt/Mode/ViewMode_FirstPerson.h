// Copyright (C) 2023 owoDra

#pragma once

#include "Mode/ViewMode.h"

#include "ViewMode_FirstPerson.generated.h"


/**
 * ViewMode base class for FPP viewpoint
 */
UCLASS(Abstract, Blueprintable)
class GVEXT_API UViewMode_FirstPerson : public UViewMode
{
	GENERATED_BODY()
public:
	UViewMode_FirstPerson(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//
	// Transition speed when crouching
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "First Person")
	float CrouchOffsetBlendMultiplier{ 8.0f };

protected:
	float CrouchOffsetBlendPct{ 1.0f };
	FVector InitialCrouchOffset{ FVector::ZeroVector };
	FVector TargetCrouchOffset{ FVector::ZeroVector };
	FVector CurrentCrouchOffset{ FVector::ZeroVector };

protected:
	virtual void UpdateView(float DeltaTime) override;
	void UpdateForTarget(float DeltaTime);

	void SetTargetCrouchOffset(FVector NewTargetOffset);
	void UpdateCrouchOffset(float DeltaTime);

};
