// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActorStateDefinitions.generated.h"

class ACharacter;
class ACoreCharacter;
class UActorStateComponent;
class USkillComponent;
class USkillBase;

UCLASS(Blueprintable, BlueprintType)
class COURTOFSEBELKEH_API UActorState : public UObject
{
	GENERATED_BODY()

public:

	virtual void Begin(UActorStateComponent* Owner, UActorState* OldState);
	virtual void Tick(float DeltaSeconds);
	virtual void End(UActorState* NewState);

	UFUNCTION(BlueprintCallable)
		UActorStateComponent* GetActorStateComponent() const { return OwnerComponent; }

protected:

	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Begin"))
		void ReceiveBegin(UActorStateComponent* Owner, UActorState* OldState);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Tick"))
		void ReceiveTick(float DeltaSeconds);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "End"))
		void ReceiveEnd(UActorState* NewState);

	UPROPERTY()
		UActorStateComponent* OwnerComponent;

};

UCLASS(Blueprintable, BlueprintType)
class COURTOFSEBELKEH_API UChannelingActorState : public UActorState
{
	GENERATED_BODY()

public:

	void SetChanneledSkill(USkillBase* Skill) { ChanneledSkill = Skill; }
	void SetDuration(float Duration) { EndTimestamp = BeginTimestamp + Duration; }
	float GetDuration() const { return GetWorld()->GetTimeSeconds() - BeginTimestamp; }
	void EnableEndOnMove();

protected:

	virtual void Begin(UActorStateComponent* Owner, UActorState* OldState) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void End(UActorState* NewState) override;

protected:

	UFUNCTION()
		void OnMovement(ACharacter* Sender);

	UPROPERTY(EditDefaultsOnly)
		bool bEndOnMove;
	UPROPERTY()
		ACoreCharacter* Character;
	UPROPERTY()
		USkillComponent* SkillComponent;
	UPROPERTY()
		USkillBase* ChanneledSkill;

	float BeginTimestamp;
	float EndTimestamp;
	bool bHasAuthority;
	bool bNotifiedEndAlready;

};

UCLASS(Blueprintable, BlueprintType)
class COURTOFSEBELKEH_API UKnockdownActorState : public UActorState
{
	GENERATED_BODY()

public:

	void SetDuration(float Duration) { EndTimestamp = BeginTimestamp + Duration; }

protected:

	virtual void Begin(UActorStateComponent* Owner, UActorState* OldState) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void End(UActorState* NewState) override;

protected:

	float EndTimestamp;
	float BeginTimestamp;

};

UCLASS(Blueprintable, BlueprintType)
class COURTOFSEBELKEH_API UAttackingActorState : public UActorState
{
	GENERATED_BODY()

public:

	virtual void End(UActorState* NewState) override;

};