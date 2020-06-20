// Fill out your copyright notice in the Description page of Project Settings.


#include "StatComponent.h"
#include "CourtOfSebelkeh/Components/CallbackComponent.h"
#include "CourtOfSebelkeh/Components/State/ActorStateComponent.h"
#include "CourtOfSebelkeh/ActorStates/ActorStateDefinitions.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"

UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


int32 UStatComponent::Heal(int32 Value, AActor* Healer, UObject* Source)
{
	if (!GetWorld()->IsServer())
		return 0;

	UCallbackComponent* HealerCallback = nullptr;
	if (Healer)
	{
		HealerCallback = Healer->FindComponentByClass<UCallbackComponent>();
	}

	if (HealerCallback)
	{
		FPreDamageHealedEventInfo PreHealInfo;
		PreHealInfo.Amount = Value;
		PreHealInfo.Healer = Healer;
		PreHealInfo.Source = Source;
		PreHealInfo.Target = GetOwner();

		HealerCallback->BroadcastPreHealApplied(PreHealInfo);
		Value = PreHealInfo.Amount;
	}

	if (Callback)
	{
		FPreDamageHealedEventInfo PreHealInfo;
		PreHealInfo.Amount = Value;
		PreHealInfo.Healer = Healer;
		PreHealInfo.Source = Source;
		PreHealInfo.Target = GetOwner();

		Callback->BroadcastPreHealReceived(PreHealInfo);
		Value = PreHealInfo.Amount;
	}

	Value = FMath::Max(Value, 0);
	AddStat(EStat::Health, Value);

	if (HealerCallback)
	{
		FDamageHealedEventInfo HealInfo;
		HealInfo.Amount = Value;
		HealInfo.Healer = Healer;
		HealInfo.Source = Source;
		HealInfo.Target = GetOwner();

		HealerCallback->BroadcastHealApplied(HealInfo);
	}

	if (Callback)
	{
		FDamageHealedEventInfo HealInfo;
		HealInfo.Amount = Value;
		HealInfo.Healer = Healer;
		HealInfo.Source = Source;
		HealInfo.Target = GetOwner();

		Callback->BroadcastHealReceived(HealInfo);
	}

	return Value;
}

bool UStatComponent::Resurrect(int32 Health, int32 Energy, AActor* Instigator)
{
	UActorStateComponent* ActorStateComponent = GetOwner()->FindComponentByClass<UActorStateComponent>();
	bool bIsStateDead = true;

	if (ActorStateComponent)
	{
		bIsStateDead = ActorStateComponent->GetState()->IsA<UDeadActorState>();
	}

	if (bIsStateDead && GetStatRealNoCheck(EStat::Health) <= 0)
	{
		AddStat(EStat::Health, Health);
		AddStat(EStat::Energy, Energy);

		if (ActorStateComponent)
		{
			ActorStateComponent->SetState(nullptr, Instigator);
		}

		return true;
	}

	return false;
}

void UStatComponent::AddStat(EStat Type, int32 Value)
{
	int32 CurValue = 0;
	if (GetStatRaw(Type, CurValue))
	{
		SetStatInternal(Type, FMath::Clamp<int32>(CurValue + Value, 0, TNumericLimits<uint16>::Max()));
	}
}

void UStatComponent::SetStat(EStat Type, int32 Value)
{
	SetStatInternal(Type, FMath::Clamp<int32>(Value, 0, TNumericLimits<uint16>::Max()));
}

bool UStatComponent::GetStatRaw(EStat Type, int32& OutStat) const
{
	for (int i = 0; i < Stats.Num(); i++)
	{
		if (Stats[i].Type == Type)
		{
			OutStat = Stats[i].Value;
			return true;
		}
	}
	return false;
}

int32 UStatComponent::GetStatRawNoCheck(EStat Type) const
{
	for (int i = 0; i < Stats.Num(); i++)
	{
		if (Stats[i].Type == Type)
		{
			return Stats[i].Value;
		}
	}
	return 0;
}

bool UStatComponent::GetStatReal(EStat Type, int32& OutStat) const
{
	if (GetStatRaw(Type, OutStat))
	{
		OutStat = ConvertRawToRealStat(Type, OutStat);
		return true;
	}

	return false;
}

