// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "CourtOfSebelkeh/GameStates/CoreGameState.h"
#include "CourtOfSebelkeh/Components/CallbackComponent.h"

// Sets default values for this component's properties
UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
	SetComponentTickEnabled(GetWorld()->IsServer());

	CallbackComponent = Cast<UCallbackComponent>(GetOwner()->GetComponentByClass(UCallbackComponent::StaticClass()));

	Buffs.SetNum(static_cast<int32>(EBuffType::Count));
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float Time = GetWorld()->GetTimeSeconds();

	for (FBuffArrayWrapper& BuffArray : Buffs)
	{
		for (UBuff* Buff : BuffArray.Buffs)
		{
			if (!Buff->IsDone())
			{
				if (Buff->GetEndTimestamp() > 0 && Time >= Buff->GetEndTimestamp())
				{
					QueueBuffRemoval(Buff, EBuffEndReason::Expiration, nullptr, nullptr);
				}
				else if (Buff->WantsTick(Time))
				{
					Buff->Tick(DeltaTime);
				}
			}
		}
	}

	for (UBuff* Buff : BuffsMarkedForDestruction)
	{
		RemoveBuffInternal(Buff);
	}
	BuffsMarkedForDestruction.Empty(BuffsMarkedForDestruction.Max());

	if (GetWorld()->IsServer())
	{
		if (AccumulatedAddedBuffData.Num() > 0)
		{
			if (AccumulatedAddedBuffData.Num() == 1)
			{
				//Send optimized single argument RPC to save vector size params
				OwnerOnly_BuffAdded(AccumulatedAddedBuffData[0]);
			}
			else
			{
				//Send RPC with vector argument
				OwnerOnly_BuffAddedMulti(AccumulatedAddedBuffData);
			}

			AccumulatedAddedBuffData.Empty(AccumulatedAddedBuffData.Max());
		}

		if (AccumulatedRemovedBuffData.Num() > 0)
		{
			if (AccumulatedRemovedBuffData.Num() == 1)
			{
				//Send optimized single argument RPC to save vector size params
				OwnerOnly_BuffRemoved(AccumulatedRemovedBuffData[0]);
			}
			else
			{
				//Send RPC with vector argument
				OwnerOnly_BuffRemovedMulti(AccumulatedRemovedBuffData);
			}

			AccumulatedRemovedBuffData.Empty(AccumulatedRemovedBuffData.Max());
		}

		if (AccumulatedAddedBuffDataMulticast.Num() > 0)
		{
			if (AccumulatedAddedBuffDataMulticast.Num() == 1)
			{
				//Send optimized single argument RPC to save vector size params
				Multicast_BuffAdded(AccumulatedAddedBuffDataMulticast[0]);
			}
			else
			{
				//Send RPC with vector argument
				Multicast_BuffAddedMulti(AccumulatedAddedBuffDataMulticast);
			}

			AccumulatedAddedBuffDataMulticast.Empty(AccumulatedAddedBuffDataMulticast.Max());
		}

		if (AccumulatedRemovedBuffDataMulticast.Num() > 0)
		{
			if (AccumulatedRemovedBuffDataMulticast.Num() == 1)
			{
				//Send optimized single argument RPC to save vector size params
				Multicast_BuffRemoved(AccumulatedRemovedBuffDataMulticast[0]);
			}
			else
			{
				//Send RPC with vector argument
				Multicast_BuffRemovedMulti(AccumulatedRemovedBuffDataMulticast);
			}

			AccumulatedRemovedBuffDataMulticast.Empty(AccumulatedRemovedBuffDataMulticast.Max());
		}
	}
}

