// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StatDefinitions.generated.h"

UENUM(BlueprintType)
enum class EStat : uint8
{
	Health = 0,
	HealthRegeneration,
	MaxHealth,
	Energy,
	EnergyRegeneration,
	MaxEnergy,
	Adrenaline,
	Armor,
	Stamina,
	ArmorPenetration,
	Experience,
	ExperienceToLevelUp,
	MovementSpeed,
	AttackSpeed,
	Overcast
};

USTRUCT()
struct FStatInfo
{
	GENERATED_BODY();

	/** Type of this stat slot */
	UPROPERTY(EditDefaultsOnly)
		EStat Type;
	/** Initial value of this stat slot, also runtime value */
	UPROPERTY(EditDefaultsOnly)
		uint16 Value;
	/** Whether this slot is replicated to clients or server-only */
	UPROPERTY(EditDefaultsOnly)
		bool bServerOnly = true;
	/** If this stat is updated only occassionally, enable this to save bandwidth */
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "ServerOnly"))
		bool bUpdatedRarely = false;

	int32 ReplicationIndex;
};