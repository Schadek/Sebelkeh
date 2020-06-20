// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffSkill.h"
#include "CourtOfSebelkeh/Components/Buffs/BuffComponent.h"
#include "CourtOfSebelkeh/Components/Skills/SkillComponent.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"

UBuffSkill::UBuffSkill()
{
	DescriptionParserCallback Callback;
	Callback.BindUObject(this, &UBuffSkill::ParseDuration);
	DescriptionParser.RegisterKeyword("Duration", Callback);
}

FString UBuffSkill::ParseDuration(const FString& Keyword) const
{
	if (bUseFixedBuffDuration)
	{
		return FString::SanitizeFloat(FixedDuration, 0);
	}
	else
	{
		return FString::SanitizeFloat(GetBuffDuration(), 0);
	}
}

void UBuffSkill::Begin(USkillComponent* InOwner, bool bAutoEquip)
{
	Super::Begin(InOwner, bAutoEquip);

	if (BuffClass)
	{
		DescriptionParser.AddDescribedObject(BuffClass->GetDefaultObject());
	}
}

void UBuffSkill::UseTargetAuthority(AActor* Target)
{
	Super::UseTargetAuthority(Target);

	switch (TargetLogic)
	{
	case EBuffSkillTargetLogic::SingleTarget:
	{
		if (UBuffComponent* BuffComponent = Target->FindComponentByClass<UBuffComponent>())
		{
			float Duration = bUseFixedBuffDuration ? FixedDuration : GetBuffDuration();
			if (UBuff* Buff = BuffComponent->AddBuff(BuffClass, Owner->GetOwner(), this, Duration))
			{
				OnBuffCreated(Buff);
			}
		}
		break;
	}
	case EBuffSkillTargetLogic::AllInArea:
	{
		const float EffectiveRange = BuffRangePreset != ERangePreset::None ? UCoreBlueprintLibrary::GetRangeFromPreset(this, BuffRangePreset) : BuffRange;
		 
		TArray<AActor*> Targets;
		UCoreBlueprintLibrary::GetValidTargetsInArea(this, Owner->GetOwner(), PossibleTargets, Target->GetActorLocation(), EffectiveRange, TArray<AActor*>(), Targets);

		for (AActor* NearbyTarget : Targets)
		{
			if (UBuffComponent* BuffComponent = NearbyTarget->FindComponentByClass<UBuffComponent>())
			{
				float Duration = bUseFixedBuffDuration ? FixedDuration : GetBuffDuration();
				if (UBuff* Buff = BuffComponent->AddBuff(BuffClass, Owner->GetOwner(), this, Duration))
				{
					OnBuffCreated(Buff);
				}
			}
		}
		break;
	}
	}
}

void UBuffSkill::PayCostSpecific(UStatComponent* StatComponent, ESkillCost CostType, int32& Amount)
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
	Buff->SetInstigatorSkill(this);
}

#if WITH_EDITOR
bool UBuffSkill::CanEditChange(const FProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UBuffSkill, BuffRange))
	{
		return BuffRangePreset == ERangePreset::None && TargetLogic == EBuffSkillTargetLogic::AllInArea;
	}
	else if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UBuffSkill, BuffRangePreset))
	{
		return TargetLogic == EBuffSkillTargetLogic::AllInArea;
	}

	return ParentVal;
}
#endif
