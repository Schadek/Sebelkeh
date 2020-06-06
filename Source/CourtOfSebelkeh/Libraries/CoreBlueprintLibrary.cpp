// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreBlueprintLibrary.h"
#include "CourtOfSebelkeh/GameStates/CoreGameState.h"
#include "CourtOfSebelkeh/Settings/UISettings.h"
#include "CourtOfSebelkeh/Settings/GameSettings.h"
#include "CourtOfSebelkeh/Components/Meta/ActorMetaComponent.h"
#include "CourtOfSebelkeh/Damage/DamageSystem.h"
#include "CourtOfSebelkeh/Buffs/BuffSystemDefinitions.h"
#include "Runtime/Engine/Public/WorldCollision.h"
#include "Kismet/GameplayStatics.h"

ACoreGameState* UCoreBlueprintLibrary::GetCoreGameState(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		return Cast<ACoreGameState>(WorldContextObject->GetWorld()->GetGameState());
	}

	return nullptr;
}

FLinearColor UCoreBlueprintLibrary::GetActorFactionColor(AActor* Target)
{
	if (Target)
	{
		if (UUISettings* UISettings = Cast<ACoreGameState>(Target->GetWorld()->GetGameState())->GetUISettings())
		{
			EFactionState State = GetActorFactionState(Target);
			switch (State)
			{
			case EFactionState::Ally:
				return UISettings->AllyFactionColor;
			case EFactionState::Enemy:
				return UISettings->EnemyFactionColor;
			case EFactionState::Neutral:
				return UISettings->NeutralFactionColor;
			case EFactionState::Passive:
				return UISettings->PassiveFactionColor;
			}
		}
	}

	//Magenta to make things obviously ugly
	return FLinearColor(1.f, 0, 1.f);
}

FLinearColor UCoreBlueprintLibrary::GetFactionColorByState(UObject* WorldContextObject, EFactionState State)
{
	if (WorldContextObject)
	{
		if (UUISettings* UISettings = Cast<ACoreGameState>(WorldContextObject->GetWorld()->GetGameState())->GetUISettings())
		{
			switch (State)
			{
			case EFactionState::Ally:
				return UISettings->AllyFactionColor;
			case EFactionState::Enemy:
				return UISettings->EnemyFactionColor;
			case EFactionState::Neutral:
				return UISettings->NeutralFactionColor;
			case EFactionState::Passive:
				return UISettings->PassiveFactionColor;
			}
		}
	}

	//Magenta to make things obviously ugly
	return FLinearColor(1.f, 0, 1.f);
}

FLinearColor UCoreBlueprintLibrary::GetBuffTypeColor(UBuff* Buff)
{
	if (UUISettings* Settings = GetUISettings(Buff))
	{
		switch (Buff->GetType())
		{
		case EBuffType::Enchantment:
			return Settings->EnchantmentColor;
		case EBuffType::Hex:
			return Settings->HexColor;
		default:
			return Settings->BaseBuffColor;
		}
	}

	return FLinearColor::Red;
}

EFactionState UCoreBlueprintLibrary::GetActorFactionState(AActor* Target)
{
	return GetActorFactionStateRelativeTo(UGameplayStatics::GetPlayerPawn(Target, 0), Target);
}

EFactionState UCoreBlueprintLibrary::GetActorFactionStateRelativeTo(AActor* Origin, AActor* Target)
{
	if (Target && Origin)
	{
		if (UGameSettings* GameSettings = Cast<ACoreGameState>(Target->GetWorld()->GetGameState())->GetGameSettings())
		{
			if (UActorMetaComponent* MetaComponent = Cast<UActorMetaComponent>(Target->GetComponentByClass(UActorMetaComponent::StaticClass())))
			{
				if (UActorMetaComponent* OwnMetaComponent = Cast<UActorMetaComponent>(Origin->GetComponentByClass(UActorMetaComponent::StaticClass())))
				{
					const uint8 OtherFaction = MetaComponent->GetFaction();
					const uint8 OwnFaction = OwnMetaComponent->GetFaction();
					if (OwnFaction == OtherFaction)
					{
						return EFactionState::Ally;
					}
					else if (OtherFaction == GameSettings->PassiveFaction)
					{
						return EFactionState::Passive;
					}
					else if (OtherFaction == GameSettings->NeutralFaction)
					{
						return EFactionState::Neutral;
					}
					else
					{
						return EFactionState::Enemy;
					}
				}

				return EFactionState::Neutral;
			}
		}
	}

	//Magenta to make things obviously ugly
	return EFactionState::Neutral;
}

UUISettings* UCoreBlueprintLibrary::GetUISettings(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UUISettings* UISettings = Cast<ACoreGameState>(WorldContextObject->GetWorld()->GetGameState())->GetUISettings())
		{
			return UISettings;
		}
	}

	return nullptr;
}

UGameSettings* UCoreBlueprintLibrary::GetGameSettings(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UGameSettings* GameSettings = Cast<ACoreGameState>(WorldContextObject->GetWorld()->GetGameState())->GetGameSettings())
		{
			return GameSettings;
		}
	}

	return nullptr;
}

float UCoreBlueprintLibrary::GetRangeFromPreset(UObject* WorldContextObject, ERangePreset Preset)
{
	if (Preset != ERangePreset::None)
	{
		return GetGameSettings(WorldContextObject)->RangePresets[Preset];
	}

	return 0.0f;
}

TArray<AActor*> UCoreBlueprintLibrary::FilterActorsByFaction(UObject* WorldContextObject, AActor* Origin, const TArray<AActor*>& Targets, const TArray<ESkillTarget>& PossibleTargets)
{
	TArray<AActor*> FilteredActors;

	bool bIncludeSelf = Origin && PossibleTargets.Contains(ESkillTarget::Self);

	for (AActor* Target : Targets)
	{
		if (bIncludeSelf && Target == Origin)
		{
			FilteredActors.Add(Origin);
			continue;
		}

		ESkillTarget FactionState = static_cast<ESkillTarget>(UCoreBlueprintLibrary::GetActorFactionStateRelativeTo(Origin, Target));
		if (PossibleTargets.Contains(FactionState))
		{
			FilteredActors.Add(Target);
		}
	}

	return FilteredActors;
}

void UCoreBlueprintLibrary::DealAreaDamage(UObject* WorldContextObject, AActor* Origin, const TArray<ESkillTarget>& PossibleTargets, const FVector& Location, float Radius, const FDamageInfo& DamageInfo)
{
	if (!WorldContextObject)
		return;

	ACoreGameState* GameState = Cast<ACoreGameState>(WorldContextObject->GetWorld()->GetGameState());
	if (!GameState)
		return;

	TArray<FOverlapResult> Results;
	FCollisionObjectQueryParams Params;
	Params.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape Shape;
	Shape.SetSphere(Radius);

	WorldContextObject->GetWorld()->OverlapMultiByObjectType(Results, Location, FQuat::Identity, Params, Shape);

	TArray<AActor*> Targets;
	for (const FOverlapResult& Result : Results)
	{
		Targets.Add(Result.GetActor());
	}

	UDamageSystem* DamageSystem = GameState->GetDamageSystem();
	TArray<AActor*> FilteredTargets = FilterActorsByFaction(WorldContextObject, Origin, Targets, PossibleTargets);
	FDamageInfo Info = DamageInfo;
	for (AActor* Target : FilteredTargets)
	{
		Info.Target = Target;
		DamageSystem->ProcessDamage(Info);
	}
}
