// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CourtOfSebelkeh/Skills/SkillBase.h"
#include "BuffSkill.generated.h"

class UBuff;

/**
 *
 */
UCLASS()
class COURTOFSEBELKEH_API UBuffSkill : public USkillBase
{
	GENERATED_BODY()

protected:

	virtual void UseTargetAuthority(AActor* Target) override;
	virtual void PayCostSpecific(UStatComponent* StatComponent, ESkillCost CostType, int32 Amount) override;

	UFUNCTION(BlueprintNativeEvent)
		float GetBuffDuration() const;

	UFUNCTION(BlueprintNativeEvent)
		void OnBuffCreated(UBuff* Buff);

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UBuff> BuffClass;

	UPROPERTY(EditDefaultsOnly)
		bool bUseFixedBuffDuration;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "bUseFixedBuffDuration"))
		float FixedDuration = -1.f;

	TMap<EStat, int32> MaintenanceCosts;

};
