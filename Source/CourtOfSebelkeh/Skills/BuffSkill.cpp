// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffSkill.h"
#include "CourtOfSebelkeh/Components/Buffs/BuffComponent.h"
#include "CourtOfSebelkeh/Components/Skills/SkillComponent.h"

void UBuffSkill::UseTargetAuthority(AActor* Target)
{
	Super::UseTargetAuthority(Target);

	if (UBuffComponent* BuffComponent = Target->FindComponentByClass<UBuffComponent>())
	{
		float Duration = bUseFixedBuffDuration ? FixedDuration : GetBuffDuration();
		if (UBuff* Buff = BuffComponent->AddBuff(BuffClass, Owner->GetOwner(), Duration))
		{
			OnBuffCreated(Buff);
		}
	}
}

void UBuffSkill::PayCostSpecific(UStatComponent* StatComponent, ESkillCost CostType, int32 Amount)
{
	Super::PayCostSpecific(StatComponent, CostType, Amount);

	if (CostType == ESkillCost::EnergyRegeneration)
	{
		MaintenanceCosts.Add(EStat::EnergyRegeneration, Amount);
	}
}

float UBuffSkill::GetBuffDuration_Implementation() const
{
	return -1.f;
}

void UBuffSkill::OnBuffCreated_Implementation(UBuff* Buff)
{
	Buff->SetMaintenanceCosts(MaintenanceCosts);
}
