// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillComponent.h"
#include "CourtOfSebelkeh/GameStates/CoreGameState.h"
#include "CourtOfSebelkeh/Settings/GameSettings.h"
#include "CourtOfSebelkeh/Skills/SkillBase.h"
#include "CourtOfSebelkeh/Components/CallbackComponent.h"
#include "CourtOfSebelkeh/Components/State/ActorStateComponent.h"
#include "CourtOfSebelkeh/Components/Buffs/BuffComponent.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "CourtOfSebelkeh/Characters/CoreCharacter.h"
#include "CourtOfSebelkeh/HUD/NativeCharacterBar.h"
#include "CourtOfSebelkeh/Buffs/BuffSystemDefinitions.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USkillComponent::SetSkill(int32 Index, TSubclassOf<USkillBase> Class)
{
	if (Index < 8 && Class)
	{
		FSkillRPCData Data;
		Data.Index = Index;
		Data.Class = Class;
		SetNetworkSkill(Data);
	}
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeDefaultSkills();
	InitializeDefaultAttributes();
	InitializeAdrenaline();

	Callback = GetOwner()->FindComponentByClass<UCallbackComponent>();
	ActorState = GetOwner()->FindComponentByClass<UActorStateComponent>();
}

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (USkillBase* Skill : Skills)
	{
		Skill->Tick(DeltaTime);
	}
}

void USkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USkillComponent, MaintainedBuffs, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USkillComponent, AdrenalineLevels, COND_OwnerOnly);
}

void USkillComponent::InitializeDefaultSkills()
{
	Skills.SetNum(8);
	for (int i = 0; i < DefaultSkills.Num(); i++)
	{
		SetSkillInternal(i, DefaultSkills[i]);
	}

	TSubclassOf<USkillBase> Class = GetEmptySkillClass();
	for (int i = DefaultSkills.Num(); i < 8; i++)
	{
		SetSkillInternal(i, Class);
	}
}

void USkillComponent::InitializeDefaultAttributes()
{
	for (const FDefaultAttribute& DefaultAttribute : DefaultAttributes)
	{
		USkillAttribute* NewAttribute = NewObject<USkillAttribute>(this, DefaultAttribute.Class);
		NewAttribute->Value = DefaultAttribute.Value;
		Attributes.Add(NewAttribute);
	}
}

void USkillComponent::InitializeAdrenaline()
{
	AdrenalineLevels.SetNum(8);
}

void USkillComponent::OnSkillChanged(int32 Index, USkillBase* OldSkill, USkillBase* NewSkill)
{
	OnSkillChangedDel.Broadcast(this, Index, OldSkill, NewSkill);
}

void USkillComponent::NotifySkillChannelEnd(USkillBase* Skill, float Duration, bool bSuccess)
{
	int32 Index = 0;
	if (Skills.Find(Skill, Index))
	{
		NotifySkillChannelEnd(Index, Duration, bSuccess);
	}
}

void USkillComponent::NotifySkillChannelEnd(int32 Index, float Duration, bool bSuccess)
{
	if (GetWorld()->IsServer())
	{
		SyncSkillChannelEnd(Index, bSuccess);
	}
	else
	{
		CancelChanneling();
	}
}

void USkillComponent::OnMaintainedSkillsChanged()
{
	OnMaintainedBuffsChangedDel.Broadcast(this);
}

void USkillComponent::OnAdrenalineReplicated()
{

}

void USkillComponent::SetSkillInternal(int32 Index, TSubclassOf<USkillBase> Class)
{
	if (Index < 8)
	{
		USkillBase* OldSkill = Skills[Index];
		USkillBase* NewSkill = NewObject<USkillBase>(this, Class);

		if (OldSkill)
			OldSkill->End();

		Skills[Index] = NewSkill;
		NewSkill->Begin(this, true);

		OnSkillChanged(Index, OldSkill, NewSkill);
	}
}

TSubclassOf<USkillBase> USkillComponent::GetEmptySkillClass() const
{
	if (ACoreGameState* GameState = Cast<ACoreGameState>(GetWorld()->GetGameState()))
	{
		if (UGameSettings* Settings = GameState->GetGameSettings())
		{
			return Settings->EmptySkillClass ? Settings->EmptySkillClass : USkillBase::StaticClass();
		}
	}

	return USkillBase::StaticClass();
}

void USkillComponent::InternalRequestMaintainedBuffEnd_Implementation(int32 BuffId)
{
	for (int i = 0; i < MaintainedBuffs.Num(); i++)
	{
		if (MaintainedBuffs[i].BuffInstanceId == BuffId)
		{
			if (UBuffComponent* BuffComponent = MaintainedBuffs[i].Target->FindComponentByClass<UBuffComponent>())
			{
				BuffComponent->RemoveBuff(BuffId, GetOwner(), this);
			}
			break;
		}
	}
}

bool USkillComponent::InternalRequestMaintainedBuffEnd_Validate(int32 BuffId)
{
	return true;
}

void USkillComponent::UseSkillOnTargetInternal_Implementation(uint8 Index, AActor* Target)
{
	USkillBase* Skill = Skills[Index];
	Skill->UseTarget(Target);
}

