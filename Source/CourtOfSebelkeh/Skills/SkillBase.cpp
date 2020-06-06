// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillBase.h"
#include "CourtOfSebelkeh/Skills/SkillSelector.h"
#include "CourtOfSebelkeh/GameStates/CoreGameState.h"
#include "CourtOfSebelkeh/Damage/DamageSystem.h"
#include "CourtOfSebelkeh/Components/Skills/SkillComponent.h"
#include "CourtOfSebelkeh/Components/Stats/StatComponent.h"
#include "CourtOfSebelkeh/Characters/CoreCharacter.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "GameFramework/GameStateBase.h"

bool USkillBase::IsUsable_Implementation(FText& UsabilityMessage) const
{
	if (RemainingCooldown > 0.0f)
	{
		UsabilityMessage = FText::FromString(TEXT("This skill is on cooldown!"));
		return false;
	}

	if (!CanPayCost())
	{
		UsabilityMessage = FText::FromString(TEXT("You cannot afford to use this skill!"));
		return false;
	}

	return true;
}

void USkillBase::PayCost()
{
	UStatComponent* StatComponent = Owner->GetOwner()->FindComponentByClass<UStatComponent>();
	USkillComponent* SkillComponent = Owner;

	if (!StatComponent || !SkillComponent)
		return;

	for (auto& Pair : Costs)
	{
		PayCostSpecific(StatComponent, Pair.Key, Pair.Value);
	}
}

void USkillBase::PayCostSpecific(UStatComponent* StatComponent, ESkillCost CostType, int32 Amount)
{
	switch (CostType)
	{
	case ESkillCost::Energy:
	{
		StatComponent->AddStat(EStat::Energy, -Amount);
		break;
	}
	case ESkillCost::Adrenaline:
	{
		Owner->AddAdrenaline(this, Amount);
		break;
	}
	case ESkillCost::HealthPercentage:
	{
		int32 MaxHealth = StatComponent->GetStatRawNoCheck(EStat::MaxHealth);

		FDamageInfo Info;
		Info.Amount = static_cast<int32>(MaxHealth * (0.01f * Amount));
		Info.Instigator = Owner->GetOwner();
		Info.Source = this;
		Info.Target = Owner->GetOwner();
		Info.Type = UCoreBlueprintLibrary::GetGameSettings(this)->HealthSacrificeDamageType;

		UCoreBlueprintLibrary::GetCoreGameState(this)->GetDamageSystem()->ProcessDamage(Info);
		break;
	}
	}
}

void USkillBase::Select()
{
	FText OutUsabilityText;
	if (!IsUsable(OutUsabilityText))
	{
		Owner->NotifyOwner(OutUsabilityText);
		return;
	}

	//If a skill indicator was assigned
	if (SkillSelectorTemplate)
	{
		ActiveSkillSelector = GetWorld()->SpawnActorDeferred<ASkillSelector>(SkillSelectorTemplate, FTransform());
		ActiveSkillSelector->SetSkill(this);
		bIsSelected = true;

		ActiveSkillSelector->FinishSpawning(FTransform());
	}
}

void USkillBase::Deselect()
{
	if (ActiveSkillSelector)
	{
		ActiveSkillSelector->CancelSelection();
	}

	bIsSelected = false;
	ReceiveDeselect();
}

void USkillBase::NotifyGameText(const FText& Text)
{
	Owner->NotifyOwner(Text);
}

bool USkillBase::IsLocalSkill() const
{
	if (APawn* Pawn = Cast<APawn>(GetSkillComponent()->GetOwner()))
	{
		return Pawn->IsLocallyControlled();
	}

	return true;
}

bool USkillBase::IsActorInRange(AActor* Target, float DistanceMultiplier) const
{
	return Target->GetHorizontalDistanceTo(GetSkillComponent()->GetOwner()) <= Range * DistanceMultiplier;
}

ACoreCharacter* USkillBase::GetOwnerAsCharacter() const
{
	return Cast<ACoreCharacter>(Owner->GetOwner());
}

void USkillBase::OnHoverExit()
{

}

