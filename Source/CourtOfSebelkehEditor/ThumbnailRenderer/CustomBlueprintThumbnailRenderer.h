// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/BlueprintThumbnailRenderer.h"
#include "CourtOfSebelkeh/Settings/UISettings.h"
#include "CustomBlueprintThumbnailRenderer.generated.h"

DECLARE_DELEGATE_SevenParams(CustomBlueprintDrawDelegate, UClass*, UObject*, FCanvas&, int32, int32, uint32, uint32);

UCLASS()
class UCustomBlueprintThumbnailRenderer : public UBlueprintThumbnailRenderer
{
	GENERATED_BODY()

protected:

	UCustomBlueprintThumbnailRenderer(const FObjectInitializer& ObjectInitializer);

	// UThumbnailRenderer implementation
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	virtual bool CanVisualizeAsset(UObject* Object) override;

protected:

	void DrawCheckerBoard(FCanvas& Canvas, int32 X, int32 Y, uint32 Width, uint32 Height);
	void DrawTextureToCanvas(UTexture2D* Texture, FCanvas& Canvas, int32 X, int32 Y, uint32 Width, uint32 Height, const FLinearColor& Color, bool bForceTranslucency = false);

	void DrawSkillBase(UClass* Class, UObject* Object, FCanvas& Canvas, int32 X, int32 Y, uint32 Width, uint32 Height);
	void DrawBuff(UClass* Class, UObject* Object, FCanvas& Canvas, int32 X, int32 Y, uint32 Width, uint32 Height);

	TMap<UClass*, CustomBlueprintDrawDelegate> DrawDelegates;

	UPROPERTY(Transient)
		UTexture2D* EliteSkillBorder;
	UPROPERTY(Transient)
		UTexture2D* RegularSkillBorder;

	//////////////////////////////////////////////////////////////////////////
	UPROPERTY(Transient)
		UTexture2D* RegularBuffBorder;
	UPROPERTY(Transient)
		UTexture2D* EnchantmentIcon;
	UPROPERTY(Transient)
		UTexture2D* HexIcon;
	UPROPERTY(Transient)
		UUISettings* UISettings;
};