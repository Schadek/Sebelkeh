// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CourtOfSebelkeh/Meta/Factions/FactionDefinitions.h"
#include "CourtOfSebelkeh/Settings/GameSettings.h"
#include "CourtOfSebelkeh/Settings/UISettings.h"
#include "CourtOfSebelkeh/Skills/SkillDefinitions.h"
#include "CourtOfSebelkeh/Damage/DamageSystemDefinitions.h"
#include "CoreBlueprintLibrary.generated.h"

class UBuff;
class ACoreGameState;

UCLASS()
class COURTOFSEBELKEH_API UCoreBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static ACoreGameState* GetCoreGameState(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure)
		static FLinearColor GetActorFactionColor(AActor* Target);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static FLinearColor GetFactionColorByState(UObject* WorldContextObject, EFactionState State);

	UFUNCTION(BlueprintPure)
		static FLinearColor GetBuffTypeColor(UBuff* Buff);

	UFUNCTION(BlueprintPure)
		static EFactionState GetActorFactionState(AActor* Target);

	UFUNCTION(BlueprintPure)
		static EFactionState GetActorFactionStateRelativeTo(AActor* Origin, AActor* Target);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static UUISettings* GetUISettings(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static UGameSettings* GetGameSettings(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static float GetRangeFromPreset(UObject* WorldContextObject, ERangePreset Preset);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		static TArray<AActor*> FilterActorsByFaction(UObject* WorldContextObject, AActor* Origin, const TArray<AActor*>& Targets, const TArray<ESkillTarget>& PossibleTargets);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		static void DealAreaDamage(UObject* WorldContextObject, AActor* Origin, const TArray<ESkillTarget>& PossibleTargets, const FVector& Location, float Radius, const FDamageInfo& DamageInfo);

};
