// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageSystem.h"
#include "CourtOfSebelkeh/Components/Stats/StatComponent.h"
#include "CourtOfSebelkeh/Components/CallbackComponent.h"

void UDamageSystem::ProcessDamage(const FDamageInfo& Info)
{
	if (Info.Target)
	{
		if (UStatComponent* StatComponent = Cast<UStatComponent>(Info.Target->GetComponentByClass(UStatComponent::StaticClass())))
		{
			int32 CurrentHealth = 0;
			if (!StatComponent->GetStatRaw(EStat::Health, CurrentHealth))
			{
				return;
			}

			FDamageEventInfo Event;
			Event.DamageInfo = Info;

			UCallbackComponent* InstigatorCallback = Info.Instigator ? Cast<UCallbackComponent>(Info.Instigator->GetComponentByClass(UCallbackComponent::StaticClass())) : nullptr;
			UCallbackComponent* TargetCallback = Cast<UCallbackComponent>(Info.Target->GetComponentByClass(UCallbackComponent::StaticClass()));

			if (InstigatorCallback)
			{
				InstigatorCallback->BroadcastPreDamageDealt(Event);
			}

			if (TargetCallback)
			{
				TargetCallback->BroadcastPreDamageReceived(Event);
			}

			if (Event.DamageInfo.Amount > 0)
			{
				StatComponent->SetStat(EStat::Health, FMath::Max(0, CurrentHealth - Event.DamageInfo.Amount));
			}

			if (APawn* Pawn = Cast<APawn>(Info.Instigator))
			{
				if (!Pawn->IsLocallyControlled())
				{
					if (UStatComponent* InstigatorStatComponent = Cast<UStatComponent>(Info.Instigator->FindComponentByClass<UStatComponent>()))
					{
						InstigatorStatComponent->OnDamageDealt(Event.DamageInfo);
					}
				}
			}

			if (InstigatorCallback)
			{
				FDamageDealtEventInfo DealtInfo;
				DealtInfo.DamageInfo = Event.DamageInfo;
				InstigatorCallback->BroadcastDamageDealt(DealtInfo);
			}

			if (TargetCallback)
			{
				TargetCallback->BroadcastDamageReceived(Event);
			}
		}
	}
}

int32 UDamageSystem::CalculateDamage(const FDamageInfo& Info) const
{
	return Info.Amount;
}
