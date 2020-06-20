// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "CoreDamageType.generated.h"

/**
 *
 */
UCLASS()
class COURTOFSEBELKEH_API UCoreDamageType : public UDamageType
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FText DisplayName;

};

UCLASS()
class COURTOFSEBELKEH_API USacrificeDamageType : public UCoreDamageType
{
	GENERATED_BODY()

};