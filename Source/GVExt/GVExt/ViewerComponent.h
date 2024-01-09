// Copyright (C) 2024 owoDra

#pragma once

#include "Camera/CameraComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "Mode/ViewModeTypes.h"

#include "GameplayTagContainer.h"

#include "ViewerComponent.generated.h"

class UViewModeStack;
class UViewMode;


/**
 * Components that perform processing to enable the player to control the viewpoint
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GVEXT_API UViewerComponent 
	: public UCameraComponent
	, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	UViewerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//
	// Function name used to add this component
	//
	static const FName NAME_ActorFeatureName;

protected:
	UPROPERTY()
	TObjectPtr<UViewModeStack> CameraModeStack;

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;

protected:
	virtual bool CanChangeInitStateToSpawned(UGameFrameworkComponentManager* Manager) const { return true; }
	virtual bool CanChangeInitStateToDataAvailable(UGameFrameworkComponentManager* Manager) const { return true; }
	virtual bool CanChangeInitStateToDataInitialized(UGameFrameworkComponentManager* Manager) const { return true; }
	virtual bool CanChangeInitStateToGameplayReady(UGameFrameworkComponentManager* Manager) const { return true; }

	virtual void HandleChangeInitStateToSpawned(UGameFrameworkComponentManager* Manager) {}
	virtual void HandleChangeInitStateToDataAvailable(UGameFrameworkComponentManager* Manager) {}
	virtual void HandleChangeInitStateToDataInitialized(UGameFrameworkComponentManager* Manager) {}
	virtual void HandleChangeInitStateToGameplayReady(UGameFrameworkComponentManager* Manager) {}


protected:
	UPROPERTY(Transient)
	TSubclassOf<UViewMode> DefaultViewMode{ nullptr };

	UPROPERTY(Transient)
	TSubclassOf<UViewMode> OverrideViewMode{ nullptr };

protected:
	TSubclassOf<UViewMode> DetermineViewMode() const;

public:
	/**
	 * Set default ViewMode and initialize
	 */
	UFUNCTION(BlueprintCallable, Category = "View")
	void InitializeViewMode(TSubclassOf<UViewMode> InViewModeClass);

	/**
	 * Override ViewMode
	 */
	UFUNCTION(BlueprintCallable, Category = "View")
	void SetViewModeOverride(TSubclassOf<UViewMode> InViewModeClass);

	/**
	 * Cancel overridden ViewMode
	 */
	UFUNCTION(BlueprintCallable, Category = "View")
	void ClearViewModeOverride();


protected:
	FRotator PreviousControlRotation;
	FRotator ControlRotationDelta;

protected:
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	/**
	 * Calculate final viewpoint information for Camera
	 */
	virtual void ComputeCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);


public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Components")
	static UViewerComponent* FindCameraComponent(const APawn* Pawn);

	/**
	 * Returns the difference in ControlRotation from the previous cached frame
	 */
	UFUNCTION(BlueprintPure, Category = "View")
	FRotator GetControlRotationDelta() const { return ControlRotationDelta; }


public:
	template <class T>
	T* GetPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' template parameter to GetPawn must be derived from APawn");
		return Cast<T>(GetOwner());
	}

	template <class T>
	T* GetPawnChecked() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' template parameter to GetPawnChecked must be derived from APawn");
		return CastChecked<T>(GetOwner());
	}

	template <class T>
	T* GetPlayerState() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APlayerState>::Value, "'T' template parameter to GetPlayerState must be derived from APlayerState");
		return GetPawnChecked<APawn>()->GetPlayerState<T>();
	}

	template <class T>
	T* GetController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "'T' template parameter to GetController must be derived from AController");
		return GetPawnChecked<APawn>()->GetController<T>();
	}

};
