// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CourtOfSebelkeh/Meta/Factions/FactionDefinitions.h"
#include "UISettings.generated.h"

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class COURTOFSEBELKEH_API UUISettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factions")
		FLinearColor AllyFactionColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factions")
		FLinearColor EnemyFactionColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factions")
		FLinearColor NeutralFactionColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factions")
		FLinearColor PassiveFactionColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factions")
		TMap<TEnumAsByte<ESkillTarget>, uint8> FactionStateToOutlineIndexMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buffs")
		FLinearColor BaseBuffColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buffs")
		FLinearColor EnchantmentColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buffs")
		FLinearColor HexColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buffs")
		FLinearColor ConditionColor;

};
