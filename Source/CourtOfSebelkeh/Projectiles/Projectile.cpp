// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectile::SetProperties(AActor* InTargetActor, const FVector& InTargetLocation, float InMaxRange, float InRadius, float InSpeedMultiplier)
{
	TargetActor = InTargetActor;
	TargetLocation = InTargetLocation;
	MaxRange = InMaxRange;
	Radius = InRadius;
	RadiusSq = Radius * Radius;
	Speed = DefaultSpeed * InSpeedMultiplier;
}

AProjectile* AProjectile::SpawnProjectile(TSubclassOf<AProjectile> Class, AActor* Instigator, UObject* Owner, const FVector& Location)
{
	if (!Class || !Instigator)
		return nullptr;

	AProjectile* Result = Instigator->GetWorld()->SpawnActor<AProjectile>(Class, Location, FRotator());
	Result->Owner = Owner;
	Result->Instigator = Instigator;
	return Result;
}

void AProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TargetActor)
	{
		MoveTowards(TargetActor->GetActorLocation(), DeltaSeconds);
	}
	else
	{
		MoveTowards(TargetLocation, DeltaSeconds);
	}
}

void AProjectile::MoveTowards(const FVector& Target, float DeltaSeconds)
{
	const FVector CurrentLocation = GetActorLocation();
	const FVector MovementVector = (Target - CurrentLocation).GetSafeNormal();
	const FVector NewLocation = CurrentLocation + MovementVector * Speed * DeltaSeconds;
	TraveledDistance += Speed * DeltaSeconds;

	if (FVector::DistSquared(NewLocation, Target) <= RadiusSq)
	{
		OnProjectileEnd.Broadcast(this, true);
		OnProjectileEndNative.Broadcast(this, true);
		Destroy();
	}
	else if (TraveledDistance >= MaxRange)
	{
		OnProjectileEnd.Broadcast(this, false);
		OnProjectileEndNative.Broadcast(this, false);
		Destroy();
	}
	else
	{
		SetActorLocation(NewLocation);
	}
}
