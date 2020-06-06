// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NativeCharacterBar.generated.h"

/**
 *
 */
UCLASS()
class COURTOFSEBELKEH_API UNativeCharacterBar : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
		void DisplayGameInfoText(const FText& Text);

};
