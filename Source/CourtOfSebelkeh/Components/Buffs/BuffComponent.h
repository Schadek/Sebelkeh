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

	UPROPERTY()
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
		UBuff* AddBuff(TSubclassOf<UBuff> Class, AActor* Instigator, UObject* Source, float Duration = -1.f);

	UFUNCTION(BlueprintCallable)
		void RemoveBuff(int32 ID, AActor* Instigator, UObject* Source);

	UFUNCTION(BlueprintCallable)
		void RemoveBuffByReference(UBuff* Buff, AActor* Instigator, UObject* Source);

	UFUNCTION(BlueprintCallable)
		bool RemoveBuffOfType(EBuffType Type, AActor* Instigator, UObject* Source, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, meta = (KeyWords = "Remove All"))
		void ClearAll(AActor* Instigator, UObject* Source, int32& OutRemovedActiveBuffs, int32& OutRemovedBuffsTotal, TArray<UBuff*>& OutRemovedActiveBuffInstances);

	UFUNCTION(BlueprintCallable, meta = (KeyWords = "Remove All Type"))
		void ClearAllOfType(EBuffType Type, AActor* Instigator, UObject* Source, int32& OutRemovedActiveBuffs, int32& OutRemovedBuffsTotal, TArray<UBuff*>& OutRemovedActiveBuffInstances);

	UFUNCTION(BlueprintCallable)
		UBuff* GetBuffById(int32 ID) const;

	UFUNCTION(BlueprintCallable)
		int32 GetBuffAmount() const;

	UFUNCTION(BlueprintCallable)
		int32 GetBuffAmountByType(EBuffType Type) const;

	UFUNCTION(BlueprintCallable)
		bool HasBuffByType(EBuffType Type) const { return GetBuffAmountByType(Type) > 0; }

protected:

	UBuff* GetStrongestBuffOfClass(TSubclassOf<UBuff> Class, UBuff*& SecondStrongest, UBuff* QueryOrigin);

	void RemoveAllBuffsOfType(TSubclassOf<UBuff> Class, AActor* Instigator, UObject* Source);

	void QueueBuffRemoval(UBuff* Buff, EBuffEndReason Reason, AActor* Instigator, UObject* Source);
	void QueueBuffRemoval(int32 ID, EBuffEndReason Reason, AActor* Instigator, UObject* Source);
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
	int32 ActiveBuffAmount;
};