// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "SkillThumbnailRenderer.generated.h"

/**
 *
 */
UCLASS()
class USkillThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

protected:

	// Begin UThumbnailRenderer Object
	COURTOFSEBELKEHEDITOR_API virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;
	COURTOFSEBELKEHEDITOR_API virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	// End UThumbnailRenderer Object
};

UCLASS()
class UBuffThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

protected:

	// Begin UThumbnailRenderer Object
	COURTOFSEBELKEHEDITOR_API virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override;
	COURTOFSEBELKEHEDITOR_API virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	// End UThumbnailRenderer Object
};
