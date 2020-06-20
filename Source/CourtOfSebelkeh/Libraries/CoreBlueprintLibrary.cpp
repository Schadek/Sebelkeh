// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreBlueprintLibrary.h"
#include "CourtOfSebelkeh/GameStates/CoreGameState.h"
#include "CourtOfSebelkeh/Settings/UISettings.h"
#include "CourtOfSebelkeh/Settings/GameSettings.h"
#include "CourtOfSebelkeh/Components/Meta/ActorMetaComponent.h"
#include "CourtOfSebelkeh/Damage/DamageSystem.h"
#include "CourtOfSebelkeh/Buffs/BuffSystemDefinitions.h"
#include "CourtOfSebelkeh/Skills/Utility/AreaOfEffectActor.h"
#include "CourtOfSebelkeh/Projectiles/Projectile.h"
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
			ESkillTarget State = GetActorFactionState(Target);
			switch (State)
			{
			case ESkillTarget::Ally:
			case ESkillTarget::Self:
				return UISettings->AllyFactionColor;
			case ESkillTarget::Enemy:
				return UISettings->EnemyFactionColor;
			case ESkillTarget::Neutral:
				return UISettings->NeutralFactionColor;
			case ESkillTarget::Passive:
				return UISettings->PassiveFactionColor;
			}
		}
	}

	//Magenta to make things obviously ugly
	return FLinearColor(1.f, 0, 1.f);
}

FLinearColor UCoreBlueprintLibrary::GetFactionColorByState(UObject* WorldContextObject, ESkillTarget State)
{
	if (WorldContextObject)
	{
		if (UUISettings* UISettings = Cast<ACoreGameState>(WorldContextObject->GetWorld()->GetGameState())->GetUISettings())
		{
			switch (State)
			{
			case ESkillTarget::Ally:
			case ESkillTarget::Self:
				return UISettings->AllyFactionColor;
			case ESkillTarget::Enemy:
				return UISettings->EnemyFactionColor;
			case ESkillTarget::Neutral:
				return UISettings->NeutralFactionColor;
			case ESkillTarget::Passive:
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

ESkillTarget UCoreBlueprintLibrary::GetActorFactionState(AActor* Target)
{
	return GetActorFactionStateRelativeTo(UGameplayStatics::GetPlayerPawn(Target, 0), Target);
}

ESkillTarget UCoreBlueprintLibrary::GetActorFactionStateRelativeTo(AActor* Origin, AActor* Target)
{
	if (Target && Origin)
	{
		if (Origin == Target)
		{
			return ESkillTarget::Self;
		}

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
						return ESkillTarget::Ally;
					}
					else if (OtherFaction == GameSettings->PassiveFaction)
					{
						return ESkillTarget::Passive;
					}
					else if (OtherFaction == GameSettings->NeutralFaction)
					{
						return ESkillTarget::Neutral;
					}
					else
					{
						return ESkillTarget::Enemy;
					}
				}

				return ESkillTarget::Neutral;
			}
		}
	}

	//Magenta to make things obviously ugly
	return ESkillTarget::Neutral;
}

UUISettings* UCoreBlueprintLibrary::GetUISettings(const UObject* WorldContextObject)
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

UGameSettings* UCoreBlueprintLibrary::GetGameSettings(const UObject* WorldContextObject)
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

float UCoreBlueprintLibrary::GetRangeFromPreset(const UObject* WorldContextObject, ERangePreset Preset)
{
	if (Preset != ERangePreset::None)
	{
		return GetGameSettings(WorldContextObject)->RangePresets[Preset];
	}

	return 0.0f;
}

TArray<AActor*> UCoreBlueprintLibrary::FilterActorsByFaction(UObject* WorldContextObject, AActor* Origin, const TArray<AActor*>& Targets, int32 PossibleTargestMask)
{
	TArray<AActor*> FilteredActors;

	bool bIncludeSelf = (Origin && (1 << ESkillTarget::Ally)) != 0;

	for (AActor* Target : Targets)
	{
		if (bIncludeSelf && Target == Origin)
		{
			FilteredActors.Add(Origin);
			continue;
		}

		ESkillTarget FactionState = static_cast<ESkillTarget>(UCoreBlueprintLibrary::GetActorFactionStateRelativeTo(Origin, Target));
		if ((PossibleTargestMask & (1 << FactionState)) != 0)
		{
			FilteredActors.Add(Target);
		}
	}

	return FilteredActors;
}

