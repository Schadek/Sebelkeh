// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorStateDefinitions.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "CourtOfSebelkeh/Components/State/ActorStateComponent.h"
#include "CourtOfSebelkeh/Components/Skills/SkillComponent.h"
#include "CourtOfSebelkeh/Components/Inventory/InventoryComponent.h"
#include "CourtOfSebelkeh/Characters/CoreCharacter.h"

void UActorState::Begin(UActorStateComponent* Owner, UActorState* OldState)
{
	OwnerComponent = Owner;

	ReceiveBegin(OwnerComponent, OldState);
}

void UActorState::Tick(float DeltaSeconds)
{
	ReceiveTick(DeltaSeconds);
}

void UActorState::End(UActorState* NewState)
{
	ReceiveEnd(NewState);
}

UWorld* UActorState::GetWorld() const
{
	return OwnerComponent ? OwnerComponent->GetWorld() : nullptr;
}

//////////////////////////////////////////////////////////////////////////
void UChannelingActorState::EnableEndOnMove()
{
	if (Character)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, "OnMovement");
		Character->OnMovementInput.Add(Delegate);
	}
}

void UChannelingActorState::Begin(UActorStateComponent* Owner, UActorState* OldState)
{
	Super::Begin(Owner, OldState);

	Character = Cast<ACoreCharacter>(OwnerComponent->GetOwner());
	SkillComponent = OwnerComponent->GetOwner()->FindComponentByClass<USkillComponent>();
	BeginTimestamp = GetWorld()->GetTimeSeconds();
	bHasAuthority = GetWorld()->IsServer();

	if (bEndOnMove)
	{
		EnableEndOnMove();
	}

	if (Character)
	{
		Character->StopAutoWalk(false);
	}
}

void UChannelingActorState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bHasAuthority && GetWorld()->GetTimeSeconds() > EndTimestamp)
	{
		SkillComponent->NotifySkillChannelEnd(ChanneledSkill, EndTimestamp - BeginTimestamp, true);
	}
}

void UChannelingActorState::End(UActorState* NewState)
{
	Super::End(NewState);

	if (Character)
	{
		Character->OnMovement.RemoveAll(this);
	}
}

void UChannelingActorState::OnMovement(ACharacter* Sender)
{
	if (!bNotifiedEndAlready)
	{
		SkillComponent->CancelChanneling();
		bNotifiedEndAlready = true;
	}
}

//////////////////////////////////////////////////////////////////////////
void UKnockdownActorState::Begin(UActorStateComponent* Owner, UActorState* OldState)
{
	Super::Begin(Owner, OldState);

	BeginTimestamp = GetWorld()->GetTimeSeconds();

	if (APawn* PawnOwner = Cast<APawn>(OwnerComponent->GetOwner()))
	{
		if (AController* Controller = PawnOwner->GetController())
		{
			Controller->SetIgnoreMoveInput(true);
		}
	}
}

void UKnockdownActorState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetWorld()->IsServer() && GetWorld()->GetTimeSeconds() >= EndTimestamp)
	{
		OwnerComponent->SetState(nullptr);
	}
}

void UKnockdownActorState::End(UActorState* NewState)
{
	Super::End(NewState);

	if (APawn* PawnOwner = Cast<APawn>(OwnerComponent->GetOwner()))
	{
		if (AController* Controller = PawnOwner->GetController())
		{
			Controller->SetIgnoreMoveInput(false);
		}
	}
}

void UAttackingActorState::End(UActorState* NewState)
{
	Super::End(NewState);

	if (GetWorld()->IsServer())
	{
		if (UInventoryComponent* InventoryComponent = OwnerComponent->GetOwner()->FindComponentByClass<UInventoryComponent>())
		{
			InventoryComponent->StopAttacking();
		}
	}
}
