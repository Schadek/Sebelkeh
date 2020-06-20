// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FProjectileDelegate, AProjectile*, Projectile, bool, bHitTarget);
DECLARE_MULTICAST_DELEGATE_TwoParams(FNativeProjectileDelegate, AProjectile*, bool);

UCLASS()
class COURTOFSEBELKEH_API AProjectile : public AActor
{
	GENERATED_BODY()

public:

	AProjectile();

	FNativeProjectileDelegate OnProjectileEndNative;

	UPROPERTY(BlueprintAssignable)
		FProjectileDelegate OnProjectileEnd;

	UFUNCTION(BlueprintCallable)
		UObject* GetProjectileOwner() const { return Owner; }

	UFUNCTION(BlueprintCallable)
		AActor* GetProjectileInstigator() const { return Instigator; }

	UFUNCTION(BlueprintCallable)
		AActor* GetProjectileTargetActor() const { return TargetActor; }

	UFUNCTION(BlueprintCallable)
		const FVector& GetProjectileTargetLocation() const { return TargetLocation; }

	UFUNCTION(BlueprintCallable)
		void SetProperties(AActor* InTargetActor, const FVector& InTargetLocation, float InMaxRange, float InRadius, float InSpeedMultiplier);
	
	static AProjectile* SpawnProjectile(TSubclassOf<AProjectile> Class, AActor* Instigator, UObject* Owner, const FVector& Location);

protected:

	UPROPERTY(EditDefaultsOnly)
		float DefaultSpeed = 500.0f;

	virtual void Tick(float DeltaSeconds) override;

	void MoveTowards(const FVector& Target, float DeltaSeconds);

	AActor* Instigator;
	UObject* Owner;
	AActor* TargetActor;
	FVector TargetLocation;
	float MaxRange;
	float Radius;
	float RadiusSq;
	float Speed;

	float TraveledDistance;

};
