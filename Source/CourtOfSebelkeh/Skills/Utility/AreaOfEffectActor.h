// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AreaOfEffectActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAreaOfEffectTickDelegate, AAreaOfEffectActor*, Area);
DECLARE_MULTICAST_DELEGATE_OneParam(FNativeAreaOfEffectTickDelegate, AAreaOfEffectActor*);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAreaOfEffectActorDelegate, AAreaOfEffectActor*, Area, AActor*, OverlappingActor);
DECLARE_MULTICAST_DELEGATE_TwoParams(FNativeAreaOfEffectActorDelegate, AAreaOfEffectActor*, AActor*);

class USphereComponent;

UCLASS()
class COURTOFSEBELKEH_API AAreaOfEffectActor : public AActor
{
	GENERATED_BODY()

public:

	AAreaOfEffectActor();

	UPROPERTY(BlueprintAssignable)
		FAreaOfEffectTickDelegate OnTick;
	UPROPERTY(BlueprintAssignable)
		FAreaOfEffectActorDelegate OnTargetEnter;
	UPROPERTY(BlueprintAssignable)
		FAreaOfEffectActorDelegate OnTargetExit;

	FNativeAreaOfEffectTickDelegate OnNativeTick;
	FNativeAreaOfEffectActorDelegate OnNativeTargetEnter;
	FNativeAreaOfEffectActorDelegate OnNativeTargetExit;

	UFUNCTION(BlueprintCallable)
		void SetValidTargets(int32 InValidTargetsMask);
	UFUNCTION(BlueprintCallable)
		void SetTimeInterval(float Interval);
	UFUNCTION(BlueprintCallable)
		void SetShape(float Radius);
	UFUNCTION(BlueprintCallable)
	const TArray<AActor*>& GetValidTargetsInside() const { return ValidTargetsInside; }

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USphereComponent* SphereOverlapComponent;

	void OnTimerTick();
	bool IsActorValid(AActor* OtherActor) const;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor);
	virtual void NotifyActorEndOverlap(AActor* OtherActor);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	float TickInterval;
	FTimerHandle TickHandle;
	int32 ValidTargetsMask;

	TArray<AActor*> ValidTargetsInside;

};
