// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreGameState.h"
#include "CourtOfSebelkeh/Damage/DamageSystem.h"
#include "CourtOfSebelkeh/Settings/UISettings.h"
#include "CourtOfSebelkeh/Settings/GameSettings.h"

void ACoreGameState::SpawnSettingsConst() const
{
	if (!bAreSettingsSpawned)
	{
		ACoreGameState* NonConstThis = const_cast<ACoreGameState*>(this);
		NonConstThis->SpawnSettings();
	}
}

void ACoreGameState::SpawnSettings()
{
	if (GetWorld()->IsServer())
	{
		if (DamageSystemClass)
			DamageSystemInstance = NewObject<UDamageSystem>(this, DamageSystemClass);
	}

	if (UISettingsClass)
		UISettingsInstance = NewObject<UUISettings>(this, UISettingsClass);

	if (GameSettingsClass)
		GameSettingsInstance = NewObject<UGameSettings>(this, GameSettingsClass);
}

void ACoreGameState::BeginPlay()
{
	Super::BeginPlay();

	SpawnSettings();
}