int32 UStatComponent::GetStatRealNoCheck(EStat Type) const
{
	return ConvertRawToRealStat(Type, GetStatRawNoCheck(Type));
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

	Callback = Cast<UCallbackComponent>(GetOwner()->GetComponentByClass(UCallbackComponent::StaticClass()));

	ConvertDefaultValues();
	InitializeReplication();

	if (GetWorld()->IsServer())
	{
		UpdateReplicatedValuesFromLocalOnes();
	}

	InitializeHealth();
	InitializeEnergy();
}

void UStatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HealthRegenerationIndex >= 0)
	{
		//Formula = 1 Regen = 2 Health per second
		const int32 RegenValue = ConvertRawToRealStat(EStat::HealthRegeneration, Stats[HealthRegenerationIndex].Value);
		const int32 UsableValue = FMath::Clamp(RegenValue, 0, 10);
		const float RegeneratedHealth = DeltaTime * RegenValue * 2.f;
		const float AccumulatedRegeneratedHealth = HealthRegenerationOverflow + RegeneratedHealth;
		const int32 RegeneratedHealthRounded = FMath::FloorToInt(AccumulatedRegeneratedHealth);
		HealthRegenerationOverflow = AccumulatedRegeneratedHealth - RegeneratedHealthRounded;

		AddStat(EStat::Health, RegeneratedHealthRounded);
	}

	if (EnergyRegenerationIndex >= 0)
	{
		//Pfeil = 0,33EN/Sek
		const int32 RegenValue = ConvertRawToRealStat(EStat::EnergyRegeneration, Stats[EnergyRegenerationIndex].Value);
		const int32 UsableValue = FMath::Clamp(RegenValue, 0, 10);
		const float RegeneratedEnergy = DeltaTime * RegenValue * 0.333333f;
		const float AccumulatedRegeneratedEnergy = EnergyRegenerationOverflow + RegeneratedEnergy;
		const int32 RegeneratedEnergyRounded = FMath::FloorToInt(AccumulatedRegeneratedEnergy);
		EnergyRegenerationOverflow = AccumulatedRegeneratedEnergy - RegeneratedEnergyRounded;

		AddStat(EStat::Energy, RegeneratedEnergyRounded);
	}
}

void UStatComponent::InitializeReplication()
{
	if (IsReplicationInitialized)
		return;

	SetIsReplicated(true);

	if (GetWorld()->IsServer())
	{
		for (int i = 0; i < Stats.Num(); i++)
		{
			if (!Stats[i].bServerOnly && !Stats[i].bUpdatedRarely)
			{
				ReplicatedValues.Emplace();
			}
		}
	}

	for (int i = 0; i < Stats.Num(); i++)
	{
		if (!Stats[i].bServerOnly && !Stats[i].bUpdatedRarely)
		{
			RepValuesToLocalMap.Add(i);
			Stats[i].ReplicationIndex = RepValuesToLocalMap.Num() - 1;
		}
	}

	IsReplicationInitialized = true;
}

void UStatComponent::UpdateReplicatedValuesFromLocalOnes()
{
	InitializeReplication();
	for (int i = 0; i < Stats.Num(); i++)
	{
		if (!Stats[i].bServerOnly && !Stats[i].bUpdatedRarely)
		{
			ReplicatedValues[Stats[i].ReplicationIndex] = Stats[i].Value;
		}
	}
}

void UStatComponent::UpdateLocalValuesFromReplicatedOnes()
{
	InitializeReplication();
	for (int i = 0; i < ReplicatedValues.Num(); i++)
	{
		FStatInfo& Info = Stats[RepValuesToLocalMap[i]];
		uint16 OldValue = Info.Value;
		uint16 NewValue = ReplicatedValues[i];
		Info.Value = ReplicatedValues[i];

		if (OldValue != NewValue && Callback)
		{
			FStatEventInfo EventInfo;
			EventInfo.Type = Info.Type;
			EventInfo.OldValue = OldValue;
			EventInfo.NewValue = NewValue;
			Callback->BroadcastStatChanged(EventInfo);
		}
	}
}

int32 UStatComponent::GetStatIndex(EStat Type) const
{
	for (int i = 0; i < Stats.Num(); i++)
	{
		if (Stats[i].Type == Type)
		{
			return i;
		}
	}

	return -1;
}

void UStatComponent::ConvertDefaultValues()
{
	for (int i = 0; i < Stats.Num(); i++)
	{
		Stats[i].Value = ConvertRealToRawStat(Stats[i].Type, Stats[i].Value);
	}
}

