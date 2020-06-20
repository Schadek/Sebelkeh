// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CourtOfSebelkeh/Stats/StatDefinitions.h"
#include "CourtOfSebelkeh/Damage/DamageSystemDefinitions.h"
#include "StatComponent.generated.h"

class UCoreDamageType;
class UCallbackComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COURTOFSEBELKEH_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStatComponent();

	UFUNCTION(BlueprintCallable)
		int32 Heal(int32 Value, AActor* Healer, UObject* Source);

	UFUNCTION(BlueprintCallable)
		bool Resurrect(int32 Health, int32 Energy, AActor* Instigator);

	UFUNCTION(BlueprintCallable)
		void AddStat(EStat Type, int32 Value);

	UFUNCTION(BlueprintCallable)
		void SetStat(EStat Type, int32 Value);

	UFUNCTION(BlueprintCallable)
		bool GetStatRaw(EStat Type, int32& OutStat) const;

	UFUNCTION(BlueprintCallable)
		int32 GetStatRawNoCheck(EStat Type) const;

	UFUNCTION(BlueprintCallable)
		bool GetStatReal(EStat Type, int32& OutStat) const;

	UFUNCTION(BlueprintCallable)
		int32 GetStatRealNoCheck(EStat Type) const;

	UFUNCTION(Client, Unreliable)
		void OnDamageDealt(const FDamageInfo& Info);

protected:

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	void InitializeReplication();
	void UpdateReplicatedValuesFromLocalOnes();
	void UpdateLocalValuesFromReplicatedOnes();
	int32 GetStatIndex(EStat Type) const;

	void ConvertDefaultValues();
	void InitializeHealth();
	void InitializeEnergy();

	void SetStatInternal(EStat Type, uint16 Value);

	int32 ConvertRealToRawStat(EStat Type, int32 RealValue) const;
	int32 ConvertRawToRealStat(EStat Type, int32 RawValue) const;
	void ClampStats(EStat Type, uint16& InOutValue) const;

	UFUNCTION()
		void OnRep_ReplicatedValues();

	UPROPERTY()
		UCallbackComponent* Callback;

	UPROPERTY(EditDefaultsOnly)
		TArray<FStatInfo> Stats;

	UPROPERTY(Replicated, ReplicatedUsing = "OnRep_ReplicatedValues")
		TArray<uint16> ReplicatedValues;

	bool IsReplicationInitialized;
	TArray<int32> RepValuesToLocalMap;
	TArray<int32> LocalValuesToRepMap;

	float HealthRegenerationOverflow;
	float EnergyRegenerationOverflow;

	int32 HealthIndex;
	int32 MaxHealthIndex;
	int32 HealthRegenerationIndex;

	int32 EnergyIndex;
	int32 MaxEnergyIndex;
	int32 EnergyRegenerationIndex;
};
