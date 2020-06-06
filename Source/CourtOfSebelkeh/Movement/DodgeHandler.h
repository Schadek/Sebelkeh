// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DodgeHandler.generated.h"

UCLASS()
class COURTOFSEBELKEH_API ADodgeHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ADodgeHandler();

	static ADodgeHandler* Create(class AController* TargetController, const FVector& DodgeDirection, const float Speed, const float Distance);

protected:

	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
		class AController* TargetController;

	FVector DodgeDirection;
	float Speed;
	float RemainingDistance;

};
