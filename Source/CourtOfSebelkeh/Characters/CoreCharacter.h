// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CoreCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFollowTargetSignature, ACoreCharacter*, Sender, AActor*, FollowTarget, const FVector&, Location, float, Range);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementSignature, ACoreCharacter*, Sender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementInputSignature, ACoreCharacter*, Sender);

class UGroup;
class UActorStateComponent;
class UCallbackComponent;
class UNativeCharacterBar;
class ADodgeHandler;
class ACorePlayerController;
class AThirdPersonCamera;

enum class EDodgeDirection
{
	Forward,
	Backwards,
	Right,
	Left
};

UCLASS(config = Game)
class ACoreCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ACoreCharacter();

	UFUNCTION(BlueprintCallable)
		UNativeCharacterBar* GetCharacterBar() const { return HUDCharacterBar; }
	UFUNCTION(BlueprintCallable)
		void SetCharacterBar(UNativeCharacterBar* Bar) { HUDCharacterBar = Bar; }

	UFUNCTION(BlueprintCallable)
		void WalkToLocation(const FVector& Location, float RangeForCallback = -1.f);
	UFUNCTION(BlueprintCallable)
		void FollowActor(AActor* Target, float RangeForCallback = -1.f);
	UFUNCTION(BlueprintCallable)
		void StopAutoWalk(bool bReachedTarget);
	UFUNCTION(BlueprintCallable)
		bool RequestKnockDown(float Duration, AActor* RequestInstigator = nullptr);

	UFUNCTION(BlueprintPure)
		UGroup* GetGroup();

	UPROPERTY(BlueprintAssignable)
		FFollowTargetSignature OnFollowTargetReached;
	UPROPERTY(BlueprintAssignable)
		FFollowTargetSignature OnFollowTargetAborted;
	UPROPERTY(BlueprintAssignable)
		FMovementSignature OnMovement;
	UPROPERTY(BlueprintAssignable)
		FMovementInputSignature OnMovementInput;

	void SetPlayerInputIgnored(bool bIsIgnored);
	void BeginPlayLocal();

protected:

	void UpdateAutoWalk();

	void MoveForward(float Value);
	void MoveRight(float Value);

	void EnableCameraMode();
	void DisableCameraMode();

protected:

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void OnAsyncPathFound(uint32 QueryId, ENavigationQueryResult::Type Result, FNavPathSharedPtr Path);
	void UpdateFollowTargetPath();

	bool CanDodge() const;


	UFUNCTION(Server, Unreliable, WithValidation)
		void Sv_Dodge(FVector Direction);

	void Dodge(float ForwardBias, float RightBias);
	void DodgeForward();
	void DodgeBackwards();
	void DodgeLeft();
	void DodgeRight();

	void AddCameraYaw(float AxisValue);
	void AddCameraPitch(float AxisValue);

	UPROPERTY(EditDefaultsOnly)
		float DodgeSpeed = 1.0f;
	UPROPERTY(EditDefaultsOnly)
		float DodgeDistance = 200.0f;

	UPROPERTY(EditDefaultsOnly)
		float BackwardsSpeedMaxValue = -0.3f;

	UPROPERTY(EditDefaultsOnly)
		float AutoWalkMinDistance = 15.0f;
	UPROPERTY(EditDefaultsOnly)
		float AutoWalkMaxZDistance = 80.0f;
	UPROPERTY(EditDefaultsOnly)
		float FollowTargetQueryInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly)
		float DefaultCameraRotationLag = 5.f;
	UPROPERTY(EditDefaultsOnly)
		float CameraModeRotationLag = 20.f;

	UPROPERTY()
		AActor* FollowTarget;
	UPROPERTY()
		ADodgeHandler* CurrentDodgeHandler;
	UPROPERTY()
		UGroup* Group;

	UPROPERTY()
		UActorStateComponent* ActorStateComponent;
	UPROPERTY()
		UCallbackComponent* CallbackComponent;
	UPROPERTY()
		UNativeCharacterBar* HUDCharacterBar;
	UPROPERTY()
		ACorePlayerController* CoreController;
	UPROPERTY()
		AThirdPersonCamera* ThirdPersonCamera;

	bool bIsPlayerInputIgnored;
	bool bIsCameraModeEnabled;

	bool bIsWalkingToLocation;
	int32 CurrentPathPointIndex;
	TArray<FVector> CurrentPath;
	TArray<uint32> AsyncPathIds;
	float FollowTargetIntervalTimestamp;
	float FollowTargetRange;

	FVector LastCheckedLocation;

	float LastDodgeButtonPressTimestamp;
	EDodgeDirection LastDodgeDirection;

};

