// Copyright (C) 2024 owoDra

#include "ViewerComponent.h"

#include "Mode/ViewModeStack.h"
#include "GVExtLogs.h"

#include "InitState/InitStateTags.h"
#include "InitState/InitStateComponent.h"

#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ViewerComponent)


const FName UViewerComponent::NAME_ActorFeatureName("Viewer");

UViewerComponent::UViewerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
}


void UViewerComponent::OnRegister()
{
	Super::OnRegister();

	CameraModeStack = NewObject<UViewModeStack>(this);

	// This component can only be added to classes derived from APawn

	const auto* Pawn{ GetPawn<APawn>() };
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("[%s] on [%s] can only be added to Pawn actors."), *GetNameSafe(StaticClass()), *GetNameSafe(GetOwner()));

	// No more than two of these components should be added to a Actor.

	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one [%s] should exist on [%s]."), *GetNameSafe(StaticClass()), *GetNameSafe(GetOwner()));

	// Register this component in the GameFrameworkComponentManager.

	RegisterInitStateFeature();
}

void UViewerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for changes in the initialization state of all features 
	// related to the Pawn that owns this component.

	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Change the initialization state of this component to [Spawned]

	ensureMsgf(TryToChangeInitState(TAG_InitState_Spawned), TEXT("[%s] on [%s]."), *GetNameSafe(this), *GetNameSafe(GetOwner()));

	// Check if initialization process can continue

	CheckDefaultInitialization();
}

void UViewerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}


bool UViewerComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	/**
	 * [InitState None] -> [InitState Spawned]
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		if (GetOwner() == nullptr)
		{
			return false;
		}

		return CanChangeInitStateToSpawned(Manager);
	}

	/**
	 * [InitState Spawned] -> [InitState DataAvailable]
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		if (!Manager->HasFeatureReachedInitState(GetOwner(), UInitStateComponent::NAME_ActorFeatureName, TAG_InitState_DataAvailable))
		{
			return false;
		}

		return CanChangeInitStateToDataAvailable(Manager);
	}

	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		if (GetOwner()->GetNetMode() != ENetMode::NM_DedicatedServer)
		{
			if (!DefaultViewMode)
			{
				return false;
			}
		}

		return CanChangeInitStateToDataInitialized(Manager);
	}

	/**
	 * [InitState DataInitialized] -> [InitState GameplayReady]
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		return CanChangeInitStateToGameplayReady(Manager);
	}

	return false;
}

void UViewerComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	check(Manager);

	UE_LOG(LogGVE, Log, TEXT("[%s] %s: InitState Reached: %s"),
		GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
		*GetNameSafe(this),
		*DesiredState.GetTagName().ToString());

	/**
	 * [InitState None] -> [InitState Spawned]
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		HandleChangeInitStateToSpawned(Manager);
	}

	/**
	 * [InitState Spawned] -> [InitState DataAvailable]
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		HandleChangeInitStateToDataAvailable(Manager);
	}

	/**
	 * [InitState DataAvailable] -> [InitState DataInitialized]
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		HandleChangeInitStateToDataInitialized(Manager);
	}

	/**
	 * [InitState DataInitialized] -> [InitState GameplayReady]
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		HandleChangeInitStateToGameplayReady(Manager);
	}
}

void UViewerComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UInitStateComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UViewerComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain
	{
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}


TSubclassOf<UViewMode> UViewerComponent::DetermineViewMode() const
{
	return OverrideViewMode ? OverrideViewMode : DefaultViewMode;
}

void UViewerComponent::InitializeViewMode(TSubclassOf<UViewMode> InViewModeClass)
{
	if (DefaultViewMode != InViewModeClass)
	{
		DefaultViewMode = InViewModeClass;

		CheckDefaultInitialization();
	}
}

void UViewerComponent::SetViewModeOverride(TSubclassOf<UViewMode> InViewModeClass)
{
	if (OverrideViewMode != InViewModeClass)
	{
		OverrideViewMode = InViewModeClass;
	}
}

void UViewerComponent::ClearViewModeOverride()
{
	OverrideViewMode = nullptr;
}


void UViewerComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);

	if (auto Class{ DetermineViewMode() })
	{
		CameraModeStack->PushViewMode(Class);
	}

	ComputeCameraView(DeltaTime, DesiredView);
}

void UViewerComponent::ComputeCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	auto* PlayerController{ GetController<APlayerController>() };
	check(PlayerController);
	
	FViewModeInfo CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

	ControlRotationDelta = (CameraModeView.ControlRotation - PreviousControlRotation);
	PreviousControlRotation = CameraModeView.ControlRotation;
	
	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	
	FieldOfView = CameraModeView.FieldOfView;
	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}
}


UViewerComponent* UViewerComponent::FindViewerComponent(const APawn* Pawn)
{
	return (Pawn ? Pawn->FindComponentByClass<UViewerComponent>() : nullptr);
}
