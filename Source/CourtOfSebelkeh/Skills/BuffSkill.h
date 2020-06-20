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

public:

	UBuffSkill();

protected:

	UFUNCTION()
		FString ParseDuration(const FString& Keyword) const;

	virtual void Begin(USkillComponent* InOwner, bool bAutoEquip) override;
	virtual void UseTargetAuthority(AActor* Target) override;
	virtual void PayCostSpecific(UStatComponent* StatComponent, ESkillCost CostType, int32& Amount) override;

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

	UPROPERTY(EditDefaultsOnly)
		EBuffSkillTargetLogic TargetLogic;

	UPROPERTY(EditDefaultsOnly)
		ERangePreset BuffRangePreset;

	UPROPERTY(EditDefaultsOnly)
		float BuffRange;

	TMap<EStat, int32> MaintenanceCosts;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

};