void USkillBase::OnHoverEnter()
{

}

bool USkillBase::CanPayCost() const
{
	UStatComponent* StatComponent = Owner->GetOwner()->FindComponentByClass<UStatComponent>();
	USkillComponent* SkillComponent = Owner;

	if (!StatComponent || !SkillComponent)
		return false;

	for (auto& Pair : Costs)
	{
		const bool bCanPay = CanPaySpecificCost(StatComponent, Pair.Key, Pair.Value);
		if (!bCanPay)
			return false;
	}

	return true;
}

bool USkillBase::CanPaySpecificCost(UStatComponent* StatComponent, ESkillCost CostType, int32 Amount) const
{
	switch (CostType)
	{
	case ESkillCost::Energy:
		return StatComponent->GetStatRealNoCheck(EStat::Energy) >= Amount;
	case ESkillCost::Adrenaline:
		return Owner->GetAdrenaline(this) >= Amount;
	case ESkillCost::HealthPercentage:
	{
		const int32 MaxHealth = StatComponent->GetStatRealNoCheck(EStat::MaxHealth);
		const int32 Health = StatComponent->GetStatRealNoCheck(EStat::Health);
		return ((0.01f * Amount) * MaxHealth) >= Health;
	}
	}

	return true;
}

void USkillBase::UseLocation(FVector Location)
{
	//If executing on the server, directly call the server RPC. Otherwise use the
	//local controller as proxy.
	if (GetWorld()->IsServer())
	{
		UseLocationInternal(Location);
	}
	else
	{
		GetSkillComponent()->UseSkillOnLocation(this, Location);
	}

	Deselect();
}

int32 USkillBase::GetAttributeValue() const
{
	return Owner->GetAttributeValue(Attribute);
}

AActor* USkillBase::GetSkillOwner() const
{
	return Owner->GetOwner();
}

void USkillBase::UseTarget(AActor* Target)
{
	//If executing on the server, directly call the server RPC. Otherwise use the
	//local controller as proxy.
	if (GetWorld()->IsServer())
	{
		const bool bIsLocalSkill = IsLocalSkill();
		if (IsActorInRange(Target, bIsLocalSkill ? 1.f : 1.1f))
		{
			UseTargetInternal(Target);
		}
		else if (bIsLocalSkill)
		{
			//Player not in range. Try to walk into range.
			if (ACoreCharacter* Character = GetOwnerAsCharacter())
			{
				Character->FollowActor(Target, Range);
				bIsWalkingTowardsTarget = true;
			}
		}
	}
	else
	{
		if (IsActorInRange(Target))
		{
			GetSkillComponent()->UseSkillOnTarget(this, Target);
		}
		else
		{
			//Player not in range. Try to walk into range.
			if (ACoreCharacter* Character = GetOwnerAsCharacter())
			{
				Character->FollowActor(Target, Range);
				bIsWalkingTowardsTarget = true;
			}
		}
	}

	Deselect();
}

void USkillBase::UseLocationInternal(FVector Location)
{
}

void USkillBase::UseTargetAuthority(AActor* Target)
{
	ReceiveUseTarget(Target);
}

void USkillBase::UseLocationAuthority(const FVector& Location)
{
	ReceiveUseLocation(Location);
}

void USkillBase::OnOwnerReachedTarget(ACoreCharacter* Character, AActor* Target, const FVector& Location, float TargetRange)
{
	if (bIsWalkingTowardsTarget)
	{
		ensure(!bIsWalkingTowardsLocation);
		UseTarget(Target);
	}
	else if (bIsWalkingTowardsLocation)
	{
		UseLocation(Location);
	}

	bIsWalkingTowardsLocation = false;
	bIsWalkingTowardsTarget = false;
}

void USkillBase::OnOwnerAbortedWalk(ACoreCharacter* Character, AActor* Target, const FVector& Location, float TargetRange)
{
	bIsWalkingTowardsTarget = false;
	bIsWalkingTowardsLocation = false;
}

void USkillBase::UseLocationMulticast_Implementation(FVector Location)
{
	/*GetOwnerCharacter()->OnSkillUse(this, Location, nullptr);
	ReceiveUseLocationCosmetic(Location);*/
}

