// Fill out your copyright notice in the Description page of Project Settings.


#include "CallbackComponent.h"
#include "CourtOfSebelkeh/Interfaces/CallbackListener.h"

UCallbackComponent::UCallbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCallbackComponent::BeginPlay()
{
	Super::BeginPlay();

	const int32 Count = static_cast<int32>(ECallback::Count);
	Listeners.SetNum(Count);
	DirtyListenerArrays.SetNum(Count);

	SetComponentTickEnabled(false);
}

void UCallbackComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Clean up removed callbacks
	for (int i = 0; i < DirtyListenerArrays.Num(); i++)
	{
		if (DirtyListenerArrays[i])
		{
			for (int j = Listeners[i].Num() - 1; j >= 0; j--)
			{
				if (!Listeners[i][j].bActive)
				{
					Listeners[i].RemoveAt(j);
				}
			}

			DirtyListenerArrays[i] = false;
		}
	}

	SetComponentTickEnabled(false);
}


bool UCallbackComponent::RegisterCallback(ECallback Type, UObject* Listener)
{
	if (Listener->Implements<UCallbackListener>())
	{
		FListener ListenerContainer;
		ListenerContainer.bActive = true;
		ListenerContainer.Listener = Listener;

		Listeners[static_cast<int32>(Type)].Add(ListenerContainer);
		return true;
	}

	return false;
}

bool UCallbackComponent::UnregisterCallback(ECallback Type, UObject* Listener)
{
	if (Listener->Implements<UCallbackListener>())
	{
		auto Predicate = [&](const FListener& ListenerContainer)
		{
			return ListenerContainer.Listener == Listener;
		};

		const int32 ArrayIndex = static_cast<int32>(Type);
		TArray<FListener>& EventListeners = Listeners[ArrayIndex];
		if (FListener* FoundListener = EventListeners.FindByPredicate(Predicate))
		{
			FoundListener->bActive = false;
			DirtyListenerArrays[ArrayIndex] = true;
			SetComponentTickEnabled(true);
			return true;
		}
	}

	return false;
}

void UCallbackComponent::BroadcastPreDamageDealt(FDamageEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::PreDamageDealt)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnPreDamageDealt(Object.Listener, this, Info, Info);
	}
}

void UCallbackComponent::BroadcastDamageDealt(const FDamageDealtEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::DamageDealt)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnDamageDealt(Object.Listener, this, Info);
	}
}

void UCallbackComponent::BroadcastPreDamageReceived(FDamageEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::PreDamageReceived)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnPreDamageReceived(Object.Listener, this, Info, Info);
	}
}

void UCallbackComponent::BroadcastDamageReceived(const FDamageEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::DamageReceived)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnDamageReceived(Object.Listener, this, Info);
	}
}

void UCallbackComponent::BroadcastEnchantmentAdded(FBuffEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::EnchantmentAdded)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnEnchantmentAdded(Object.Listener, this, Info, Info);
	}
}

void UCallbackComponent::BroadcastEnchantmentRemoved(FBuffEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::EnchantmentRemoved)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnEnchantmentRemoved(Object.Listener, this, Info, Info);
	}
}

void UCallbackComponent::BroadcastHexAdded(FBuffEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::HexAdded)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnHexAdded(Object.Listener, this, Info, Info);
	}
}

void UCallbackComponent::BroadcastHexRemoved(FBuffEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::HexRemoved)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnHexRemoved(Object.Listener, this, Info, Info);
	}
}

void UCallbackComponent::BroadcastStatChanged(const FStatEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::StatChanged)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnStatChanged(Object.Listener, this, Info);
	}
}

void UCallbackComponent::BroadcastSkillOnCooldown(FCooldownEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::SkillOnCooldown)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnSkillOnCooldown(Object.Listener, this, Info, Info);
	}
}

void UCallbackComponent::BroadcastSkillChannel(FSkillChannelEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::SkillChannel)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnSkillChannel(Object.Listener, this, Info, Info);
	}
}

void UCallbackComponent::BroadcastPreActorStateChange(FPreActorStateEventInfo& Info)
{
	TArray<FListener>& EventListeners = Listeners[static_cast<int32>(ECallback::PreActorStateChanged)];
	for (int i = 0, Num = EventListeners.Num(); i < Num; i++)
	{
		FListener& Object = EventListeners[i];
		if (Object.bActive)
			ICallbackListener::Execute_OnPreActorStateChange(Object.Listener, this, Info, Info);
	}
}