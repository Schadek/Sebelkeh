// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillSelector.h"
#include "Components/InputComponent.h"
#include "CourtOfSebelkeh/Skills/SkillBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "CourtOfSebelkeh/Components/Skills/SkillComponent.h"

ASkillSelector::ASkillSelector()
{
	PrimaryActorTick.bCanEverTick = true;

	InputComponent = CreateDefaultSubobject<UInputComponent>(TEXT("Input"));
}

void ASkillSelector::BeginPlay()
{
	Super::BeginPlay();

	InitializeInputComponent();
}

void ASkillSelector::InitializeInputComponent()
{
	if (APawn* Pawn = Cast<APawn>(OwnerSkill->GetSkillComponent()->GetOwner()))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			EnableInput(PlayerController);
		}
	}
	InputComponent->BindAction("SkillApply", IE_Pressed, this, &ASkillSelector::MouseDown);
	InputComponent->BindAction("SkillApply", IE_Released, this, &ASkillSelector::MouseUp);
}

void ASkillSelector::MouseDown()
{

}

void ASkillSelector::MouseUp()
{

}

void ASkillSelector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

void ASkillSelector::CancelSelection()
{
	Destroy();
}