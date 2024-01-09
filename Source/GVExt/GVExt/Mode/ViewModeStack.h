// Copyright (C) 2024 owoDra

#pragma once

#include "ViewModeTypes.h"

#include "ViewModeStack.generated.h"

class UViewMode;


/**
 * Stack to manage and blend ViewModes.
 */
UCLASS()
class UViewModeStack : public UObject
{
	GENERATED_BODY()
public:
	UViewModeStack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY()
	TArray<TObjectPtr<UViewMode>> ViewModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UViewMode>> ViewModeStack;

protected:
	UViewMode* GetViewModeInstance(TSubclassOf<UViewMode> ViewModeClass);

	/**
	 * Update the ViewMode in the Stack.
	 */
	void UpdateStack(float DeltaTime);

	/**
	 * Blend of ViewMode in Stack
	 */
	void BlendStack(FViewModeInfo& OutViewModeInfo) const;

public:
	/**
	 * Add a new ViewMode to the beginning of the Stack and start Blend.
	 */
	void PushViewMode(TSubclassOf<UViewMode> ViewModeClass);

	/**
	 * Called by ViewerComponent to update Stack and return final output data
	 */
	void EvaluateStack(float DeltaTime, FViewModeInfo& OutViewModeInfo);

};