void UStatComponent::InitializeHealth()
{
	int32 Health = 0;
	int32 MaxHealth = 0;

	const bool bHasHealth = GetStatRaw(EStat::Health, Health);
	const bool bHasMaxHealth = GetStatRaw(EStat::MaxHealth, MaxHealth);

	if (bHasHealth ^ bHasMaxHealth)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor %s has incorrect health setup!"), *GetOwner()->GetName());
	}
	else if (Health > MaxHealth)
	{
		//Only the case if bHasHealth and bHasMaxHealth is true and the default params say that health is higher than max health
		SetStat(EStat::Health, MaxHealth);
	}

	HealthIndex = GetStatIndex(EStat::Health);
	MaxHealthIndex = GetStatIndex(EStat::MaxHealth);
	HealthRegenerationIndex = GetStatIndex(EStat::HealthRegeneration);
}

void UStatComponent::InitializeEnergy()
{
	int32 Energy = 0;
	int32 MaxEnergy = 0;

	const bool bHasEnergy = GetStatRaw(EStat::Energy, Energy);
	const bool bHasMaxEnergy = GetStatRaw(EStat::MaxEnergy, MaxEnergy);

	if (bHasEnergy ^ bHasMaxEnergy)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor %s has incorrect energy setup!"), *GetOwner()->GetName());
	}
	else if (Energy > MaxEnergy)
	{
		//Only the case if bHasHealth and bHasMaxHealth is true and the default params say that health is higher than max health
		SetStat(EStat::Energy, MaxEnergy);
	}

	EnergyIndex = GetStatIndex(EStat::Energy);
	MaxEnergyIndex = GetStatIndex(EStat::MaxEnergy);
	EnergyRegenerationIndex = GetStatIndex(EStat::EnergyRegeneration);
}

void UStatComponent::SetStatInternal(EStat Type, uint16 Value)
{
	ClampStats(Type, Value);

	for (int i = 0; i < Stats.Num(); i++)
	{
		if (Stats[i].Type == Type)
		{
			FStatInfo& Info = Stats[i];
			uint16 OldValue = Info.Value;
			Info.Value = Value;

			if (OldValue != Value && Callback)
			{
				FStatEventInfo EventInfo;
				EventInfo.Type = Info.Type;
				EventInfo.OldValue = OldValue;
				EventInfo.NewValue = Value;
				Callback->BroadcastStatChanged(EventInfo);
			}

			if (!Stats[i].bServerOnly && !Stats[i].bUpdatedRarely)
			{
				ReplicatedValues[i] = Value;
			}

			break;
		}
	}
}

int32 UStatComponent::ConvertRealToRawStat(EStat Type, int32 RealValue) const
{
	switch (Type)
	{
	case EStat::EnergyRegeneration:
	case EStat::HealthRegeneration:
	{
		constexpr int32 Offset = TNumericLimits<uint16>::Max() / 2;
		return RealValue + Offset;
	}
	default:
		break;
	}

	return RealValue;
}

int32 UStatComponent::ConvertRawToRealStat(EStat Type, int32 RawValue) const
{
	constexpr int32 Offset = TNumericLimits<uint16>::Max() / 2;

	switch (Type)
	{
	case EStat::EnergyRegeneration:
	case EStat::HealthRegeneration:
		return RawValue - Offset;
	default:
		break;
	}

	return RawValue;
}

void UStatComponent::ClampStats(EStat Type, uint16& InOutValue) const
{
	int32 Value32 = static_cast<int32>(InOutValue);

	switch (Type)
	{
	case EStat::Health:
		InOutValue = FMath::Min(Value32, GetStatRawNoCheck(EStat::MaxHealth));
		break;
	case EStat::Energy:
		InOutValue = FMath::Min(Value32, GetStatRawNoCheck(EStat::MaxEnergy));
		break;
	case EStat::Experience:
		InOutValue = FMath::Min(Value32, GetStatRawNoCheck(EStat::ExperienceToLevelUp));
		break;
	case EStat::ExperienceToLevelUp:
		break;
	case EStat::Overcast:
		InOutValue = FMath::Min(Value32, GetStatRawNoCheck(EStat::MaxEnergy));
		break;
	default:
		break;
	}
}

void UStatComponent::OnDamageDealt_Implementation(const FDamageInfo& Info)
{
	if (Callback)
	{
		FDamageDealtEventInfo DealInfo;
		DealInfo.DamageInfo = Info;
		Callback->BroadcastDamageDealt(DealInfo);
	}
}

void UStatComponent::OnRep_ReplicatedValues()
{
	UpdateLocalValuesFromReplicatedOnes();
}

void UStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatComponent, ReplicatedValues);
}
