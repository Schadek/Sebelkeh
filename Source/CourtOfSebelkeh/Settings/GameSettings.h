// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CourtOfSebelkeh/ActorStates/ActorStateDefinitions.h"
#include "CourtOfSebelkeh/Skills/SkillDefinitions.h"
#include "CourtOfSebelkeh/Damage/CoreDamageType.h"
#include "CourtOfSebelkeh/Camera/ThirdPersonCamera.h"
#include "GameSettings.generated.h"

class USkillBase;

UENUM(BlueprintType)
enum class EActorState : uint8
{
	Free,
	ChannelingIdle,
	ChannelingMoving,
	Attacking,
	Dodging,
	Downed
};

UCLASS(BlueprintType, Blueprintable)
class COURTOFSEBELKEH_API UGameSettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		uint8 NeutralFaction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		uint8 PassiveFaction = ~0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TMap<ERangePreset, float> RangePresets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<USkillBase> EmptySkillClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UActorState> FreeActorState = UActorState::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UChannelingActorState> IdleChannelingState = UChannelingActorState::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UChannelingActorState> MovableChannelingState = UChannelingActorState::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UActorState> AttackingState = UActorState::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UActorState> DodgingState = UActorState::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UActorState> DownedState = UActorState::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UKnockdownActorState> KnockdownState = UKnockdownActorState::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<USacrificeDamageType> HealthSacrificeDamageType = USacrificeDamageType::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<AThirdPersonCamera> ThirdPersonCameraClass = AThirdPersonCamera::StaticClass();

};
