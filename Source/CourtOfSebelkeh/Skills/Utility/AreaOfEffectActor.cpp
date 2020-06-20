// Fill out your copyright notice in the Description page of Project Settings.


#include "AreaOfEffectActor.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "Components/SphereComponent.h"

AAreaOfEffectActor::AAreaOfEffectActor()
{
	SphereOverlapComponent = CreateDefaultSubobject<USphereComponent>("SphereOverlapComponent");
	SetRootComponent(SphereOverlapComponent);
}

void AAreaOfEffectActor::SetValidTargets(int32 InValidTargetsMask)
{
	ValidTargetsMask = InValidTargetsMask;
}

void AAreaOfEffectActor::SetTimeInterval(float Interval)
{
	if (Interval > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(TickHandle, this, &AAreaOfEffectActor::OnTimerTick, Interval, true);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(TickHandle);
	}
}

void AAreaOfEffectActor::SetShape(float Radius)
{
	SphereOverlapComponent->SetSphereRadius(Radius);
}

void AAreaOfEffectActor::OnTimerTick()
{
	OnNativeTick.Broadcast(this);
	OnTick.Broadcast(this);
}

bool AAreaOfEffectActor::IsActorValid(AActor* OtherActor) const
{
	if (GetOwner() == OtherActor)
	{
		return (ValidTargetsMask & (1 << static_cast<int32>(ESkillTarget::Self))) != 0;
	}

	const ESkillTarget State = UCoreBlueprintLibrary::GetActorFactionStateRelativeTo(GetOwner(), OtherActor);
	return (ValidTargetsMask & (1 << State)) != 0;
}

void AAreaOfEffectActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (IsActorValid(OtherActor))
	{
		OnNativeTargetEnter.Broadcast(this, OtherActor);
		OnTargetEnter.Broadcast(this, OtherActor);

		ValidTargetsInside.Add(OtherActor);
	}
}

void AAreaOfEffectActor::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (ValidTargetsInside.Remove(OtherActor) > 0)
	{
		OnNativeTargetExit.Broadcast(this, OtherActor);
		OnTargetExit.Broadcast(this, OtherActor);
	
		ValidTargetsInside.RemoveSwap(OtherActor);
	}
}

void AAreaOfEffectActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	const float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(TickHandle);
	if (RemainingTime <= 0.0f)
	{
		//Tick for the last time
		OnTimerTick();
	}

	GetWorld()->GetTimerManager().ClearTimer(TickHandle);
}
