// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillThumbnailRenderer.h"
#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureLightProfile.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "EngineGlobals.h"
#include "Engine/TextureCube.h"
#include "Engine/Texture2DArray.h"
#include "Texture2DPreview.h"
#include "Engine/TextureRenderTargetCube.h"

#include "CubemapUnwrapUtils.h"
#include "NormalMapPreview.h"
#include "CanvasTypes.h"

#include "CourtOfSebelkeh/Skills/SkillBase.h"
#include "CourtOfSebelkeh/Buffs/BuffSystemDefinitions.h"

void USkillThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	USkillBase* Skill = Cast<USkillBase>(Object);
	UTexture2D* Texture = Skill->GetIcon();

	Super::GetThumbnailSize(Texture, Zoom, OutWidth, OutHeight);
}

void USkillThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	USkillBase* Skill = Cast<USkillBase>(Object);
	UTexture2D* Texture = Skill->GetIcon();

	Super::Draw(Texture, X, Y, Width, Height, RenderTarget, Canvas, bAdditionalViewFamily);
}

COURTOFSEBELKEHEDITOR_API void UBuffThumbnailRenderer::GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const
{
	UBuff* Buff = Cast<UBuff>(Object);
	UTexture2D* Texture = Buff->GetIcon();

	Super::GetThumbnailSize(Texture, Zoom, OutWidth, OutHeight);
}

COURTOFSEBELKEHEDITOR_API void UBuffThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UBuff* Buff = Cast<UBuff>(Object);
	UTexture2D* Texture = Buff->GetIcon();

	Super::Draw(Texture, X, Y, Width, Height, RenderTarget, Canvas, bAdditionalViewFamily);
}
