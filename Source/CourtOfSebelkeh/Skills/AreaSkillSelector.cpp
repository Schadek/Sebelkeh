// Fill out your copyright notice in the Description page of Project Settings.

#include "AreaSkillSelector.h"

//
//
//void AAreaSkillSelector::MouseDown()
//{
//	if (bHoveredLocationSet)
//	{
//		Skill->UseLocation(HoveredLocation);
//		Destroy();
//	}
//}
//
//void AAreaSkillSelector::BeginPlay()
//{
//	Super::BeginPlay();
//
//	Indicator = GetWorld()->SpawnActor<ARadialIndicator>(FMoba::GetGameInstance(this)->DefaultRadialIndicatorClass, FTransform());
//}
//
//void AAreaSkillSelector::EndPlay(const EEndPlayReason::Type EndPlayReason)
//{
//	Super::EndPlay(EndPlayReason);
//
//	if (Indicator)
//	{
//		Indicator->Destroy();
//	}
//}
//
//void AAreaSkillSelector::Tick(float DeltaSeconds)
//{
//	Super::Tick(DeltaSeconds);
//
//	FHitResult Hit;
//	LOCALCONTROLLER->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit);
//
//	if (Hit.bBlockingHit)
//	{
//		Indicator->SetActorHiddenInGame(false);
//		Indicator->SetActorLocation(Hit.Location + FVector(0, 0, 10));
//		HoveredLocation = Hit.Location;
//		bHoveredLocationSet = true;
//	}
//	else
//	{
//		bHoveredLocationSet = false;
//		Indicator->SetActorHiddenInGame(true);
//	}
//}
//
//void AAreaSkillSelector::SetSkillRadius(float _Radius)
//{
//	if (Indicator)
//	{
//		Indicator->SetRadius(_Radius);
//	}
//}