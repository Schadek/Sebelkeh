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
class AAreaOfEffectActor;

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
		static FLinearColor GetFactionColorByState(UObject* WorldContextObject, ESkillTarget State);

	UFUNCTION(BlueprintPure)
		static FLinearColor GetBuffTypeColor(UBuff* Buff);

	UFUNCTION(BlueprintPure)
		static ESkillTarget GetActorFactionState(AActor* Target);

	UFUNCTION(BlueprintPure)
		static ESkillTarget GetActorFactionStateRelativeTo(AActor* Origin, AActor* Target);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static UUISettings* GetUISettings(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static UGameSettings* GetGameSettings(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
		static float GetRangeFromPreset(const UObject* WorldContextObject, ERangePreset Preset);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		static TArray<AActor*> FilterActorsByFaction(UObject* WorldContextObject, AActor* Origin, const TArray<AActor*>& Targets, int32 PossibleTargetsMask);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
		static void DealAreaDamage(UObject* WorldContextObject, AActor* Origin, int32 PossibleTargetsMask, const FVector& Location, float Radius, const TArray<AActor*>& ActorsToIgnore, const FDamageInfo& DamageInfo);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		static AAreaOfEffectActor* SpawnAreaActor(UObject* WorldContextObject, const FTransform& Transform, int32 PossibleTargetsMask, AActor* Owner, float Radius, float TickInterval = -1.f);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
		static void GetValidTargetsInArea(UObject* WorldContextObject, AActor* Origin, int32 PossibleTargetsMask, const FVector& Location, float Radius, const TArray<AActor*>& ActorsToIgnore, TArray<AActor*>& OutActors);

	UFUNCTION(BlueprintCallable)
		static void GetAllPropertyNames(UObject* Object, TArray<FName>& OutNames);

	UFUNCTION(BlueprintCallable)
		static void ExpandSkillTargetMask(int32 Bitmask, TArray<TEnumAsByte<ESkillTarget>>& OutTargets);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
		static AProjectile* SpawnProjectile(UObject* WorldContextObject, TSubclassOf<AProjectile> Class, AActor* Instigator, UObject* Owner, const FVector& Location);

};