bool USkillComponent::UseSkillOnTargetInternal_Validate(uint8 Index, AActor* Target)
{
	return true;
}

void USkillComponent::GetSkillsWithAttribute(TSubclassOf<USkillAttribute> Class, TArray<USkillBase*>& OutSkills)
{
	for (USkillBase* Skill : Skills)
	{
		if (Skill->GetAttribute() == Class)
		{
			OutSkills.Add(Skill);
		}
	}
}

void USkillComponent::RequestMaintainedBuffEnd(int32 BuffId)
{
	InternalRequestMaintainedBuffEnd(BuffId);
}

int32 USkillComponent::GetAttributeValue(TSubclassOf<USkillAttribute> Class) const
{
	for (USkillAttribute* Attribute : Attributes)
	{
		if (Attribute->GetClass() == Class)
		{
			return Attribute->Value;
		}
	}

	return 0;
}

bool USkillComponent::InterruptChanneling()
{
	if (USkillBase* ChanneledSkill = GetChanneledSkill())
	{
		if (ChanneledSkill->CanBeInterrupted())
		{
			SyncSkillChannelEnd(ChanneledSkillIndex, false);
		}
	}

	return false;
}

void USkillComponent::UseSkillOnTarget(int32 Index, AActor* Target)
{
	UseSkillOnTargetInternal(Index, Target);
}

void USkillComponent::UseSkillOnTarget(USkillBase* Skill, AActor* Target)
{
	int32 Index = 0;
	if (Skills.Find(Skill, Index))
	{
		UseSkillOnTarget(Index, Target);
	}
}

void USkillComponent::UseSkillOnLocation(int32 Index, const FVector& Location)
{
	UseSkillOnLocationInternal(Index, FVector_NetQuantize(Location));
}

void USkillComponent::UseSkillOnLocation(USkillBase* Skill, const FVector& Location)
{
	int32 Index = 0;
	if (Skills.Find(Skill, Index))
	{
		UseSkillOnLocation(Index, Location);
	}
}

void USkillComponent::SetSkillOnCooldown(int32 Index, float EndTimestamp)
{
	FSkillCooldownRPCData Data;
	Data.Index = Index;
	Data.EndTimestamp = EndTimestamp;

	SetSkillOnCooldownInternal(Data);
}

void USkillComponent::SetSkillOnCooldown(USkillBase* Skill, float EndTimestamp)
{
	int32 Index = 0;
	if (Skills.Find(Skill, Index))
	{
		SetSkillOnCooldown(Index, EndTimestamp);
	}
}


void USkillComponent::ChannelSkill(int32 Index, AActor* Target, const FVector& Location)
{
	USkillBase* Skill = Skills[Index];

	ChannelTarget = Target;
	ChannelLocation = Location;

	if (ActorState)
	{
		float ChannelTime = Skill->GetChannelTime();
		if (Callback)
		{
			FSkillChannelEventInfo Info;
			Info.Skill = Skill;
			Info.ChannelTime = ChannelTime;
			Callback->BroadcastSkillChannel(Info);

			ChannelTime = Info.ChannelTime;
		}

		if (Skill->IsStunnedWhileChanneling())
		{
			UChannelingActorState* ChannelState = Cast<UChannelingActorState>(ActorState->SetState(UCoreBlueprintLibrary::GetGameSettings(this)->IdleChannelingState));
			ChannelState->SetChanneledSkill(Skill);
			ChannelState->SetDuration(ChannelTime);

			if (Callback && Skill->IsEasyToInterrupt())
			{
				ChannelState->SetEasilyInterruptable(Callback);
			}
		}
		else
		{
			UChannelingActorState* ChannelState = Cast<UChannelingActorState>(ActorState->SetState(UCoreBlueprintLibrary::GetGameSettings(this)->MovableChannelingState));
			ChannelState->SetChanneledSkill(Skill);
			ChannelState->SetDuration(ChannelTime);

			if (Callback && Skill->IsEasyToInterrupt())
			{
				ChannelState->SetEasilyInterruptable(Callback);
			}
		}

		SyncSkillChannelBegin(Index, ChannelTime);
	}
}

void USkillComponent::ChannelSkill(USkillBase* Skill, AActor* Target, const FVector& Location)
{
	int32 Index = 0;
	if (Skills.Find(Skill, Index))
	{
		ChannelSkill(Index, Target, Location);
	}
}

void USkillComponent::NotifyOwner_Implementation(const FText& Text)
{
	if (ACoreCharacter* Character = Cast<ACoreCharacter>(GetOwner()))
	{
		Character->GetCharacterBar()->DisplayGameInfoText(Text);
	}
}

void USkillComponent::UseSkillOnLocationInternal_Implementation(uint8 Index, const FVector_NetQuantize& Location)
{
	USkillBase* Skill = Skills[Index];
	Skill->UseLocation(Location);
}

bool USkillComponent::UseSkillOnLocationInternal_Validate(uint8 Index, const FVector_NetQuantize& Location)
{
	return true;
}

