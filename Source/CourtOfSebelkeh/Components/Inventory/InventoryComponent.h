// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CourtOfSebelkeh/Inventory/InventoryDefinitions.h"
#include "InventoryComponent.generated.h"

class ACoreCharacter;
class UActorStateComponent;
class UWeapon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COURTOFSEBELKEH_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInventoryComponent();

	UFUNCTION(BlueprintCallable)
		void StopAttacking();

	UFUNCTION(BlueprintCallable)
		bool SetAttackTarget(AActor* Target);

	UFUNCTION(BlueprintCallable)
		void EquipWeaponByDefinition(const FWeaponDefinition& Definition);

	UFUNCTION(BlueprintCallable)
		void EquipWeapon(UWeapon* Weapon);

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	float EvaluateAttackSpeed() const;
	bool IsInAttackState() const;
	bool IsLookingAtTarget() const;

	void TickAttackLogic(float DeltaSeconds);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SetAttackTarget(AActor* Target);
	UFUNCTION(NetMulticast, Reliable)
		void Multicast_StopAttacking(AActor* Target);

	void SetAttackTargetInternal(AActor* Target);

protected:

	UFUNCTION()
		void OnOwnerReachedTarget(ACoreCharacter* Character, AActor* Target, const FVector& Location, float TargetRange);
	UFUNCTION()
		void OnOwnerAbortedWalk(ACoreCharacter* Character, AActor* Target, const FVector& Location, float TargetRange);

	UFUNCTION(NetMulticast, Unreliable)
		void Attack(AActor* Target, UWeapon* Weapon);
	UFUNCTION(NetMulticast, Reliable)
		void EquipWeaponInternal(const FWeaponDefinition& Definition);

	void UnequipWeaponInternal(UWeapon* Weapon);

	UFUNCTION()
		OnRep_EquippedWeaponDefinitions();

	UPROPERTY(Replicated, ReplicatedUsing = "OnRep_EquippedWeaponDefinitions")
		TArray<FWeaponDefinition> EquippedWeaponDefinitions;
	UPROPERTY()
		TArray<UWeapon*> EquippedWeapons;
	UPROPERTY(EditDefaultsOnly)
		float LookAtAngle = 40.f;


	AActor* AttackTarget;
	ACoreCharacter* CharacterOwner;
	UActorStateComponent* ActorStateComponent;

	bool bIsWaitingForAttackCallback;

	float AttackCooldown;
	float CurrentCycleAttackSpeed;

};
