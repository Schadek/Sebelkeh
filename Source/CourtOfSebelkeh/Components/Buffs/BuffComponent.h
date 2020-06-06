// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CourtOfSebelkeh/Buffs/BuffSystemDefinitions.h"
#include "BuffComponent.generated.h"

class UCallbackComponent;

USTRUCT()
struct FBuffArrayWrapper
{
	GENERATED_BODY()

public:

	TArray<UBuff*> Buffs;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, DisplayThumbnail = "true"))
class COURTOFSEBELKEH_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UBuffComponent();

protected:

	virtual void BeginPlay() override;

public:

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, meta = (ComponentClass = "BuffComponent"), meta = (DeterminesOutputType = "Class"))
		UBuff* AddBuff(TSubclassOf<UBuff> Class, AActor* Instigator, float Duration = -1.f);

	UFUNCTION(BlueprintCallable)
		void RemoveBuff(int32 ID);

	UFUNCTION(BlueprintCallable)
		void RemoveBuffByReference(UBuff* Buff);

	UFUNCTION(BlueprintCallable, meta = (KeyWords = "Remove All"))
		void ClearAll();

	UFUNCTION(BlueprintCallable, meta = (KeyWords = "Remove All Type"))
		void ClearAllOfType(EBuffType Type);

	UFUNCTION(BlueprintCallable)
		UBuff* GetBuffById(int32 ID) const;

protected:

	UBuff* GetStrongestBuffOfClass(TSubclassOf<UBuff> Class, UBuff* SecondStrongest);

	void RemoveAllBuffsOfType(TSubclassOf<UBuff> Class);

	void QueueBuffRemoval(UBuff* Buff, EBuffEndReason Reason);
	void QueueBuffRemoval(int32 ID, EBuffEndReason Reason);
	void RemoveBuffInternal(UBuff* Buff);

	void AddReplicatedBuff(const FBuffAddedRMIData& Data);
	void RemoveReplicatedBuff(const FBuffRemovedRMIData& Data);

	UFUNCTION(Client, Unreliable)
		void OwnerOnly_BuffAdded(const FBuffAddedRMIData& Data);
	UFUNCTION(Client, Unreliable)
		void OwnerOnly_BuffRemoved(const FBuffRemovedRMIData& Data);
	UFUNCTION(NetMulticast, Unreliable)
		void Multicast_BuffAdded(const FBuffAddedRMIData& Data);
	UFUNCTION(NetMulticast, Unreliable)
		void Multicast_BuffRemoved(const FBuffRemovedRMIData& Data);

	UFUNCTION(Client, Unreliable)
		void OwnerOnly_BuffAddedMulti(const TArray<FBuffAddedRMIData>& Data);
	UFUNCTION(Client, Unreliable)
		void OwnerOnly_BuffRemovedMulti(const TArray<FBuffRemovedRMIData>& Data);
	UFUNCTION(NetMulticast, Unreliable)
		void Multicast_BuffAddedMulti(const TArray<FBuffAddedRMIData>& Data);
	UFUNCTION(NetMulticast, Unreliable)
		void Multicast_BuffRemovedMulti(const TArray<FBuffRemovedRMIData>& Data);

	UPROPERTY()
		TArray<FBuffArrayWrapper> Buffs;

	TMap<int32, EBuffType> BuffTypeMap;
	TArray<UBuff*> BuffsMarkedForDestruction;

	UPROPERTY()
		UCallbackComponent* CallbackComponent;

	TArray<FBuffAddedRMIData> AccumulatedAddedBuffData;
	TArray<FBuffRemovedRMIData> AccumulatedRemovedBuffData;

	TArray<FBuffAddedRMIData> AccumulatedAddedBuffDataMulticast;
	TArray<FBuffRemovedRMIData> AccumulatedRemovedBuffDataMulticast;

	int32 BuffIDCounter;
};