// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorStateComponent.h"
#include "CourtOfSebelkeh/ActorStates/ActorStateDefinitions.h"
#include "CourtOfSebelkeh/Components/CallbackComponent.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"

UActorStateComponent::UActorStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

UActorState* UActorStateComponent::SetState(TSubclassOf<UActorState> NewState, AActor* Instigator)
{
	if (!GetWorld()->IsServer())
	{
		return nullptr;
	}

	if (!NewState)
	{
		NewState = UCoreBlueprintLibrary::GetGameSettings(this)->FreeActorState;
	}

	if (CallbackComponent)
	{
		FPreActorStateEventInfo Info;
		Info.StateClass = NewState;
		Info.Instigator = Instigator;
		Info.bPreventChange = false;

		CallbackComponent->BroadcastPreActorStateChange(Info);
		if (Info.bPreventChange)
			return nullptr;
	}

	return SetStateInternal(NewState);
}

UActorState* UActorStateComponent::SetStateInternal(TSubclassOf<UActorState> NewStateClass)
{
	UActorState* NewState = NewObject<UActorState>(this, NewStateClass);
	UActorState* OldState = State;

	if (State)
	{
		State->End(NewState);
	}

	State = NewState;
	NewState->Begin(this, OldState);

	OnStateChanged.Broadcast(this, OldState, NewState);
	ReplicatedStateClass = NewStateClass;

	return NewState;
}

void UActorStateComponent::BeginPlay()
{
	Super::BeginPlay();

	CallbackComponent = GetOwner()->FindComponentByClass<UCallbackComponent>();
	SetState(nullptr);
}

void UActorStateComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State)
	{
		State->Tick(DeltaTime);
	}
}

void UActorStateComponent::OnRep_StateClass()
{
	SetStateInternal(ReplicatedStateClass);
}

void UActorStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActorStateComponent, ReplicatedStateClass);
}