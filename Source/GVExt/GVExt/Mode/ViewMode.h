// Copyright (C) 2024 owoDra

#pragma once

#include "ViewModeTypes.h"

#include "ViewMode.generated.h"

class UViewerComponent;
class UViewModeAction;


/**
 * Base class for specification of camera view
 */
UCLASS(Abstract, NotBlueprintable)
class GVEXT_API UViewMode : public UObject
{
	GENERATED_BODY()
public:
	UViewMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView{ 90.0f };

	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin{ -89.0f };

	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax{ 89.0f };

	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EViewModeBlendFunction BlendFunction{ EViewModeBlendFunction::EaseOut };

	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime{ 0.5f };

	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent{ 4.0f };

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Action")
	TArray<TObjectPtr<UViewModeAction>> Actions;


protected:
	UPROPERTY(Transient)
	EViewModeActivationState ActivationState{ EViewModeActivationState::Deactevated };
	
public:
	/**
	 * Change the ActivationState of the ViewMode
	 */
	virtual void SetActivationState(EViewModeActivationState NewActivationState);

protected:
	/**
	 * Called when Blend starts before ViewMode becomes Active.
	 */
	virtual void PreActivateMode();

	/**
	 * Called when Blend in ViewMode ends and becomes Active.
	 */
	virtual void PostActivateMode();

	/**
	 * Called when Blend starts before ViewMode becomes Deactive.
	 */
	virtual void PreDeactivateMode();

	/**
	 * Called when Blend in ViewMode ends and becomes Deactive.
	 */
	virtual void PostDeactivateMode();


protected:
	UPROPERTY(Transient)
	float BlendAlpha{ 1.0f };

	UPROPERTY(Transient)
	float BlendWeight{ 1.0f };

	UPROPERTY(Transient)
	uint32 bResetInterpolation : 1{ false };

	FViewModeInfo View;

protected:
	virtual FVector GetPivotLocation() const;
	virtual FRotator GetPivotRotation() const;

	virtual void UpdateView(float DeltaTime);
	virtual void UpdateBlending(float DeltaTime);

public:
	void UpdateViewMode(float DeltaTime);

	void SetBlendWeight(float Weight);

	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	const FViewModeInfo& GetViewModeInfo() const { return View; }


public:
	virtual UWorld* GetWorld() const override;

	virtual AActor* GetTarget() const;

	template<typename T = UViewerComponent>
	T* GetViewerComponent() const
	{
		return GetTypedOuter<T>();
	}

	template<typename T = APawn>
	T* GetTargetPawn() const
	{
		return Cast<T>(GetTarget());
	}

	template<typename T = APawn>
	T* GetTargetPawnChecked() const
	{
		return CastChecked<T>(GetTarget());
	}

};
