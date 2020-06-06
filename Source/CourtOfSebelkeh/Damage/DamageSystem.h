// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CourtOfSebelkeh/Damage/DamageSystemDefinitions.h"
#include "DamageSystem.generated.h"


/**
 *
 */
UCLASS(BlueprintType)
class COURTOFSEBELKEH_API UDamageSystem : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		void ProcessDamage(const FDamageInfo& Info);

	UFUNCTION(BlueprintCallable)
		int32 CalculateDamage(const FDamageInfo& Info) const;

};
