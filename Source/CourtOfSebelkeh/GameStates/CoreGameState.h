// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CoreGameState.generated.h"

class UDamageSystem;
class UUISettings;
class UGameSettings;;

UCLASS()
class COURTOFSEBELKEH_API ACoreGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		UDamageSystem* GetDamageSystem() const { SpawnSettingsConst(); return DamageSystemInstance; }

	UFUNCTION(BlueprintCallable)
		UUISettings* GetUISettings() const { SpawnSettingsConst(); return UISettingsInstance; }

	UFUNCTION(BlueprintCallable)
		UGameSettings* GetGameSettings() const { SpawnSettingsConst(); return GameSettingsInstance; }

protected:

	void SpawnSettingsConst() const;
	void SpawnSettings();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UDamageSystem> DamageSystemClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UUISettings> UISettingsClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UGameSettings> GameSettingsClass;

	UPROPERTY()
		UDamageSystem* DamageSystemInstance;

	UPROPERTY()
		UUISettings* UISettingsInstance;

	UPROPERTY()
		UGameSettings* GameSettingsInstance;

	bool bAreSettingsSpawned;

};