void USkillBase::UseTargetInternal(AActor* Target)
{
	FText OutUsabilityText;
	if (!IsUsable(OutUsabilityText))
	{
		Owner->NotifyOwner(OutUsabilityText);
		return;
	}

	if (ChannelTime > 0.0f)
	{
		PayCost();
		Owner->ChannelSkill(this, Target, FVector::ZeroVector);
		SetOnCooldown();
	}
	else
	{
		UseTargetAuthority(Target);
	}
}

void USkillBase::UseTargetAfterChannel(AActor* Target)
{
	UseTargetAuthority(Target);
}

void USkillBase::UseLocationAfterChannel(FVector Location)
{
	UseLocationAuthority(Location);
}

void USkillBase::UseTargetMulticast_Implementation(AActor* Target, FVector_NetQuantize Location, FRotator Rotation)
{
	/*FTransform ReconstructedTransform(Rotation, Location, FVector(1, 1, 1));
	GetOwnerCharacter()->OnSkillUse(this, ReconstructedTransform.GetLocation(), Target);*/
}

void USkillBase::SetOnCooldown()
{
	Owner->SetSkillOnCooldown(this);
}

void USkillBase::SyncCooldown(float EndTimestamp)
{
	float Cooldown = EndTimestamp - GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	Deselect();

	RemainingCooldown = Cooldown;
	CurrentCooldownStart = Cooldown;
}

float USkillBase::GetRemainingCooldown() const
{
	return RemainingCooldown;
}

float USkillBase::GetRemainingCooldownFraction() const
{
	float Cooldown = RemainingCooldown / CurrentCooldownStart;

	return FMath::IsNaN(Cooldown) ? 1 : (1 - Cooldown);
}

float USkillBase::GetBaseCooldown() const
{
	return BaseCooldown;
}

float USkillBase::GetAdjustedCooldown() const
{
	float CDR = 0;

	if (GetSkillComponent())
	{
		//CDR = GetOwnerCharacter()->GetStatComponent()->Get(TSubclassOf<UStat>::CooldownReduction);
	}

	return BaseCooldown * (1 - CDR);
}

USkillComponent* USkillBase::GetSkillComponent() const
{
	return Owner;
}

bool USkillBase::GetCost(ESkillCost CostType, int32& OutAmount) const
{
	for (auto Pair : Costs)
	{
		if (Pair.Key == CostType)
		{
			OutAmount = Pair.Value;
			return true;
		}
	}

	return false;
}

FText USkillBase::GetFormattedSkillDescription_Implementation(bool bPreview) const
{
	return FText::FromString("EMPTY_SKILL_DESCRIPTION");
}

UWorld* USkillBase::GetWorld() const
{
	return Owner ? Owner->GetWorld() : nullptr;
}

void USkillBase::Begin(USkillComponent* InOwner)
{
	Owner = InOwner;

	if (RangePreset != ERangePreset::None)
	{
		Range = UCoreBlueprintLibrary::GetRangeFromPreset(this, RangePreset);
	}

	if (ACoreCharacter* Character = GetOwnerAsCharacter())
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, "OnOwnerReachedTarget");
		Character->OnFollowTargetReached.Add(Delegate);

		FScriptDelegate Delegate2;
		Delegate2.BindUFunction(this, "OnOwnerAbortedWalk");
		Character->OnFollowTargetAborted.Add(Delegate2);
	}
}

void USkillBase::End()
{
	if (ACoreCharacter* Character = GetOwnerAsCharacter())
	{
		Character->OnFollowTargetReached.RemoveAll(this);
	}
}

void USkillBase::Tick(float DeltaSeconds)
{
	RemainingCooldown -= DeltaSeconds;
	RemainingCooldown = FMath::Max<float>(0, RemainingCooldown);
}

#if WITH_EDITOR
bool USkillBase::CanEditChange(const FProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(USkillBase, Range))
	{
		return RangePreset == ERangePreset::None;
	}

	return ParentVal;
}
#endif