// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CourtOfSebelkeh/Callbacks/CallbackSystemDefinitions.h"
#include "CallbackListener.generated.h"

class UBuff;
class UCallbackComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCallbackListener : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class COURTOFSEBELKEH_API ICallbackListener
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnPreDamageDealt(const UCallbackComponent* CallbackComponent, const FDamageEventInfo& Info, FDamageEventInfo& OutInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnDamageDealt(const UCallbackComponent* CallbackComponent, const FDamageDealtEventInfo& Info);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnPreDamageReceived(const UCallbackComponent* CallbackComponent, const FDamageEventInfo& Info, FDamageEventInfo& OutInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnDamageReceived(const UCallbackComponent* CallbackComponent, const FDamageEventInfo& Info);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnEnchantmentAdded(const UCallbackComponent* CallbackComponent, const FBuffEventInfo& Info, FBuffEventInfo& OutInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnEnchantmentRemoved(const UCallbackComponent* CallbackComponent, const FBuffEventInfo& Info, FBuffEventInfo& OutInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnHexAdded(const UCallbackComponent* CallbackComponent, const FBuffEventInfo& Info, FBuffEventInfo& OutInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnHexRemoved(const UCallbackComponent* CallbackComponent, const FBuffEventInfo& Info, FBuffEventInfo& OutInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnStatChanged(const UCallbackComponent* CallbackComponent, const FStatEventInfo& Info);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnSkillOnCooldown(const UCallbackComponent* CallbackComponent, const FCooldownEventInfo& Info, FCooldownEventInfo& OutInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnSkillChannel(const UCallbackComponent* CallbackComponent, const FSkillChannelEventInfo& Info, FSkillChannelEventInfo& OutInfo);
	UFUNCTION(BlueprintNativeEvent, Category = "Callback")
		void OnPreActorStateChange(const UCallbackComponent* CallbackComponent, const FPreActorStateEventInfo& Info, FPreActorStateEventInfo& OutInfo);

};
