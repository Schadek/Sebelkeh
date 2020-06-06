// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CourtOfSebelkeh/Skills/SkillSelector.h"
#include "TargetedSkillSelector.generated.h"

UCLASS()
class COURTOFSEBELKEH_API ATargetedSkillSelector : public ASkillSelector
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

	bool CanBeUsedAgainst(AActor* Target) const;

	UPROPERTY(EditDefaultsOnly)
		FText InvalidTargetText;

};
