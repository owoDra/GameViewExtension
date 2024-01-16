// Copyright (C) 2024 owoDra

#pragma once

#include "ViewModeAction.generated.h"

class UViewMode;


/**
 * Base class for processing to be performed when ViewMode is applied
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, NotBlueprintable)
class GVEXT_API UViewModeAction : public UObject
{
	GENERATED_BODY()

	friend class UViewMode;

public:
	UViewModeAction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/**
	 * Called when Blend starts before ViewMode becomes Active.
	 */
	virtual void PreActivateMode(UViewMode* OwningViewMode) {}

	/**
	 * Called when Blend in ViewMode ends and becomes Active.
	 */
	virtual void PostActivateMode(UViewMode* OwningViewMode) {}

	/**
	 * Called when Blend starts before ViewMode becomes Deactive.
	 */
	virtual void PreDeactivateMode(UViewMode* OwningViewMode) {}

	/**
	 * Called when Blend in ViewMode ends and becomes Deactive.
	 */
	virtual void PostDeactivateMode(UViewMode* OwningViewMode) {}

};
