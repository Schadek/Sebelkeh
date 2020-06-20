// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CourtOfSebelkeh/Skills/SkillDefinitions.h"
#include "CourtOfSebelkeh/Stats/StatDefinitions.h"
#include "CourtOfSebelkeh/Meta/Factions/FactionDefinitions.h"
#include "CourtOfSebelkeh/Utility/DescriptionParser.h"
#include "GameplayTagContainer.h"
#include "SkillBase.generated.h"

class UStatComponent;
class USkillComponent;
class ACharacter;
class ACoreCharacter;

UCLASS(Blueprintable, BlueprintType, meta = (DisplayThumbnail = "true"))
class COURTOFSEBELKEH_API USkillBase : public UObject
{
	friend class ASkillSelector;
	friend class USkillComponent;

	GENERATED_BODY()

public:

	USkillBase();

	virtual UWorld* GetWorld() const override;

	virtual void Begin(USkillComponent* InOwner, bool bAutoEquip);
	virtual void End();
	virtual void Tick(float DeltaSeconds);

	UFUNCTION(BlueprintCallable)
		virtual void Select();
	UFUNCTION(BlueprintCallable)
		virtual void Deselect();
	UFUNCTION(BlueprintCallable)
		virtual void NotifyGameText(const FText& Text);

	UFUNCTION(BlueprintCallable)
		UTexture2D* GetIcon() const { return Icon; }
	UFUNCTION(BlueprintCallable)
		bool IsElite() const { return bIsElite; }
	UFUNCTION(BlueprintCallable)
		int32 GetPossibleTargets() const { return PossibleTargets; }
	UFUNCTION(BlueprintCallable)
		TSubclassOf<USkillAttribute> GetAttribute() const { return Attribute; }
	UFUNCTION(BlueprintCallable, Category = "Utility")
		int32 GetAttributeValue() const;
	UFUNCTION(BlueprintCallable, Category = "Utility")
		AActor* GetSkillOwner() const;
	UFUNCTION(BlueprintCallable, Category = "Utility")
		bool IsStunnedWhileChanneling() const { return bStunnedWhileChanneling; }
	UFUNCTION(BlueprintCallable, Category = "Utility")
		float GetChannelTime() const { return ChannelTime; }
	UFUNCTION(BlueprintCallable, Category = "Utility")
		FGameplayTag GetSkillType() const { return Type; }
	UFUNCTION(BlueprintCallable, Category = "Utility")
		bool HasCostType(ESkillCost CostType) const { return Costs.Contains(CostType); }

	/**
	* Triggers once the player has selected the skill. Used e.g. for range indicators.
	* If you override either Select or Deselect make sure to call both parent implementations or none of them
	* to prevent the range indicator from staying alive beyond the targeting mode!
	* @default: Spawns a range indicator if Range > 0.0f.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Selection", meta = (DisplayName = "Select"))
		void ReceiveSelect();

	/**
	* When the skill is either executed or aborted. Undo changes done in Select here
	* If you override either Select or Deselect make sure to call both parent implementations or none of them
	* to prevent the range indicator from staying alive beyond the targeting mode!
	* @default: Despawns the spawned range indicator of Select event.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Selection", meta = (DisplayName = "Deselect"))
		void ReceiveDeselect();

	/**
	* Wrapper function for the internal methods. Sends client->server call to the controller to avoid ownership issues.
	* @param Target: The target to use this skill on
	*/
	UFUNCTION(BlueprintCallable, Category = "Activation")
		void UseTarget(AActor* Target);

	/**
	* Wrapper function for the internal methods. Sends client->server call to the controller to avoid ownership issues.
	* @param Location: The location to use this skill on
	*/
	UFUNCTION(BlueprintCallable, Category = "Activation")
		void UseLocation(FVector Location);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Description")
		FText GetFormattedSkillDescription() const;

	/**
	* Called when the local player hovers over this skill's icon. Use this for previewing range
	* or other quick-access information
	*/
	UFUNCTION(BlueprintCallable, Category = "UserInterface")
		void OnHoverEnter();

	/**
	* Called when the local player stops hovering over this skill's icon. Use this to clean up
	* potential garbage actors left by OnHover()
	*/
	UFUNCTION(BlueprintCallable, Category = "UserInterface")
		void OnHoverExit();

