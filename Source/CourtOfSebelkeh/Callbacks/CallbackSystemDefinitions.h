// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CourtOfSebelkeh/Damage/DamageSystemDefinitions.h"
#include "CourtOfSebelkeh/Stats/StatDefinitions.h"
#include "CallbackSystemDefinitions.generated.h"

class UBuff;
class USkillBase;
class UActorState;

UENUM(BlueprintType)
enum class ECallback : uint8
{
	PreDamageDealt,
	DamageDealt,
	PreDamageReceived,
	DamageReceived,
	EnchantmentAdded,
	EnchantmentRemoved,
	HexAdded,
	HexRemoved,
	StatChanged,
	SkillOnCooldown,
	SkillChannel,
	PreActorStateChanged,
	Count
};

USTRUCT(BlueprintType)
struct FBuffEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBuff* Buff;
};

USTRUCT(BlueprintType)
struct FDamageEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FDamageInfo DamageInfo;
};

USTRUCT(BlueprintType)
struct FDamageDealtEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FDamageInfo DamageInfo;

};

USTRUCT(BlueprintType)
struct FStatEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EStat Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 OldValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 NewValue;
};

USTRUCT(BlueprintType)
struct FCooldownEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USkillBase* Skill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Cooldown;
};

USTRUCT(BlueprintType)
struct FSkillChannelEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USkillBase* Skill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ChannelTime;
};

USTRUCT(BlueprintType)
struct FPreActorStateEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UActorState> StateClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bPreventChange;

};