UBuff* UBuffComponent::AddBuff(TSubclassOf<UBuff> Class, AActor* Instigator, UObject* Source, float Duration /* = -1.f*/)
{
	if (!GetWorld()->IsServer())
		return nullptr;

	const int32 NewID = BuffIDCounter++;
	const bool bIsInfinite = Duration < 0.0f;

	UBuff* NewBuff = NewObject<UBuff>(this, Class);

	if (UCallbackComponent* InstigatorCallback = Instigator->FindComponentByClass<UCallbackComponent>())
	{
		FPreBuffAppliedEventInfo PreBuffAppliedEventInfo;
		PreBuffAppliedEventInfo.Buff = NewBuff;
		PreBuffAppliedEventInfo.Duration = Duration;
		PreBuffAppliedEventInfo.Instigator = Instigator;
		PreBuffAppliedEventInfo.Source = Source;

		InstigatorCallback->BroadcastPreBuffApplied(PreBuffAppliedEventInfo);
		Duration = PreBuffAppliedEventInfo.Duration;
	}

	if (CallbackComponent)
	{
		FPreBuffReceivedEventInfo PreBuffReceivedEventInfo;
		PreBuffReceivedEventInfo.Buff = NewBuff;
		PreBuffReceivedEventInfo.Duration = Duration;
		PreBuffReceivedEventInfo.Instigator = Instigator;
		PreBuffReceivedEventInfo.Source = Source;

		CallbackComponent->BroadcastPreBuffReceived(PreBuffReceivedEventInfo);
		Duration = PreBuffReceivedEventInfo.Duration;
	}

	const float EndTimeStamp = bIsInfinite ? -1.f : GetWorld()->GetTimeSeconds() + Duration;
	NewBuff->Begin(this, GetOwner(), Instigator, NewID, EndTimeStamp, bIsInfinite);

	Buffs[static_cast<int32>(NewBuff->GetType())].Buffs.Add(NewBuff);
	BuffTypeMap.Add(NewID, NewBuff->GetType());

	UBuff* SecondStrongest = nullptr;
	UBuff* StrongestBuff = GetStrongestBuffOfClass(Class, SecondStrongest, NewBuff);
	if (NewBuff == StrongestBuff)
	{
		//This is the second, stronger instance of the buff 
		if (SecondStrongest && SecondStrongest->IsActive())
		{
			ActiveBuffAmount--;
			SecondStrongest->Deactivate();
		}

		ActiveBuffAmount++;
		NewBuff->Activate();
	}


	if (CallbackComponent)
	{
		FBuffEventInfo Info;
		Info.Buff = NewBuff;
		Info.Instigator = Instigator;
		Info.Source = Source;

		CallbackComponent->BroadcastBuffAdded(Info);
	}

	switch (NewBuff->GetNetRelevancy())
	{
	case EBuffRelevancy::Owner:
	{
		FBuffAddedRMIData& Data = AccumulatedAddedBuffData.Emplace_GetRef();
		Data.Class = Class;
		Data.ID = NewID;
		Data.EndTimeStamp = EndTimeStamp;
		Data.Instigator = Instigator;
		Data.Source = Source;
		break;
	}
	case EBuffRelevancy::All:
	{
		FBuffAddedRMIData& Data = AccumulatedAddedBuffDataMulticast.Emplace_GetRef();
		Data.Class = Class;
		Data.ID = NewID;
		Data.EndTimeStamp = EndTimeStamp;
		Data.Instigator = Instigator;
		Data.Source = Source;
		break;
	}
	default:
		break;
	}

	return NewBuff;
}

void UBuffComponent::RemoveBuffByReference(UBuff* Buff, AActor* Instigator, UObject* Source)
{
	RemoveAllBuffsOfType(Buff->GetClass(), Instigator, Source);
}

bool UBuffComponent::RemoveBuffOfType(EBuffType Type, AActor* Instigator, UObject* Source, int32 Amount)
{
	TArray<UBuff*>& BuffArray = Buffs[static_cast<int32>(Type)].Buffs;
	for (int i = 0; i < Amount; i++)
	{
		UBuff* Buff = nullptr;
		for (int k = BuffArray.Num() - 1; k >= 0; k--)
		{
			if (!BuffArray[k]->IsDone())
			{
				Buff = BuffArray[k];
				break;
			}
		}

		if (!Buff)
		{
			break;
		}

		RemoveBuffByReference(Buff, Instigator, Source);
	}

	return Amount > 0;
}

void UBuffComponent::RemoveBuff(int32 ID, AActor* Instigator, UObject* Source)
{
	if (UBuff* Buff = GetBuffById(ID))
	{
		RemoveAllBuffsOfType(Buff->GetClass(), Instigator, Source);
	}
}

void UBuffComponent::ClearAll(AActor* Instigator, UObject* Source, int32& OutRemovedActiveBuffs, int32& OutRemovedBuffsTotal, TArray<UBuff*>& OutRemovedActiveBuffInstances)
{
	OutRemovedActiveBuffInstances.Reserve(10);
	for (int i = 0; i < static_cast<int32>(EBuffType::Count); i++)
	{
		ClearAllOfType(static_cast<EBuffType>(i), Instigator, Source, OutRemovedActiveBuffs, OutRemovedBuffsTotal, OutRemovedActiveBuffInstances);
	}
}