	/**
	* Returns the cooldown for the queried skill level. If it exceeds the length of BaseCooldown it will use the value at highest index or 0 if the array is empty
	* @return:			The base cooldown in seconds
	*/
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
		virtual float GetBaseCooldown() const;

	/** Returns the actual cooldown for this skill and its current state with cooldown reduction already calculated in
	* @return:			The cooldown with all modifiers taken into account in seconds */
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
		virtual float GetAdjustedCooldown() const;

	/** Gets the remaining cooldown of this skill. This is a client individual information */
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
		virtual float GetRemainingCooldown() const;

	/** Gets the fraction of progress for the skill. 1 = no cooldown, 0 = full cooldown */
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
		virtual float GetRemainingCooldownFraction() const;

	/**
	* Returns the ready state for the skill by taking cooldown and level into account. Extend this
	* by costs and other usability factors such as nearby players etc.
	* @param UsabilityMessage: An optional message generated when the return value is false
	* @return: If the skill is usable at the moment
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Usability")
		bool IsUsable(FText& UsabilityMessage) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Usability")
		bool CanTarget(const AActor* Target, const FVector& Location, FText& OutUsabilityMessage) const;

	UFUNCTION(BlueprintCallable, Category = "Cooldown")
		bool IsActive() const { return bIsActive; }

	/** Skills of a player-controlled hero can be selected and will be highlighted in the UI */
	UFUNCTION(BlueprintCallable, Category = "Select")
		bool IsSelected() const { return bIsSelected; }

	/** Returns the character owner of this skill */
	UFUNCTION(BlueprintCallable, Category = "Ownership")
		USkillComponent* GetSkillComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Costs")
		bool GetCost(ESkillCost CostType, int32& OutAmount) const;

	UFUNCTION(BlueprintCallable, Category = "Cooldown")
		void DisableSkill(float Duration);

	void SetOnCooldown(float CooldownOverride = -1.f);
	void SyncCooldown(float EndTimestamp);

protected:

	UFUNCTION()
		FString DescriptionParser_GetAttributeRange(const FString& ArrayIndexKeyword, const TArray<int32>& Array) const;

	UFUNCTION()
		FString DescriptionParser_GetAttribute(const FString& ArrayIndexKeyword, const TArray<int32>& Array) const;

	UFUNCTION()
		FString DescriptionParser_GetAttributeName(const FString& Keyword);

	/** Whether this skill can be interacted with in the skill bar. Usually only false for empty 'fake' skills */
	UPROPERTY(EditDefaultsOnly, Category = "Information|Internal")
		bool bIsActive = true;

	/** The name of the skill. Used by default in the title line of skill tooltips */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Information")
		FText SkillName;

	UPROPERTY(EditDefaultsOnly, Category = "Information")
		FGameplayTag Type;

	UPROPERTY(EditDefaultsOnly, Category = "Information")
		TSubclassOf<USkillAttribute> Attribute;

	UPROPERTY(EditDefaultsOnly, Category = "Information")
		bool bIsElite;

	/** General information about which stat gets consumed when using this skill */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
		TMap<ESkillCost, int32> Costs;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
		float BaseCooldown;

	/** Class of the skill selector spawned after selecting this skill. Use SetupSkillSelector event to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
		TSubclassOf<ASkillSelector> SkillSelectorTemplate;

	UPROPERTY(EditDefaultsOnly, Category = "Usage", Meta = (Bitmask, BitmaskEnum = "ESkillTarget"))
		int32 PossibleTargets;

	/** The icon displayed in the UI. Change this at runtime under special circumstances */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Information")
		UTexture2D* Icon;

	/** A color associated with this skill. Can be used for many purposes, nothing specific implemented yet */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Information")
		FLinearColor SkillColor;

	/* Logic to execute when using this skill on a target character */
	virtual void UseTargetInternal(AActor* Target);
	/* Logic to execute when using this skill at a given location */
	virtual void UseLocationInternal(FVector Location);

	virtual void UseTargetAfterChannel(AActor* Target);
	virtual void UseLocationAfterChannel(FVector Location);

	virtual void UseTargetAuthority(AActor* Target);
	virtual void UseLocationAuthority(const FVector& Location);

