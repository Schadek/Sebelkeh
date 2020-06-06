// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CourtOfSebelkeh/Interfaces/CallbackListener.h"
#include "BuffSystemDefinitions.generated.h"

class UTexture2D;
class UBuffComponent;

UENUM(BlueprintType)
enum class EBuffCallback : uint8
{
	BuffAdded,
	BuffRemoved,
	DamageDealt,
	DamageReceived,
	Moved,
	DodgedSelf,
	EnemyDodged,
	KilledEnemy,
	Died,
	SummonedEntity,
	StatChanged
};

UENUM(BlueprintType)
enum class EBuffRelevancy : uint8
{
	Server,
	Owner,
	All
};

UENUM(BlueprintType)
enum class EBuffType : uint8
{
	Enchantment,
	Hex,
	Cry,
	Stance,
	Form,
	Other,
	Count
};

UENUM(BlueprintType)
enum class EBuffEndReason : uint8
{
	Expiration,
	Removal
};

USTRUCT()
struct FAutoCallback
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
		bool bRegisterOnServer;

	UPROPERTY(EditDefaultsOnly)
		bool bRegisterOnClient;

	UPROPERTY(EditDefaultsOnly)
		ECallback Type;

};

UCLASS(Blueprintable, BlueprintType)
class COURTOFSEBELKEH_API UBuff
	: public UObject
	, public ICallbackListener
{
	GENERATED_BODY()

public:

	virtual void Begin(UBuffComponent* InBuffComponent, AActor* InTarget, AActor* InInstigator, int32 InID, float InEndTimeStamp, bool bInIsInfinite);
	virtual void Tick(float DeltaSeconds);
	virtual void End(EBuffEndReason Reason);

	virtual void Activate();
	virtual void Deactivate();

	void SetMaintenanceCosts(const TMap<EStat, int32>& Costs);

	void MarkForDestroy() { bIsDone = true; }
	bool WantsTick(float WorldTime) const;

	UFUNCTION(BlueprintCallable)
		bool IsActive() const { return bIsActive; }

	UFUNCTION(BlueprintCallable)
		void RemoveSelf();

	UFUNCTION(BlueprintCallable)
		bool IsDone() const { return bIsDone; }

	UFUNCTION(BlueprintCallable)
		UBuffComponent* GetBuffComponent() const { return BuffComponent; }

	UFUNCTION(BlueprintCallable)
		AActor* GetTargetActor() const { return Target; }

	UFUNCTION(BlueprintCallable)
		AActor* GetInstigator() const { return Instigator; }

	UFUNCTION(BlueprintCallable)
		EBuffRelevancy GetNetRelevancy() const { return NetRelevancy; }

	UFUNCTION(BlueprintCallable)
		int32 GetID() const { return ID; }

	UFUNCTION(BlueprintCallable)
		FText GetName() const { return Name; }

	UFUNCTION(BlueprintCallable)
		UTexture2D* GetIcon() const { return Icon; }

	UFUNCTION(BlueprintCallable)
		EBuffType GetType() const { return Type; }

	UFUNCTION(BlueprintCallable)
		float GetEndTimestamp() const { return EndTimeStamp; }

	UFUNCTION(BlueprintCallable)
		bool IsInfinite() const { return bIsInfinite; }

	UFUNCTION(BlueprintCallable)
		float GetRemainingDurationFraction() const;

	UFUNCTION(BlueprintCallable)
		int32 GetAttributeValue() const { return AttributeValue; }

	UFUNCTION(BlueprintCallable)
		TSubclassOf<USkillAttribute> GetAttribute() const { return Attribute; }

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Buff", meta = (DisplayName = "Begin"))
		void ReceiveBegin(AActor* _Target, AActor* _Instigator, float _Duration, bool _bIsInfinite);

	UFUNCTION(BlueprintImplementableEvent, Category = "Buff", meta = (DisplayName = "Tick"))
		void ReceiveTick(float DeltaSeconds);

	UFUNCTION(BlueprintImplementableEvent, Category = "Buff", meta = (DisplayName = "End"))
		void ReceiveEnd(EBuffEndReason Reason);

	UFUNCTION(BlueprintImplementableEvent, Category = "Buff", meta = (DisplayName = "Activate"))
		void ReceiveActivate();

	UFUNCTION(BlueprintImplementableEvent, Category = "Buff", meta = (DisplayName = "Deactivate"))
		void ReceiveDeactivate();

protected:

	virtual UWorld* GetWorld() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buff")
		FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buff")
		UTexture2D* Icon;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
		EBuffType Type;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
		EBuffRelevancy NetRelevancy;

	UPROPERTY(EditDefaultsOnly, Category = "Buff")
		TArray<FAutoCallback> AutoCallbacks;

	UPROPERTY(EditDefaultsOnly, Category = "Tick")
		bool bWantsTick;

	UPROPERTY(EditDefaultsOnly, Category = "Tick")
		float TickRate = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Tick")
		bool bTickOnBegin = true;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<USkillAttribute> Attribute;

	UPROPERTY()
		USkillComponent* SkillComponent;

	UPROPERTY()
		UBuffComponent* BuffComponent;
	UPROPERTY()
		AActor* Target;
	UPROPERTY()
		AActor* Instigator;

	TMap<EStat, int32> MaintenanceCosts;

	int32 ID;
	int32 AttributeValue;
	float BeginTimeStamp;
	float EndTimeStamp;
	float NextTickTimeStamp;
	bool bIsInfinite;
	bool bIsDone;
	bool bIsActive;
};

USTRUCT()
struct COURTOFSEBELKEH_API FBuffAddedRMIData
{
	GENERATED_BODY();

	UPROPERTY()
		int32 ID;

	UPROPERTY()
		float EndTimeStamp;

	UPROPERTY()
		TSubclassOf<UBuff> Class;

	UPROPERTY()
		AActor* Instigator;

};

USTRUCT()
struct COURTOFSEBELKEH_API FBuffRemovedRMIData
{
	GENERATED_BODY();

	UPROPERTY()
		int32 ID;

	UPROPERTY()
		EBuffEndReason Reason;

};