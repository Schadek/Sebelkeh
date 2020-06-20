// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CourtOfSebelkeh/Callbacks/CallbackSystemDefinitions.h"
#include "CourtOfSebelkeh/Damage/DamageSystemDefinitions.h"
#include "CallbackComponent.generated.h"

class ICallbackListener;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COURTOFSEBELKEH_API UCallbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCallbackComponent();


	UFUNCTION(BlueprintCallable)
		bool RegisterCallback(ECallback Type, UObject* Listener);
	UFUNCTION(BlueprintCallable)
		bool UnregisterCallback(ECallback Type, UObject* Listener);

	void BroadcastCostCalculation(FCostCalculationEventInfo& Info);
	void BroadcastPreDamageDealt(FDamageEventInfo& Info);
	void BroadcastDamageDealt(const FDamageDealtEventInfo& Info);
	void BroadcastPreDamageReceived(FDamageEventInfo& Info);
	void BroadcastDamageReceived(const FDamageEventInfo& Info);
	void BroadcastPreHealApplied(FPreDamageHealedEventInfo& Info);
	void BroadcastHealApplied(const FDamageHealedEventInfo& Info);
	void BroadcastPreHealReceived(FPreDamageHealedEventInfo& Info);
	void BroadcastHealReceived(const FDamageHealedEventInfo& Info);
	void BroadcastPreBuffApplied(FPreBuffAppliedEventInfo& Info);
	void BroadcastPreBuffReceived(FPreBuffReceivedEventInfo& Info);
	void BroadcastBuffAdded(const FBuffEventInfo& Info);
	void BroadcastBuffRemoved(const FBuffEventInfo& Info);
	void BroadcastStatChanged(const FStatEventInfo& Info);
	void BroadcastSkillOnCooldown(FCooldownEventInfo& Info);
	void BroadcastSkillChannel(FSkillChannelEventInfo& Info);
	void BroadcastPreSkillUsed(FPreSkillUsedEventInfo& Info);
	void BroadcastSkillUsed(const FSkillUsedEventInfo& Info);
	void BroadcastPreActorStateChange(FPreActorStateEventInfo& Info);
	void BroadcastActorStateChange(const FActorStateEventInfo& Info);

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	struct FListener
	{
		bool bActive = false;
		UObject* Listener = nullptr;
	};

	TArray<TArray<FListener>> Listeners;
	TArray<bool> DirtyListenerArrays;

};
