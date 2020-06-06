// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThirdPersonCamera.generated.h"

UCLASS()
class COURTOFSEBELKEH_API AThirdPersonCamera : public AActor
{
	GENERATED_BODY()

public:

	AThirdPersonCamera();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void SetTarget(USceneComponent* NewTarget);
	void SetCameraRotationLag(float Value);

	void SetYaw(float NewYaw);
	void AddYaw(float YawInput);
	void AddPitch(float PitchInput);

	FRotator GetCameraRotation() const;

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY()
		APlayerController* LocalPlayerController;
	UPROPERTY()
		USceneComponent* Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

};
