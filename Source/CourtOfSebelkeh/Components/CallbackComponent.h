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

	void BroadcastPreDamageDealt(FDamageEventInfo& Info);
	void BroadcastDamageDealt(const FDamageDealtEventInfo& Info);
	void BroadcastPreDamageReceived(FDamageEventInfo& Info);
	void BroadcastDamageReceived(const FDamageEventInfo& Info);
	void BroadcastEnchantmentAdded(FBuffEventInfo& Info);
	void BroadcastEnchantmentRemoved(FBuffEventInfo& Info);
	void BroadcastHexAdded(FBuffEventInfo& Info);
	void BroadcastHexRemoved(FBuffEventInfo& Info);
	void BroadcastStatChanged(const FStatEventInfo& Info);
	void BroadcastSkillOnCooldown(FCooldownEventInfo& Info);
	void BroadcastSkillChannel(FSkillChannelEventInfo& Info);
	void BroadcastPreActorStateChange(FPreActorStateEventInfo& Info);

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
