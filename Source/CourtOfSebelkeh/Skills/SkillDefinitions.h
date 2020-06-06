// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillDefinitions.generated.h"

class USkillBase;
class USkillAttribute;
class UBuff;

UENUM(BlueprintType)
enum class ESkillCost : uint8
{
	Energy,
	Adrenaline,
	EnergyRegeneration,
	HealthPercentage,
};

UENUM(BlueprintType)
enum class ERangePreset : uint8
{
	None,
	Adjacent,
	Nearby,
	InTheArea,
	Earshot,
};

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Skill,
	Attack,
	MeleeAttack,
	AxeAttack,
	DaggerAttack,
	LeadAttack,
	OffHandAttack,
	DualAttack,
	HammerAttack,
	ScytheAttack,
	SwordAttack,
	PetAttack,
	RangedAttack,
	BowAttack,
	SpearAttack,
	Chant,
	Echo,
	Form,
	Glyph,
	Preparation,
	Ritual,
	BindingRitual,
	NatureRitual,
	Shout,
	Signet,
	Spell,
	EnchantmentSpell,
	HexSpell,
	ItemSpell,
	WardSpell,
	WeaponSpell,
	WellSpell,
	Stance,
	Trap,
};

UCLASS(Blueprintable, BlueprintType)
class COURTOFSEBELKEH_API USkillAttribute : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		FText Name;

	UPROPERTY(BlueprintReadOnly)
		int32 Value;

};

USTRUCT()
struct FSkillRPCData
{
	GENERATED_BODY();

	UPROPERTY()
		uint8 Index;

	UPROPERTY()
		TSubclassOf<USkillBase> Class;
};

USTRUCT()
struct FSkillCooldownRPCData
{
	GENERATED_BODY();

	UPROPERTY()
		uint8 Index;

	UPROPERTY()
		float EndTimestamp;
};

USTRUCT()
struct FSkillAttributeRPCData
{
	GENERATED_BODY();

	UPROPERTY()
		TSubclassOf<USkillAttribute> Class;

	UPROPERTY()
		int8 Value;

};

USTRUCT()
struct FDefaultAttribute
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
		TSubclassOf<USkillAttribute> Class;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0, ClampMax = 21))
		uint8 Value;

};

USTRUCT(BlueprintType)
struct FMaintainedBuff
{
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly)
		TSubclassOf<UBuff> Class;

	UPROPERTY(BlueprintReadOnly)
		AActor* Target;

	UPROPERTY(BlueprintReadOnly)
		int32 BuffInstanceId;

};