	UFUNCTION()
		void OnOwnerReachedTarget(ACoreCharacter* Character, AActor* Target, const FVector& Location, float TargetRange);
	UFUNCTION()
		void OnOwnerAbortedWalk(ACoreCharacter* Character, AActor* Target, const FVector& Location, float TargetRange);

	/** Notify the clients of the skill's usage. Triggers only the cosmetic use event to prevent client-side game logic. */
	UFUNCTION(NetMulticast, Unreliable)
		void UseTargetMulticast(AActor* Target, FVector_NetQuantize Location, FRotator Rotation);

	/** Notify the clients of the skill's usage. Triggers only the cosmetic use event to prevent client-side game logic */
	UFUNCTION(NetMulticast, Unreliable)
		void UseLocationMulticast(FVector Location);

	/**
	* Implement game logic here only. Be aware of the order in which you execute your skill's effect. (Damage then buff, not
	* buff then damage). Use the DamageCharacter node for easy damage dealing
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill", meta = (DisplayName = "UseOnTarget"), BlueprintAuthorityOnly)
		void ReceiveUseTarget(AActor* Target);

	/**
	* Implement game logic here only. Be aware of the order in which you execute your skill's effect. (Damage then buff, not
	* buff then damage). Use the DamageCharacter node for easy damage dealing
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill", meta = (DisplayName = "UseAtLocation"), BlueprintAuthorityOnly)
		void ReceiveUseLocation(FVector Location);

	/** Pay the costs of the skill */
	virtual void PayCost();
	virtual void PayCostSpecific(UStatComponent* StatComponent, ESkillCost CostType, int32& Amount);
	virtual bool CanPayCost() const;
	virtual bool CanPaySpecificCost(UStatComponent* StatComponent, ESkillCost CostType, int32 Amount) const;

	/** Is this skill currently selected */
	bool bIsSelected;

	/** Remaining cooldown saved on this client in seconds */
	float RemainingCooldown;

	/** The cooldown at the time of the last cooldown activation. Has to be saved to show correct cooldown overlay ratio. */
	float CurrentCooldownStart;

	/**
	* Determines if the character automatically tries to get into range of the target.
	* @UseAtLocation: Tries to get into range of the selected location and automatically uses the skill once the character is in range
	* @UseOnTarget: Follows the target character until it is in range to use the skill
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range")
		bool bFollowOutOfRangeTargets = true;

	/************************************************************************/
	/* State Variables                                                      */
	/************************************************************************/

public:

	UFUNCTION(BlueprintCallable, Category = "Range")
		float GetRange() const { return Range; }

	UFUNCTION(BlueprintNativeEvent, Category = "Range")
		float GetExplicitRange() const;

	/** Sets the range this skill has */
	void SetRange(float NewRange) { Range = NewRange; }

	UFUNCTION(BlueprintCallable)
		bool CanBeInterrupted() const { return bCanBeInterrupted; }

	UFUNCTION(BlueprintCallable)
		bool IsEasyToInterrupt() const { return bCanBeInterrupted && bIsEasyToInterrupt; }

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
		ERangePreset RangePreset = ERangePreset::Earshot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Information", meta = (MultiLine))
		FText Description;

	DescriptionParser DescriptionParser;

	/** If true a range indicator will be shown automatically when selecting this skill */
	UPROPERTY(EditDefaultsOnly, Category = "Usage")
		bool bShowRangeOnSelect = true;

	/** Values greater than 0.0 will delay the skill by set duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
		float ChannelTime = 0.25f;

	/** Whether the channeling character is stunned while channeling the spell. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
		bool bStunnedWhileChanneling;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
		bool bCanBeInterrupted = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage", meta = (EditCondition = "bCanBeInterrupted"))
		bool bIsEasyToInterrupt = false;

	UPROPERTY()
		USkillComponent* Owner;

	float Range;
	bool bIsWalkingTowardsTarget;
	bool bIsWalkingTowardsLocation;
	bool bIsEquipped;

	/** Reference to this skill's selector instance. Created after local selection by the player */
	UPROPERTY(BlueprintReadOnly, Category = "SkillSelector")
		ASkillSelector* ActiveSkillSelector;

	bool IsLocalSkill() const;
	bool IsActorInRange(AActor* Target, float DistanceMultiplier = 1.f) const;
	ACoreCharacter* GetOwnerAsCharacter() const;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

};
