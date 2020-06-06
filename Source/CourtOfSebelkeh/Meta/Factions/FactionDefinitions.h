// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FactionDefinitions.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class EFactionState : uint8
{
	Ally,
	Enemy,
	Neutral,
	Passive
};

ENUM_CLASS_FLAGS(EFactionState)

UENUM(BlueprintType, meta = (Bitflags))
enum class ESkillTarget : uint8
{
	Ally,
	Enemy,
	Neutral,
	Passive,
	Self
};

ENUM_CLASS_FLAGS(ESkillTarget)