// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CourtOfSebelkeh/Damage/CoreDamageType.h"
#include "InventoryDefinitions.generated.h"

class AProjectile;

/**
 *
 */
USTRUCT(BlueprintType)
struct FWeaponDefinition
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MinimumDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaximumDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FireRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bLaunchesProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UCoreDamageType> DamageClass = UCoreDamageType::StaticClass();

};

UCLASS(BlueprintType)
class UWeapon : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		const FWeaponDefinition& GetDefinition() const { return Definition; }

	UFUNCTION(BlueprintCallable)
		void SetDefinition(const FWeaponDefinition& NewDefinition) { Definition = NewDefinition; }

protected:

	FWeaponDefinition Definition;

};