// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomBlueprintThumbnailRenderer.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "UObject/ConstructorHelpers.h"
#include "CanvasItem.h"

#include "CourtOfSebelkeh/Skills/SkillBase.h"
#include "CourtOfSebelkeh/Buffs/BuffSystemDefinitions.h"

UCustomBlueprintThumbnailRenderer::UCustomBlueprintThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinder<UTexture2D> EliteSkillBorderTexture;
			ConstructorHelpers::FObjectFinder<UTexture2D> RegularSkillBorderTexture;
			ConstructorHelpers::FObjectFinder<UTexture2D> RegularBuffBorderTexture;
			ConstructorHelpers::FObjectFinder<UTexture2D> EnchantmentIconTexture;
			ConstructorHelpers::FObjectFinder<UTexture2D> HexIconTexture;
			ConstructorHelpers::FObjectFinder<UBlueprint> UISettingsBlueprint;

			FConstructorStatics()
				: EliteSkillBorderTexture(TEXT("/Game/Textures/UI/T_SkillOutlineElite"))
				, RegularSkillBorderTexture(TEXT("/Game/Textures/UI/T_SkillOutline"))
				, RegularBuffBorderTexture(TEXT("/Game/Textures/UI/T_Buff_Border"))
				, EnchantmentIconTexture(TEXT("/Game/Textures/UI/T_Enchantment"))
				, HexIconTexture(TEXT("/Game/Textures/UI/T_Hex"))
				, UISettingsBlueprint(TEXT("/Game/Blueprints/Settings/UISettingsBP"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		EliteSkillBorder = ConstructorStatics.EliteSkillBorderTexture.Object;
		RegularSkillBorder = ConstructorStatics.RegularSkillBorderTexture.Object;
		RegularBuffBorder = ConstructorStatics.RegularBuffBorderTexture.Object;
		EnchantmentIcon = ConstructorStatics.EnchantmentIconTexture.Object;
		HexIcon = ConstructorStatics.HexIconTexture.Object;
		
		if (UBlueprint* UISettingsBlueprint = ConstructorStatics.UISettingsBlueprint.Object)
		{
			UISettings = UISettingsBlueprint->GeneratedClass->GetDefaultObject<UUISettings>();
		}
	}

	CustomBlueprintDrawDelegate SkillBaseDelegate = CustomBlueprintDrawDelegate::CreateUObject(this, &UCustomBlueprintThumbnailRenderer::DrawSkillBase);
	CustomBlueprintDrawDelegate BuffDelegate = CustomBlueprintDrawDelegate::CreateUObject(this, &UCustomBlueprintThumbnailRenderer::DrawBuff);

	DrawDelegates.Add(USkillBase::StaticClass(), SkillBaseDelegate);
	DrawDelegates.Add(UBuff::StaticClass(), BuffDelegate);
}

void UCustomBlueprintThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(Object);

	if (Blueprint)
	{
		const UClass* GeneratedClass = Blueprint->GeneratedClass;
		for (auto& Pair : DrawDelegates)
		{
			if (GeneratedClass->IsChildOf(Pair.Key))
			{
				UObject* ObjectInstance = GeneratedClass->GetDefaultObject();
				Pair.Value.Execute(Pair.Key, ObjectInstance, *Canvas, X, Y, Width, Height);
				return;
			}
		}
	}

	Super::Draw(Object, X, Y, Width, Height, RenderTarget, Canvas, bAdditionalViewFamily);
}

bool UCustomBlueprintThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(Object);

	if (Blueprint)
	{
		const UClass* GeneratedClass = Blueprint->GeneratedClass;
		for (auto& Pair : DrawDelegates)
		{
			if (GeneratedClass->IsChildOf(Pair.Key))
			{
				return true;
			}
		}
	}

	return Super::CanVisualizeAsset(Object);
}

