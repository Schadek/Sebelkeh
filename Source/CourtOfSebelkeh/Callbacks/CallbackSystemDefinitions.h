// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CourtOfSebelkeh/Damage/DamageSystemDefinitions.h"
#include "CourtOfSebelkeh/Stats/StatDefinitions.h"
#include "CourtOfSebelkeh/Skills/SkillDefinitions.h"
#include "CallbackSystemDefinitions.generated.h"

class UBuff;
class USkillBase;
class UActorState;

UENUM(BlueprintType)
enum class ECallback : uint8
{
	CostCalculation,
	PreDamageDealt,
	DamageDealt,
	PreDamageReceived,
	DamageReceived,
	PreHealReceived,
	HealReceived,
	PreHealApplied,
	HealApplied,
	PreBuffApplied,
	PreBuffReceived,
	BuffAdded,
	BuffRemoved,
	StatChanged,
	SkillOnCooldown,
	SkillChannel,
	PreSkillUsed,
	SkillUsed,
	PreActorStateChanged,
	ActorStateChanged,
	Count
};

USTRUCT(BlueprintType)
struct FCostCalculationEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ESkillCost Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USkillBase* Skill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Amount;

};

USTRUCT(BlueprintType)
struct FPreBuffReceivedEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBuff* Buff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UObject* Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Duration;

};

USTRUCT(BlueprintType)
struct FPreBuffAppliedEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBuff* Buff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UObject* Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Duration;

};

USTRUCT(BlueprintType)
struct FBuffEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBuff* Buff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UObject* Source;

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
struct FSkillUsedEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USkillBase* Skill;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Caster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector TargetLocation;

};

USTRUCT(BlueprintType)
struct FPreSkillUsedEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSkillUsedEventInfo SkillUseEventInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bCancel = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText CancelReasonText;

};

USTRUCT(BlueprintType)
struct FPreDamageHealedEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Healer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UObject* Source;

};

USTRUCT(BlueprintType)
struct FDamageHealedEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Healer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UObject* Source;

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
		bool bPreventChange = false;

};

USTRUCT(BlueprintType)
struct FActorStateEventInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UActorState> NewStateClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* Instigator;

};