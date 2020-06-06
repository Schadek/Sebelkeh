// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DamageSystemDefinitions.generated.h"

class UCoreDamageType;

USTRUCT(BlueprintType)
struct COURTOFSEBELKEH_API FDamageInfo
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
		int32 Amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
		TSubclassOf<UCoreDamageType> Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
		AActor* Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
		AActor* Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DamageInfo")
		UObject* Source;

};