void UBuffComponent::ClearAllOfType(EBuffType Type, AActor* Instigator, UObject* Source, int32& OutRemovedActiveBuffs, int32& OutRemovedBuffsTotal, TArray<UBuff*>& OutRemovedActiveBuffInstances)
{
	OutRemovedActiveBuffInstances.Reserve(4);

	TArray<UBuff*>& Array = Buffs[static_cast<int32>(Type)].Buffs;
	for (int i = Array.Num() - 1; i >= 0; i--)
	{
		if (Array[i]->IsActive())
		{
			OutRemovedActiveBuffInstances.Add(Array[i]);
			OutRemovedActiveBuffs++;
		}
		OutRemovedBuffsTotal++;

		RemoveBuff(Array[i]->GetID(), Instigator, Source);
	}
}

UBuff* UBuffComponent::GetBuffById(int32 ID) const
{
	if (const EBuffType* Type = BuffTypeMap.Find(ID))
	{
		for (UBuff* Buff : Buffs[static_cast<int32>(*Type)].Buffs)
		{
			if (Buff->GetID() == ID)
			{
				return Buff;
			}
		}
	}

	return nullptr;
}

int32 UBuffComponent::GetBuffAmount() const
{
	return ActiveBuffAmount;
}

int32 UBuffComponent::GetBuffAmountByType(EBuffType Type) const
{
	return Buffs[static_cast<int32>(Type)].Buffs.Num();
}

UBuff* UBuffComponent::GetStrongestBuffOfClass(TSubclassOf<UBuff> Class, UBuff*& SecondStrongest, UBuff* QueryOrigin)
{
	if (!Class)
		return nullptr;

	UBuff* DefaultObject = Cast<UBuff>(Class->GetDefaultObject());
	UBuff* Result = nullptr;
	SecondStrongest = nullptr;
	int32 HighestAttribute = -1;

	for (UBuff* Buff : Buffs[static_cast<int32>(DefaultObject->GetType())].Buffs)
	{
		if (Buff->GetClass() == Class && !Buff->IsDone())
		{
			int32 Strength = Buff->GetAttributeValue();
			if (Strength > HighestAttribute || (Strength == HighestAttribute && QueryOrigin == Buff))
			{
				if (!SecondStrongest || !SecondStrongest->IsActive())
				{
					SecondStrongest = Result;
				}

				Result = Buff;
				HighestAttribute = Strength;
			}
			else if (!SecondStrongest || Buff->IsActive())
			{
				SecondStrongest = Buff;
			}
		}
	}

	return Result;
}

void UBuffComponent::RemoveAllBuffsOfType(TSubclassOf<UBuff> Class, AActor* Instigator, UObject* Source)
{
	TArray<UBuff*> QualifiedBuffs;

	UBuff* DefaultObject = Cast<UBuff>(Class->GetDefaultObject());
	for (UBuff* Buff : Buffs[static_cast<int32>(DefaultObject->GetType())].Buffs)
	{
		if (Buff->GetClass() == Class)
		{
			QualifiedBuffs.Add(Buff);
		}
	}

	for (UBuff* Buff : QualifiedBuffs)
	{
		QueueBuffRemoval(Buff, EBuffEndReason::Removal, Instigator, Source);
	}
}