void USkillComponent::CancelChanneling_Implementation()
{
	if (ChanneledSkillIndex >= 0 && ActorState)
	{
		if (UChannelingActorState* ChannelState = Cast<UChannelingActorState>(ActorState->GetState()))
		{
			NotifySkillChannelEnd(ChanneledSkillIndex, ChannelState->GetDuration(), false);
		}
	}
}

bool USkillComponent::CancelChanneling_Validate()
{
	return true;
}

void USkillComponent::AddMaintainedBuff(UBuff* Buff)
{
	if (!GetWorld()->IsServer())
		return;

	FMaintainedBuff MaintainedBuff;
	MaintainedBuff.Class = Buff->GetClass();
	MaintainedBuff.Target = Buff->GetTargetActor();
	MaintainedBuff.BuffInstanceId = Buff->GetID();

	MaintainedBuffs.Add(MaintainedBuff);
	OnMaintainedSkillsChanged();
}

void USkillComponent::RemoveMaintainedBuff(UBuff* Buff)
{
	if (!GetWorld()->IsServer())
		return;

	TSubclassOf<UBuff> Class = Buff->GetClass();
	auto Predicate = [=](const FMaintainedBuff& MaintainedBuff)
	{
		return MaintainedBuff.Class == Class && Buff->GetTargetActor() == MaintainedBuff.Target;
	};

	MaintainedBuffs.RemoveAll(Predicate);
	OnMaintainedSkillsChanged();
}

void USkillComponent::AddAdrenaline(int32 Index, int32 Adrenaline)
{
	AdrenalineLevels[Index] += Adrenaline;
	OnAdrenalineReplicated();
}

void USkillComponent::AddAdrenaline(USkillBase* Skill, int32 Adrenaline)
{
	int32 Index = 0;
	if (Skills.Find(Skill, Index))
	{
		AddAdrenaline(Index, Adrenaline);
	}
}

void USkillComponent::SetAdrenaline(int32 Index, int32 Adrenaline)
{
	AdrenalineLevels[Index] = Adrenaline;
	OnAdrenalineReplicated();
}

void USkillComponent::SetAdrenaline(USkillBase* Skill, int32 Adrenaline)
{
	int32 Index = 0;
	if (Skills.Find(Skill, Index))
	{
		SetAdrenaline(Index, Adrenaline);
	}
}

int32 USkillComponent::GetAdrenaline(const USkillBase* Skill) const
{
	for (int i = 0; i < Skills.Num(); i++)
	{
		if (Skills[i] == Skill)
		{
			return AdrenalineLevels[i];
		}
	}

	return 0;
}

void USkillComponent::SetSkillOnCooldownInternal_Implementation(const FSkillCooldownRPCData& Data)
{
	Skills[Data.Index]->SyncCooldown(Data.EndTimestamp);
}

void USkillComponent::SyncSkillChannelBegin_Implementation(uint8 SkillIndex, float Duration)
{
	OnSkillChannelBeginDel.Broadcast(this, Skills[SkillIndex], Duration);
	ChanneledSkillIndex = SkillIndex;
}

void USkillComponent::SyncSkillChannelEnd_Implementation(uint8 SkillIndex, bool bSuccess)
{
	if (GetWorld()->IsServer())
	{
		ActorState->SetState(UCoreBlueprintLibrary::GetGameSettings(this)->FreeActorState);

		if (bSuccess)
		{
			if (ChannelTarget)
			{
				Skills[SkillIndex]->UseTargetAfterChannel(ChannelTarget);
			}
			else
			{
				Skills[SkillIndex]->UseLocationAfterChannel(ChannelLocation);
			}
		}

		ChannelLocation = FVector::ZeroVector;
		ChannelTarget = nullptr;
	}

	OnSkillChannelEndDel.Broadcast(this, Skills[SkillIndex], bSuccess);
	ChanneledSkillIndex = -1;
}

void USkillComponent::SetSkillAttribute_Implementation(const FSkillAttributeRPCData& Data)
{
	auto Predicate = [&](USkillAttribute* Attribute)
	{
		return Attribute->GetClass() == Data.Class;
	};

	if (USkillAttribute** AttributePtr = Attributes.FindByPredicate(Predicate))
	{
		if (Data.Value < 0)
		{
			Attributes.Remove(*AttributePtr);
		}
		else
		{
			(*AttributePtr)->Value = Data.Value;
		}
	}
	else
	{
		if (Data.Value >= 0)
		{
			USkillAttribute* NewAttribute = NewObject<USkillAttribute>(this, Data.Class);
			Attributes.Add(NewAttribute);
		}
	}
}

void USkillComponent::RequestSkillAttributeChange_Implementation(const FSkillAttributeRPCData& Data)
{

}

bool USkillComponent::RequestSkillAttributeChange_Validate(const FSkillAttributeRPCData& Data)
{
	return true;
}

void USkillComponent::SetNetworkSkill_Implementation(const FSkillRPCData& Data)
{
	SetSkillInternal(Data.Index, Data.Class);
}

void USkillComponent::SetNetworkSkillMulti_Implementation(const TArray<FSkillRPCData>& Data)
{
	for (const FSkillRPCData& Entry : Data)
	{
		SetSkillInternal(Entry.Index, Entry.Class);
	}
}
