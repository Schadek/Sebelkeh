// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "SkillSelector.generated.h"

class USkillBase;

UCLASS(Blueprintable, BlueprintType)
class COURTOFSEBELKEH_API ASkillSelector : public AActor
{
	GENERATED_BODY()

public:

	void SetSkill(USkillBase* Skill) { OwnerSkill = Skill; }
	USkillBase* GetSkill() const { return OwnerSkill; }

	virtual	void CancelSelection();

	ASkillSelector();

protected:

	virtual void BeginPlay() override;
	virtual void InitializeInputComponent();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
		virtual void MouseDown();
	UFUNCTION()
		virtual void MouseUp();

	/** The skill this indicator is bound to */
	UPROPERTY(BlueprintReadOnly, Category = "Selector")
		USkillBase* OwnerSkill;
};
