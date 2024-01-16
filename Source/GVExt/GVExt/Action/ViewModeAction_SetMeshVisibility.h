// Copyright (C) 2024 owoDra

#pragma once

#include "Action/ViewModeAction.h"

#include "GameplayTagContainer.h"

#include "ViewModeAction_SetMeshVisibility.generated.h"


/**
 * Entry data for the mesh to be changed
 */
USTRUCT(BlueprintType)
struct FMeshToChangeVisibility
{
	GENERATED_BODY()
public:
	FMeshToChangeVisibility() {}

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag MeshTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (InlineEditConditionToggle))
	bool bShouldChangeHiddenInGame{ false };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bShouldChangeHiddenInGame"))
	bool bHiddenInGame{ false };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (InlineEditConditionToggle))
	bool bShouldChangeOnlyOwnerSee{ false };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bShouldChangeOnlyOwnerSee"))
	bool bOnlyOwnerSee{ false };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (InlineEditConditionToggle))
	bool bShouldChangeOwnerNoSee{ false };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bShouldChangeOwnerNoSee"))
	bool bOwnerNoSee{ false };

};


/**
 * ViewModeAction to change the Visibility of the specified mesh of the viewed character
 * 
 * Note:
 *	This change is only applied locally.
 */
UCLASS(DefaultToInstanced, EditInlineNew, NotBlueprintable)
class GVEXT_API UViewModeAction_SetMeshVisibility : public UViewModeAction
{
	GENERATED_BODY()
public:
	UViewModeAction_SetMeshVisibility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(EditAnywhere, Category = "SetMeshVisibility")
	TArray<FMeshToChangeVisibility> MeshesToChangeVisibility;

protected:
	virtual void PostActivateMode(UViewMode* OwningViewMode) override;
	virtual void PreDeactivateMode(UViewMode* OwningViewMode) override;

};
