// Copyright (C) 2023 owoDra

#include "ViewModeStack.h"

#include "Mode/ViewMode.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ViewModeStack)


UViewModeStack::UViewModeStack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UViewMode* UViewModeStack::GetViewModeInstance(TSubclassOf<UViewMode> ViewModeClass)
{
	check(ViewModeClass);

	// Check if an instance has already been created

	for (const auto& ViewMode : ViewModeInstances)
	{
		if ((ViewMode != nullptr) && (ViewMode->GetClass() == ViewModeClass))
		{
			return ViewMode;
		}
	}

	// Create an instance if not already created

	auto* NewViewMode{ NewObject<UViewMode>(GetOuter(), ViewModeClass, NAME_None, RF_NoFlags) };
	check(NewViewMode);

	// Add instances to an array for later reference

	ViewModeInstances.Add(NewViewMode);

	return NewViewMode;
}


void UViewModeStack::UpdateStack(float DeltaTime)
{
	const auto StackSize{ ViewModeStack.Num() };

	// If Stack is less than or equal to 0 (i.e., empty), skip

	if (StackSize <= 0)
	{
		return;
	}

	auto RemoveCount{ 0 };
	auto RemoveIndex{ static_cast<int32>(INDEX_NONE) };

	// Update all ViewModes in the Stack

	for (auto StackIndex{ 0 }; StackIndex < StackSize; ++StackIndex)
	{
		auto ViewMode{ ViewModeStack[StackIndex] };
		check(ViewMode);

		ViewMode->UpdateViewMode(DeltaTime);

		// Whether the ViewMode's BlendWeight is greater than or equal to 1.0 (i.e., Blend is complete).

		if (ViewMode->GetBlendWeight() >= 1.0f)
		{
			// Index of the first item added before the ViewMode for which Blend is complete and how many items to remove from it.

			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);

			// Set ActivationState to Activated because the Blend of Index 0 has been completed, which means that it has become fully Active.

			if (StackIndex == 0)
			{
				ViewMode->SetActivationState(EViewModeActivationState::Activated);
			}

			break;
		}
	}

	// Whether Blend is complete and whether there are items that can be excluded.

	if (RemoveCount > 0)
	{
		// Delete cached items

		for (auto StackIndex{ RemoveIndex }; StackIndex < StackSize; ++StackIndex)
		{
			auto ViewMode{ ViewModeStack[StackIndex] };
			check(ViewMode);

			// Set ActivationState to Deactevated.

			ViewMode->SetActivationState(EViewModeActivationState::Deactevated);
		}

		ViewModeStack.RemoveAt(RemoveIndex, RemoveCount);
	}
}

void UViewModeStack::BlendStack(FViewModeInfo& OutViewModeInfo) const
{
	const auto LastIndex{ ViewModeStack.Num() - 1 };

	// Skip if Stack is empty

	if (LastIndex < 0)
	{
		return;
	}

	// Blend in order from the last ViewMode in the Stack.

	auto ViewMode{ ViewModeStack[LastIndex] };
	check(ViewMode);

	OutViewModeInfo = ViewMode->GetViewModeInfo();

	for (auto StackIndex{ LastIndex - 1 }; StackIndex >= 0; --StackIndex)
	{
		ViewMode = ViewModeStack[StackIndex];
		check(ViewMode);

		OutViewModeInfo.Blend(ViewMode->GetViewModeInfo(), ViewMode->GetBlendWeight());
	}
}

void UViewModeStack::PushViewMode(TSubclassOf<UViewMode> ViewModeClass)
{
	// Whether the newly adapted ViewMode is valid or not

	if (!ViewModeClass)
	{
		return;
	}

	// Create an instance from ViewMode or get it from cache

	auto* ViewMode{ GetViewModeInstance(ViewModeClass) };
	check(ViewMode);

	auto StackSize{ ViewModeStack.Num() };

	// Whether the ViewMode you are adding is already at the top of the Stack (i.e., enabled or in progress)

	if ((StackSize > 0) && (ViewModeStack[0] == ViewMode))
	{
		return;
	}

	// Check if the ViewMode already exists in the Stack 
	// and determine the BlendWeight of the newly added ViewMode from the BlendWeight of the ViewMode in the Stack.

	auto ExistingStackIndex{ static_cast<int32>(INDEX_NONE) };
	auto ExistingStackContribution{ 1.0f };
	auto bViewModeExisted{ false };

	for (auto StackIndex{ 0 }; StackIndex < StackSize; ++StackIndex)
	{
		const auto bSameViewMode{ ViewModeStack[StackIndex] == ViewMode };

		// Continue calculating ExistingStackContribution only while the new ViewMode to be added is known to exist.

		if (bViewModeExisted == false)
		{
			if (bSameViewMode)
			{
				ExistingStackIndex = StackIndex;
				ExistingStackContribution *= ViewMode->GetBlendWeight();

				bViewModeExisted = true;
			}
			else
			{
				ExistingStackContribution *= (1.0f - ViewModeStack[StackIndex]->GetBlendWeight());
			}
		}

		// PreDeactivate the ActivationState of a ViewMode only if it is not the same as the newly added ViewMode.

		if (bSameViewMode == false)
		{
			ViewModeStack[StackIndex]->SetActivationState(EViewModeActivationState::PreDeactevate);
		}
	}

	// If the Stack does not already have a BlendWeight, it does not affect the overall BlendWeight, so it is set to 0.0.

	if (ExistingStackIndex == INDEX_NONE)
	{
		ExistingStackContribution = 0.0f;
	}

	//  If it is already in the Stack, delete it.

	else
	{
		ViewModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}

	// Determine the BlendWeight of the newly added ViewMode.

	const auto bShouldBlend{ (ViewMode->GetBlendTime() > 0.0f) && (StackSize > 0) };
	const auto BlendWeight{ bShouldBlend ? ExistingStackContribution : 1.0f };

	ViewMode->SetBlendWeight(BlendWeight);

	ViewModeStack.Insert(ViewMode, 0);
	ViewModeStack.Last()->SetBlendWeight(1.0f);

	ViewMode->SetActivationState(EViewModeActivationState::PreActivate);
}

void UViewModeStack::EvaluateStack(float DeltaTime, FViewModeInfo& OutViewModeInfo)
{
	UpdateStack(DeltaTime);
	BlendStack(OutViewModeInfo);
}
