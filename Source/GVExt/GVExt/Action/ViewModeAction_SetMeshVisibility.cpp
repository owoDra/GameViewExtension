// Copyright (C) 2024 owoDra

#include "ViewModeAction_SetMeshVisibility.h"

#include "Mode/ViewMode.h"

#include "Character/CharacterMeshAccessorInterface.h"

#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ViewModeAction_SetMeshVisibility)


UViewModeAction_SetMeshVisibility::UViewModeAction_SetMeshVisibility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UViewModeAction_SetMeshVisibility::PostActivateMode(UViewMode* OwningViewMode)
{
	auto* Target{ OwningViewMode->GetTargetPawn() };
	if (!Target)
	{
		return;
	}

	// Lambda function to update Visibility of Mesh based on data

	auto ChangeMeshVisibility
	{
		[](USkeletalMeshComponent* InMesh, const FMeshToChangeVisibility& InVisibility)
		{
			if (InMesh)
			{
				if (InVisibility.bShouldChangeHiddenInGame)
				{
					InMesh->SetHiddenInGame(InVisibility.bHiddenInGame);
				}

				if (InVisibility.bShouldChangeOnlyOwnerSee)
				{
					InMesh->SetOnlyOwnerSee(InVisibility.bOnlyOwnerSee);
				}

				if (InVisibility.bShouldChangeOwnerNoSee)
				{
					InMesh->SetOwnerNoSee(InVisibility.bOwnerNoSee);
				}
			}
		}
	};

	// Update Visibility of the specified mesh and any child meshes attached to it

	for (const auto& MeshToChange : MeshesToChangeVisibility)
	{
		if (auto* Mesh{ ICharacterMeshAccessorInterface::Execute_GetMeshByTag(Target, MeshToChange.MeshTag) })
		{
			ChangeMeshVisibility(Mesh, MeshToChange);

			auto Children{ Mesh->GetAttachChildren() };
			for (const auto& Child : Children)
			{
				if (auto* ChildMesh{ Cast<USkeletalMeshComponent>(Child) })
				{
					ChangeMeshVisibility(ChildMesh, MeshToChange);
				}
			}
		}
	}
}

void UViewModeAction_SetMeshVisibility::PreDeactivateMode(UViewMode* OwningViewMode)
{
	auto* Target{ OwningViewMode->GetTargetPawn() };
	if (!Target)
	{
		return;
	}

	// Lambda function to update Visibility of Mesh based on data

	auto ChangeMeshVisibility
	{
		[](USkeletalMeshComponent* InMesh, const FMeshToChangeVisibility& InVisibility)
		{
			if (InMesh)
			{
				if (InVisibility.bShouldChangeHiddenInGame)
				{
					InMesh->SetHiddenInGame(!InVisibility.bHiddenInGame);
				}

				if (InVisibility.bShouldChangeOnlyOwnerSee)
				{
					InMesh->SetOnlyOwnerSee(!InVisibility.bOnlyOwnerSee);
				}

				if (InVisibility.bShouldChangeOwnerNoSee)
				{
					InMesh->SetOwnerNoSee(!InVisibility.bOwnerNoSee);
				}
			}
		}
	};

	// Update Visibility of the specified mesh and any child meshes attached to it

	for (const auto& MeshToChange : MeshesToChangeVisibility)
	{
		if (auto* Mesh{ ICharacterMeshAccessorInterface::Execute_GetMeshByTag(Target, MeshToChange.MeshTag) })
		{
			ChangeMeshVisibility(Mesh, MeshToChange);

			auto Children{ Mesh->GetAttachChildren() };
			for (const auto& Child : Children)
			{
				if (auto* ChildMesh{ Cast<USkeletalMeshComponent>(Child) })
				{
					ChangeMeshVisibility(ChildMesh, MeshToChange);
				}
			}
		}
	}
}