void UBuffComponent::QueueBuffRemoval(int32 ID, EBuffEndReason Reason, AActor* Instigator, UObject* Source)
{
	if (!GetWorld()->IsServer())
		return;

	EBuffType Type = EBuffType::Enchantment;
	if (BuffTypeMap.RemoveAndCopyValue(ID, Type))
	{
		TArray<UBuff*>& Array = Buffs[static_cast<int32>(Type)].Buffs;
		for (int i = 0; i < Array.Num(); i++)
		{
			UBuff* Buff = Array[i];
			if (Buff->GetID() == ID)
			{
				UBuff* SecondStrongest = nullptr;
				UBuff* StrongestBuff = GetStrongestBuffOfClass(Buff->GetClass(), SecondStrongest, Buff);
				if (Buff == StrongestBuff)
				{
					check(Buff->IsActive() && "Buff is not active, should never happen!");
					ActiveBuffAmount--;
					Buff->Deactivate();

					if (SecondStrongest && !SecondStrongest->IsActive())
					{
						//This is the strongest instance of this buff but there is another one applied
						ActiveBuffAmount++;
						SecondStrongest->Activate();
					}
				}

				Buff->End(Reason, SecondStrongest != nullptr);
				Buff->MarkForDestroy();
				BuffsMarkedForDestruction.Add(Buff);

				if (CallbackComponent)
				{
					FBuffEventInfo Info;
					Info.Buff = Buff;
					Info.Instigator = Instigator;
					Info.Source = Source;
					CallbackComponent->BroadcastBuffRemoved(Info);
				}

				if (GetWorld()->IsServer())
				{
					switch (Buff->GetNetRelevancy())
					{
					case EBuffRelevancy::Owner:
					{
						FBuffRemovedRMIData& Data = AccumulatedRemovedBuffData.Emplace_GetRef();
						Data.ID = ID;
						Data.Reason = Reason;
						Data.Instigator = Instigator;
						Data.Source = Source;
						break;
					}
					case EBuffRelevancy::All:
					{
						FBuffRemovedRMIData& Data = AccumulatedRemovedBuffDataMulticast.Emplace_GetRef();
						Data.ID = ID;
						Data.Reason = Reason;
						Data.Instigator = Instigator;
						Data.Source = Source;
						break;
					}
					default:
						break;
					}

					break;
				}
			}
		}
	}
}

void UBuffComponent::QueueBuffRemoval(UBuff* Buff, EBuffEndReason Reason, AActor* Instigator, UObject* Source)
{
	if (!GetWorld()->IsServer() || Buff->GetTargetActor() != GetOwner())
		return;

	BuffTypeMap.Remove(Buff->GetID());

	UBuff* SecondStrongest = nullptr;
	UBuff* StrongestBuff = GetStrongestBuffOfClass(Buff->GetClass(), SecondStrongest, Buff);
	if (Buff == StrongestBuff)
	{
		//Can be false if there are multiple instances of a buff with the same attribute level
		if (Buff->IsActive())
		{
			ActiveBuffAmount--;
			Buff->Deactivate();

			if (SecondStrongest && !SecondStrongest->IsActive())
			{
				//This is the strongest instance of this buff but there is another one applied
				ActiveBuffAmount++;
				SecondStrongest->Activate();
			}
		}
	}

	Buff->End(Reason, SecondStrongest != nullptr);
	Buff->MarkForDestroy();
	BuffsMarkedForDestruction.Add(Buff);

	if (CallbackComponent)
	{
		FBuffEventInfo Info;
		Info.Buff = Buff;
		Info.Instigator = Instigator;
		Info.Source = Source;
		CallbackComponent->BroadcastBuffRemoved(Info);
	}

	if (GetWorld()->IsServer())
	{
		switch (Buff->GetNetRelevancy())
		{
		case EBuffRelevancy::Owner:
		{
			FBuffRemovedRMIData& Data = AccumulatedRemovedBuffData.Emplace_GetRef();
			Data.ID = Buff->GetID();
			Data.Reason = Reason;
			Data.Instigator = Instigator;
			Data.Source = Source;
			break;
		}
		case EBuffRelevancy::All:
		{
			FBuffRemovedRMIData& Data = AccumulatedRemovedBuffDataMulticast.Emplace_GetRef();
			Data.ID = Buff->GetID();
			Data.Reason = Reason;
			Data.Instigator = Instigator;
			Data.Source = Source;
			break;
		}
		default:
			break;
		}
	}
}

void UBuffComponent::RemoveBuffInternal(UBuff* Buff)
{
	const int32 ID = Buff->GetID();

	TArray<UBuff*>& Array = Buffs[static_cast<int32>(Buff->GetType())].Buffs;
	for (int i = 0; i < Array.Num(); i++)
	{
		UBuff* CurBuff = Array[i];
		if (CurBuff->GetID() == ID)
		{
			//TODO: Cannot use RemoveSwap as to not break order (Investigate!)
			Array.RemoveAt(i);
		}
	}
}

