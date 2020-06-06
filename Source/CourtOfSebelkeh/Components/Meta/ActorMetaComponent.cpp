// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorMetaComponent.h"
#include "Net/UnrealNetwork.h"

UActorMetaComponent::UActorMetaComponent()
{
}

void UActorMetaComponent::SetName(const FText& NewName)
{
	Name = NewName;
	OnNameChanged(NewName);
}

void UActorMetaComponent::SetFaction(uint8 NewFaction)
{
	Faction = NewFaction;
}

void UActorMetaComponent::OnRep_Name()
{
	OnNameChanged(Name);
}

void UActorMetaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActorMetaComponent, Faction);
	DOREPLIFETIME(UActorMetaComponent, Name);
}

void UActorMetaComponent::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);
}

void UActorMetaComponent::OnNameChanged(const FText& NewName)
{
	ReceiveOnNameChanged(NewName);
	OnNameChangedDel.Broadcast(this, NewName);
}