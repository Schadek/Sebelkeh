// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffSystemDefinitions.h"
#include "CourtOfSebelkeh/GameStates/CoreGameState.h"
#include "CourtOfSebelkeh/Components/CallbackComponent.h"
#include "CourtOfSebelkeh/Components/Stats/StatComponent.h"
#include "CourtOfSebelkeh/Components/Buffs/BuffComponent.h"
#include "CourtOfSebelkeh/Components/Skills/SkillComponent.h"

void UBuff::Begin(UBuffComponent* InBuffComponent, AActor* InTarget, AActor* InInstigator, int32 InID, float InEndTimeStamp, bool bInIsInfinite)
{
	Target = InTarget;
	BuffComponent = InBuffComponent;
	Instigator = InInstigator;
	ID = InID;
	BeginTimeStamp = GetWorld()->GetTimeSeconds();
	EndTimeStamp = InEndTimeStamp;
	bIsInfinite = bInIsInfinite;
	SkillComponent = InInstigator ? InInstigator->FindComponentByClass<USkillComponent>() : nullptr;

	if (!bTickOnBegin)
	{
		NextTickTimeStamp = GetWorld()->GetTimeSeconds() + TickRate;
	}

	if (Attribute)
	{
		if (SkillComponent)
		{
			AttributeValue = SkillComponent->GetAttributeValue(Attribute);
		}
	}

	const float Duration = InEndTimeStamp - GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	ReceiveBegin(Target, InInstigator, Duration, bInIsInfinite);
}

void UBuff::Tick(float DeltaSeconds)
{
	NextTickTimeStamp = GetWorld()->GetTimeSeconds() + TickRate;
	ReceiveTick(DeltaSeconds);
}

void UBuff::End(EBuffEndReason Reason, const bool bLastOfThisType)
{
	if (GetWorld()->IsServer())
	{
		if (UStatComponent* StatComponent = GetTargetActor()->FindComponentByClass<UStatComponent>())
		{
			//Since Begin has already been called at this point, apply costs now
			for (auto Entry : MaintenanceCosts)
			{
				StatComponent->AddStat(Entry.Key, Entry.Value);
			}
		}

		if (MaintenanceCosts.Num() > 0 && SkillComponent)
		{
			SkillComponent->RemoveMaintainedBuff(this);
		}
	}

	ReceiveEnd(Reason, bLastOfThisType);
}

void UBuff::Activate()
{
	bIsActive = true;

	if (AutoCallbacks.Num() > 0)
	{
		if (UCallbackComponent* CallbackComponent = Cast<UCallbackComponent>(Target->GetComponentByClass(UCallbackComponent::StaticClass())))
		{
			const bool bIsServer = GetWorld()->IsServer();
			for (const FAutoCallback& AutoCallback : AutoCallbacks)
			{
				if ((AutoCallback.bRegisterOnClient && !bIsServer) || (AutoCallback.bRegisterOnServer && bIsServer))
				{
					CallbackComponent->RegisterCallback(AutoCallback.Type, this);
				}
			}
		}
	}

	ReceiveActivate();
}

void UBuff::Deactivate()
{
	bIsActive = false;

	if (AutoCallbacks.Num() > 0)
	{
		if (UCallbackComponent* CallbackComponent = Cast<UCallbackComponent>(Target->GetComponentByClass(UCallbackComponent::StaticClass())))
		{
			const bool bIsServer = GetWorld()->IsServer();
			for (const FAutoCallback& AutoCallback : AutoCallbacks)
			{
				if ((AutoCallback.bRegisterOnClient && !bIsServer) || (AutoCallback.bRegisterOnServer && bIsServer))
				{
					CallbackComponent->UnregisterCallback(AutoCallback.Type, this);
				}
			}
		}
	}

	ReceiveDeactivate();
}

void UBuff::SetMaintenanceCosts(const TMap<EStat, int32>& Costs)
{
	MaintenanceCosts = Costs;

	if (UStatComponent* StatComponent = GetTargetActor()->FindComponentByClass<UStatComponent>())
	{
		//Since Begin has already been called at this point, apply costs now
		for (auto Entry : Costs)
		{
			StatComponent->AddStat(Entry.Key, -Entry.Value);
		}
	}

	if (Costs.Num() > 0 && SkillComponent)
	{
		SkillComponent->AddMaintainedBuff(this);
	}
}

bool UBuff::WantsTick(float WorldTime) const
{
	return bWantsTick && bIsActive && WorldTime >= NextTickTimeStamp;
}

void UBuff::RemoveSelf()
{
	BuffComponent->RemoveBuff(GetID(), nullptr, this);
}

float UBuff::GetRemainingDurationFraction() const
{
	return 1.f - (GetWorld()->GetTimeSeconds() - BeginTimeStamp) / (EndTimeStamp - BeginTimeStamp);
}

UWorld* UBuff::GetWorld() const
{
	return Target != nullptr ? Target->GetWorld() : nullptr;
}