void UBuffComponent::AddReplicatedBuff(const FBuffAddedRMIData& Data)
{
	UBuff* NewBuff = NewObject<UBuff>(this, Data.Class);
	NewBuff->Begin(this, GetOwner(), Data.Instigator, Data.ID, Data.EndTimeStamp, Data.EndTimeStamp < 0.0f);

	Buffs[static_cast<int32>(NewBuff->GetType())].Buffs.Add(NewBuff);
	BuffTypeMap.Add(Data.ID, NewBuff->GetType());

	UBuff* SecondStrongest = nullptr;
	UBuff* StrongestBuff = GetStrongestBuffOfClass(Data.Class, SecondStrongest, NewBuff);
	if (NewBuff == StrongestBuff)
	{
		//This is the second, stronger instance of the buff 
		if (SecondStrongest && SecondStrongest->IsActive())
		{
			ActiveBuffAmount--;
			SecondStrongest->Deactivate();
		}

		ActiveBuffAmount++;
		NewBuff->Activate();
	}

	if (CallbackComponent)
	{
		FBuffEventInfo Info;
		Info.Buff = NewBuff;
		Info.Instigator = Data.Instigator;
		Info.Source = Data.Source;
		CallbackComponent->BroadcastBuffAdded(Info);
	}
}

void UBuffComponent::RemoveReplicatedBuff(const FBuffRemovedRMIData& Data)
{
	EBuffType Type = EBuffType::Enchantment;
	if (BuffTypeMap.RemoveAndCopyValue(Data.ID, Type))
	{
		TArray<UBuff*>& Array = Buffs[static_cast<int32>(Type)].Buffs;
		for (int i = 0; i < Array.Num(); i++)
		{
			UBuff* Buff = Array[i];
			if (Buff->GetID() == Data.ID)
			{
				UBuff* SecondStrongest = nullptr;
				UBuff* StrongestBuff = GetStrongestBuffOfClass(Buff->GetClass(), SecondStrongest, Buff);
				if (Buff == StrongestBuff)
				{
					if (Buff->IsActive())
					{
						ActiveBuffAmount--;
						Buff->Deactivate();

						if (SecondStrongest && !SecondStrongest->IsActive())
						{
							//This is the strongest instance of this buff but there is another one applied
							ActiveBuffAmount++;
							SecondStrongest->Activate();
						}
					}
				}

				Buff->End(Data.Reason, SecondStrongest != nullptr);
				Buff->MarkForDestroy();
				BuffsMarkedForDestruction.Add(Buff);

				if (CallbackComponent)
				{
					FBuffEventInfo Info;
					Info.Buff = Buff;
					Info.Instigator = Data.Instigator;
					Info.Source = Data.Source;
					CallbackComponent->BroadcastBuffRemoved(Info);
				}
			}
		}
	}
}

void UBuffComponent::OwnerOnly_BuffAdded_Implementation(const FBuffAddedRMIData& Data)
{
	if (GetWorld()->IsServer())
		return;

	AddReplicatedBuff(Data);
}

void UBuffComponent::OwnerOnly_BuffRemoved_Implementation(const FBuffRemovedRMIData& Data)
{
	if (GetWorld()->IsServer())
		return;

	RemoveReplicatedBuff(Data);
}

void UBuffComponent::Multicast_BuffAdded_Implementation(const FBuffAddedRMIData& Data)
{
	if (GetWorld()->IsServer())
		return;

	AddReplicatedBuff(Data);
}

void UBuffComponent::Multicast_BuffRemoved_Implementation(const FBuffRemovedRMIData& Data)
{
	if (GetWorld()->IsServer())
		return;

	RemoveReplicatedBuff(Data);
}

void UBuffComponent::OwnerOnly_BuffAddedMulti_Implementation(const TArray<FBuffAddedRMIData>& Data)
{
	if (GetWorld()->IsServer())
		return;

	for (const FBuffAddedRMIData& RMIData : Data)
	{
		AddReplicatedBuff(RMIData);
	}
}

void UBuffComponent::OwnerOnly_BuffRemovedMulti_Implementation(const TArray<FBuffRemovedRMIData>& Data)
{
	if (GetWorld()->IsServer())
		return;

	for (const FBuffRemovedRMIData& RMIData : Data)
	{
		RemoveReplicatedBuff(RMIData);
	}
}

void UBuffComponent::Multicast_BuffAddedMulti_Implementation(const TArray<FBuffAddedRMIData>& Data)
{
	if (GetWorld()->IsServer())
		return;

	for (const FBuffAddedRMIData& RMIData : Data)
	{
		AddReplicatedBuff(RMIData);
	}
}

void UBuffComponent::Multicast_BuffRemovedMulti_Implementation(const TArray<FBuffRemovedRMIData>& Data)
{
	if (GetWorld()->IsServer())
		return;

	for (const FBuffRemovedRMIData& RMIData : Data)
	{
		RemoveReplicatedBuff(RMIData);
	}
}

