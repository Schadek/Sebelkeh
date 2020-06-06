// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CourtOfSebelkeh/Skills/SkillDefinitions.h"
#include "SkillComponent.generated.h"

class USkillBase;
class UCallbackComponent;
class UActorStateComponent;
class USkillAttribute;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSkillChangedSignature, USkillComponent*, Component, int32, Index, USkillBase*, OldSkill, USkillBase*, NewSkill);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSkillChannelBeginSignature, USkillComponent*, Component, USkillBase*, ChanneledSkill, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSkillChannelEndSignature, USkillComponent*, Component, USkillBase*, ChanneledSkill, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMaintainedSkillsChangedSignature, USkillComponent*, Component);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COURTOFSEBELKEH_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

		friend class UChannelingActorState;

public:

	USkillComponent();

	UFUNCTION(BlueprintCallable)
		void SetSkill(int32 Index, TSubclassOf<USkillBase> Class);

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSkillChanged"))
		FSkillChangedSignature OnSkillChangedDel;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSkillChannelBegin"))
		FSkillChannelBeginSignature OnSkillChannelBeginDel;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSkillChannelEnd"))
		FSkillChannelEndSignature OnSkillChannelEndDel;

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnMaintainedBuffsChangedDel"))
		FMaintainedSkillsChangedSignature OnMaintainedBuffsChangedDel;

	UFUNCTION(BlueprintCallable)
		const TArray<USkillBase*>& GetSkills() const { return Skills; }

	UFUNCTION(BlueprintCallable)
		const TArray<FMaintainedBuff>& GetMaintainedBuffs() const { return MaintainedBuffs; }

	UFUNCTION(BlueprintCallable)
		void RequestMaintainedBuffEnd(int32 BuffId);

	UFUNCTION(BlueprintCallable)
		int32 GetAttributeValue(TSubclassOf<USkillAttribute> Class) const;

	void UseSkillOnTarget(int32 Index, AActor* Target);
	void UseSkillOnTarget(USkillBase* Skill, AActor* Target);
	void UseSkillOnLocation(int32 Index, const FVector& Location);
	void UseSkillOnLocation(USkillBase* Skill, const FVector& Location);

	void SetSkillOnCooldown(int32 Index);
	void SetSkillOnCooldown(USkillBase* Skill);

	void ChannelSkill(int32 Index, AActor* Target, const FVector& Location);
	void ChannelSkill(USkillBase* Skill, AActor* Target, const FVector& Location);

	UFUNCTION(Client, Unreliable)
		void NotifyOwner(const FText& Text);

	UFUNCTION(Server, Reliable, WithValidation)
		void CancelChanneling();

	void AddMaintainedBuff(UBuff* Buff);
	void RemoveMaintainedBuff(UBuff* Buff);

	void AddAdrenaline(int32 Index, int32 Adrenaline);
	void AddAdrenaline(USkillBase* Skill, int32 Adrenaline);
	void SetAdrenaline(int32 Index, int32 Adrenaline);
	void SetAdrenaline(USkillBase* Skill, int32 Adrenaline);
	int32 GetAdrenaline(const USkillBase* Skill) const;

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnSkillChanged(int32 Index, USkillBase* OldSkill, USkillBase* NewSkill);
	virtual void NotifySkillChannelEnd(int32 Index, float Duration, bool bSuccess);
	virtual void NotifySkillChannelEnd(USkillBase* Skill, float Duration, bool bSuccess);

	UFUNCTION()
		virtual void OnMaintainedSkillsChanged();
	UFUNCTION()
		void OnAdrenalineReplicated();

	void SetSkillInternal(int32 Index, TSubclassOf<USkillBase> Class);

	void InitializeDefaultSkills();
	void InitializeDefaultAttributes();
	void InitializeAdrenaline();

	TSubclassOf<USkillBase> GetEmptySkillClass() const;

protected:

	UFUNCTION(Server, Reliable, WithValidation)
		void InternalRequestMaintainedBuffEnd(int32 BuffId);

	UFUNCTION(Server, Reliable, WithValidation)
		void UseSkillOnTargetInternal(uint8 Index, AActor* Target);
	UFUNCTION(Server, Reliable, WithValidation)
		void UseSkillOnLocationInternal(uint8 Index, const FVector_NetQuantize& Location);

	UFUNCTION(NetMulticast, Reliable)
		void SetSkillOnCooldownInternal(const FSkillCooldownRPCData& Data);
	UFUNCTION(NetMulticast, Reliable)
		void SyncSkillChannelBegin(uint8 SkillIndex, float Duration);
	UFUNCTION(NetMulticast, Reliable)
		void SyncSkillChannelEnd(uint8 SkillIndex, bool bSuccess);

	UFUNCTION(Client, Reliable)
		void SetSkillAttribute(const FSkillAttributeRPCData& Data);
	UFUNCTION(Server, WithValidation, Reliable)
		void RequestSkillAttributeChange(const FSkillAttributeRPCData& Data);

	UFUNCTION(NetMulticast, Reliable)
		void SetNetworkSkill(const FSkillRPCData& Data);

	UFUNCTION(NetMulticast, Reliable)
		void SetNetworkSkillMulti(const TArray<FSkillRPCData>& Data);

	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<USkillBase>> DefaultSkills;
	UPROPERTY(EditAnywhere)
		TArray<FDefaultAttribute> DefaultAttributes;

	UPROPERTY()
		TArray<USkillBase*> Skills;
	UPROPERTY(Replicated, ReplicatedUsing = "OnMaintainedSkillsChanged")
		TArray<FMaintainedBuff> MaintainedBuffs;
	UPROPERTY(Replicated, ReplicatedUsing = "OnAdrenalineReplicated")
		TArray<uint16> AdrenalineLevels;

	UPROPERTY()
		AActor* ChannelTarget;
	FVector ChannelLocation;
	int32 ChanneledSkillIndex;

	UPROPERTY()
		UCallbackComponent* Callback;
	UPROPERTY()
		UActorStateComponent* ActorState;

	UPROPERTY()
		TArray<USkillAttribute*> Attributes;

};