void UCoreBlueprintLibrary::DealAreaDamage(UObject* WorldContextObject, AActor* Origin, int32 PossibleTargetsMask, const FVector& Location, float Radius, const TArray<AActor*>& ActorsToIgnore, const FDamageInfo& DamageInfo)
{
	ACoreGameState* GameState = Cast<ACoreGameState>(WorldContextObject->GetWorld()->GetGameState());
	if (!GameState)
		return;

	UDamageSystem* DamageSystem = GameState->GetDamageSystem();
	if (!DamageSystem)
		return;

	TArray<AActor*> Targets;
	GetValidTargetsInArea(WorldContextObject, Origin, PossibleTargetsMask, Location, Radius, ActorsToIgnore, Targets);

	FDamageInfo Info = DamageInfo;
	for (AActor* Target : Targets)
	{
		Info.Target = Target;
		DamageSystem->ProcessDamage(Info);
	}
}

AAreaOfEffectActor* UCoreBlueprintLibrary::SpawnAreaActor(UObject* WorldContextObject, const FTransform& Transform, const int32 PossibleTargetsMask, AActor* Owner, float Radius, float TickInterval /*= -1.f*/)
{
	if (AAreaOfEffectActor* AOEActor = WorldContextObject->GetWorld()->SpawnActorDeferred<AAreaOfEffectActor>(GetGameSettings(WorldContextObject)->AreaOfEffectActorClass, Transform))
	{
		AOEActor->SetTimeInterval(TickInterval);
		AOEActor->SetValidTargets(PossibleTargetsMask);
		AOEActor->SetOwner(Owner);
		AOEActor->SetShape(Radius);
		UGameplayStatics::FinishSpawningActor(AOEActor, Transform);

		return AOEActor;
	}

	return nullptr;
}

void UCoreBlueprintLibrary::GetValidTargetsInArea(UObject* WorldContextObject, AActor* Origin, int32 PossibleTargetsMask, const FVector& Location, float Radius, const TArray<AActor*>& ActorsToIgnore, TArray<AActor*>& OutActors)
{
	if (!WorldContextObject)
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
		AActor* Actor = Result.GetActor();
		if (!ActorsToIgnore.Contains(Actor))
			Targets.AddUnique(Actor);
	}

	OutActors = FilterActorsByFaction(WorldContextObject, Origin, Targets, PossibleTargetsMask);
}

void UCoreBlueprintLibrary::GetAllPropertyNames(UObject* Object, TArray<FName>& OutNames)
{
	if (!Object)
		return;


	for (TFieldIterator<FProperty> Prop(Object->GetClass()); Prop; ++Prop)
	{
		const FName Name = Prop->GetFName();
		OutNames.Add(Name);
	}
}

void UCoreBlueprintLibrary::ExpandSkillTargetMask(int32 PossibleTargestMask, TArray<TEnumAsByte<ESkillTarget>>& OutTargets)
{
	if ((PossibleTargestMask & (1 << static_cast<uint8>(ESkillTarget::Ally))) != 0)
	{
		OutTargets.Add(ESkillTarget::Ally);
	}
	if ((PossibleTargestMask & (1 << static_cast<uint8>(ESkillTarget::Enemy))) != 0)
	{
		OutTargets.Add(ESkillTarget::Enemy);
	}
	if ((PossibleTargestMask & (1 << static_cast<uint8>(ESkillTarget::Passive))) != 0)
	{
		OutTargets.Add(ESkillTarget::Passive);
	}
	if ((PossibleTargestMask & (1 << static_cast<uint8>(ESkillTarget::Neutral))) != 0)
	{
		OutTargets.Add(ESkillTarget::Neutral);
	}
	if ((PossibleTargestMask & (1 << static_cast<uint8>(ESkillTarget::Self))) != 0)
	{
		OutTargets.Add(ESkillTarget::Self);
	}
}

AProjectile* UCoreBlueprintLibrary::SpawnProjectile(UObject* WorldContextObject, TSubclassOf<AProjectile> Class, AActor* Instigator, UObject* Owner, const FVector& Location)
{
	return AProjectile::SpawnProjectile(Class, Instigator, Owner, Location);
}
