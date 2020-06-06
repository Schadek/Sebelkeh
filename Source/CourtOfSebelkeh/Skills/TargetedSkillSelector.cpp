// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetedSkillSelector.h"
#include "CourtOfSebelkeh/Controller/CorePlayerController.h"
#include "CourtOfSebelkeh/Skills/SkillBase.h"
#include "CourtOfSebelkeh/Components/Skills/SkillComponent.h"
#include "CourtOfSebelkeh/Components/Meta/ActorMetaComponent.h"
#include "CourtOfSebelkeh/Components/Selectable/SelectableComponent.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "GameFramework/Pawn.h"

void ATargetedSkillSelector::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* Pawn = Cast<APawn>(OwnerSkill->GetSkillComponent()->GetOwner()))
	{
		if (ACorePlayerController* PlayerController = Cast<ACorePlayerController>(Pawn->GetController()))
		{
			if (USelectableComponent* Selectable = PlayerController->GetSelected())
			{
				if (CanBeUsedAgainst(Selectable->GetOwner()))
				{
					OwnerSkill->UseTarget(Selectable->GetOwner());
					return;
				}
			}
			else if (CanBeUsedAgainst(Pawn))
			{
				OwnerSkill->UseTarget(Pawn);
				return;
			}


			OwnerSkill->NotifyGameText(InvalidTargetText);
			OwnerSkill->Deselect();
		}
	}
}

bool ATargetedSkillSelector::CanBeUsedAgainst(AActor* Target) const
{
	if (OwnerSkill->GetSkillComponent()->GetOwner() == Target)
	{
		return (static_cast<uint8>(OwnerSkill->GetPossibleTargets()) & (1 << static_cast<uint8>(ESkillTarget::Self))) != 0;
	}

	uint8 FactionState = static_cast<uint8>(UCoreBlueprintLibrary::GetActorFactionState(Target));
	uint8 PossibleTargets = OwnerSkill->GetPossibleTargets();
	return (PossibleTargets & (1 << FactionState)) != 0;
}