void UCustomBlueprintThumbnailRenderer::DrawCheckerBoard(FCanvas& Canvas, int32 X, int32 Y, uint32 Width, uint32 Height)
{
	const int32 CheckerDensity = 8;
	const UTexture2D* Checker = UThumbnailManager::Get().CheckerboardTexture;
	Canvas.DrawTile(
		0.0f, 0.0f, Width, Height,							// Dimensions
		0.0f, 0.0f, CheckerDensity, CheckerDensity,			// UVs
		FLinearColor::White, Checker->Resource);			// Tint & Texture
}

void UCustomBlueprintThumbnailRenderer::DrawTextureToCanvas(UTexture2D* Texture, FCanvas& Canvas, int32 X, int32 Y, uint32 Width, uint32 Height, const FLinearColor& Color, bool bForceTranslucency)
{
	if (!Texture)
		return;

	const bool bUseTranslucentBlend = (bForceTranslucency || (Texture && Texture->HasAlphaChannel() && ((Texture->LODGroup == TEXTUREGROUP_UI) || (Texture->LODGroup == TEXTUREGROUP_Pixels2D))));

	// Use A canvas tile item to draw
	FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture->Resource, FVector2D(Width, Height), Color);
	CanvasTile.BlendMode = bUseTranslucentBlend ? SE_BLEND_Translucent : SE_BLEND_Opaque;
	CanvasTile.Draw(&Canvas);

	return;
}

void UCustomBlueprintThumbnailRenderer::DrawSkillBase(UClass* Class, UObject* Object, FCanvas& Canvas, int32 X, int32 Y, uint32 Width, uint32 Height)
{
	if (USkillBase* Skill = Cast<USkillBase>(Object))
	{
		DrawCheckerBoard(Canvas, X, Y, Width, Height);
		DrawTextureToCanvas(Skill->GetIcon(), Canvas, X, Y, Width, Height, FLinearColor::White);

		if (Skill->IsElite())
		{
			DrawTextureToCanvas(EliteSkillBorder, Canvas, X, Y, Width, Height, FLinearColor::White, true);
		}
		
		DrawTextureToCanvas(RegularSkillBorder, Canvas, X, Y, Width, Height, FLinearColor::White, true);
	}
}

void UCustomBlueprintThumbnailRenderer::DrawBuff(UClass* Class, UObject* Object, FCanvas& Canvas, int32 X, int32 Y, uint32 Width, uint32 Height)
{
	if (UBuff* Buff = Cast<UBuff>(Object))
	{
		constexpr uint32 SizeMultiplier = 4;
		constexpr uint32 Margin = 16;

		DrawCheckerBoard(Canvas, X, Y, Width, Height);
		DrawTextureToCanvas(Buff->GetIcon(), Canvas, X + Margin, Y + Margin, Width - Margin * 2, Height - Margin * 2, FLinearColor::White);

		switch (Buff->GetType())
		{
		case EBuffType::Enchantment:
			DrawTextureToCanvas(RegularBuffBorder, Canvas, X, Y, Width, Height, UISettings->EnchantmentColor, true);
			DrawTextureToCanvas(EnchantmentIcon, Canvas, X, Y, EnchantmentIcon->GetSizeX() * SizeMultiplier, EnchantmentIcon->GetSizeY() * SizeMultiplier, UISettings->EnchantmentColor, true);
			break;
		case EBuffType::Hex:
			DrawTextureToCanvas(RegularBuffBorder, Canvas, X, Y, Width, Height, UISettings->HexColor, true);
			DrawTextureToCanvas(HexIcon, Canvas, X, Y, HexIcon->GetSizeX() * SizeMultiplier, HexIcon->GetSizeY() * SizeMultiplier, UISettings->HexColor, true);
			break;
		case EBuffType::Condition:
			DrawTextureToCanvas(RegularBuffBorder, Canvas, X, Y, Width, Height, UISettings->ConditionColor, true);
			break;
		default:
			DrawTextureToCanvas(RegularBuffBorder, Canvas, X, Y, Width, Height, UISettings->BaseBuffColor, true);
			break;
		}
	}
}