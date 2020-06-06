// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CourtOfSebelkeh.h"
#include "Modules/ModuleManager.h"

#include "CourtOfSebelkeh/GameStates/CoreGameState.h"
#include "CourtOfSebelkeh/Settings/GameSettings.h"
#include "CourtOfSebelkeh/Settings/UISettings.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, CourtOfSebelkeh, "CourtOfSebelkeh" );

extern ACoreGameState* GetCoreGameState(UObject* WorldContextObject)
{
	return Cast<ACoreGameState>(WorldContextObject->GetWorld()->GetGameState());
}