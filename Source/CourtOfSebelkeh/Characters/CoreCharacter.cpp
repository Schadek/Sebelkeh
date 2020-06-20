// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CoreCharacter.h"

#include "CourtOfSebelkeh/Movement/DodgeHandler.h"
#include "CourtOfSebelkeh/Controller/CorePlayerController.h"
#include "CourtOfSebelkeh/Components/State/ActorStateComponent.h"
#include "CourtOfSebelkeh/ActorStates/ActorStateDefinitions.h"
#include "CourtOfSebelkeh/Callbacks/CallbackSystemDefinitions.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "CourtOfSebelkeh/Settings/GameSettings.h"
#include "CourtOfSebelkeh/Components/CallbackComponent.h"
#include "CourtOfSebelkeh/Meta/Group.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"

//////////////////////////////////////////////////////////////////////////
// ACoreCharacter

ACoreCharacter::ACoreCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACoreCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("CameraMode", IE_Pressed, this, &ACoreCharacter::EnableCameraMode);
	PlayerInputComponent->BindAction("CameraMode", IE_Released, this, &ACoreCharacter::DisableCameraMode);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACoreCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACoreCharacter::MoveRight);

	PlayerInputComponent->BindAction("DodgeForward", IE_Pressed, this, &ACoreCharacter::DodgeForward).bConsumeInput = false;
	PlayerInputComponent->BindAction("DodgeBackwards", IE_Pressed, this, &ACoreCharacter::DodgeBackwards).bConsumeInput = false;
	PlayerInputComponent->BindAction("DodgeLeft", IE_Pressed, this, &ACoreCharacter::DodgeLeft).bConsumeInput = false;
	PlayerInputComponent->BindAction("DodgeRight", IE_Pressed, this, &ACoreCharacter::DodgeRight).bConsumeInput = false;

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ACoreCharacter::AddCameraYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &ACoreCharacter::AddCameraPitch);
}

void ACoreCharacter::BeginPlay()
{
	Super::BeginPlay();

	ActorStateComponent = FindComponentByClass<UActorStateComponent>();
	CallbackComponent = FindComponentByClass<UCallbackComponent>();
}

void ACoreCharacter::BeginPlayLocal()
{
	CoreController = Cast<ACorePlayerController>(GetController());

	FActorSpawnParameters Params;
	Params.Owner = this;

	ThirdPersonCamera = GetWorld()->SpawnActor<AThirdPersonCamera>(UCoreBlueprintLibrary::GetGameSettings(this)->ThirdPersonCameraClass, GetActorTransform(), Params);
	ThirdPersonCamera->SetTarget(GetRootComponent());
	ThirdPersonCamera->SetCameraRotationLag(DefaultCameraRotationLag);
	CoreController->SetViewTarget(ThirdPersonCamera);
}

void ACoreCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAutoWalk();

	const FVector Location = GetActorLocation();
	if (FVector::DistSquared(Location, LastCheckedLocation) >= 4.f)
	{
		OnMovement.Broadcast(this);
		LastCheckedLocation = Location;
	}
}

void ACoreCharacter::OnAsyncPathFound(uint32 QueryId, ENavigationQueryResult::Type Result, FNavPathSharedPtr Path)
{
	if (AsyncPathIds.Remove(QueryId) > 0 && Result == ENavigationQueryResult::Success)
	{
		const TArray<FNavPathPoint>& Points = Path->GetPathPoints();
		CurrentPath.SetNum(Points.Num());

		for (int i = 0; i < Points.Num(); i++)
		{
			CurrentPath[i] = Points[i].Location;
		}

		//Skip first point
		CurrentPathPointIndex = 1;
		bIsWalkingToLocation = true;
	}
}

void ACoreCharacter::UpdateFollowTargetPath()
{
	if (FollowTarget)
	{
		WalkToLocation(FollowTarget->GetActorLocation(), FollowTargetRange);
		FollowTargetIntervalTimestamp = GetWorld()->GetTimeSeconds();
	}
}

void ACoreCharacter::WalkToLocation(const FVector& Location, float RangeForCallback)
{
	FollowTargetRange = RangeForCallback > 0.f ? RangeForCallback : AutoWalkMinDistance;

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	FNavLocation StartLocation;
	if (NavSystem->ProjectPointToNavigation(GetActorLocation(), StartLocation, FVector::OneVector * 200.f))
	{
		UCapsuleComponent* Capsule = GetCapsuleComponent();

		FNavAgentProperties AgentProperties;
		AgentProperties.AgentHeight = Capsule->GetScaledCapsuleHalfHeight() * 2.f;
		AgentProperties.AgentRadius = Capsule->GetScaledCapsuleRadius();
		AgentProperties.AgentStepHeight = GetCharacterMovement()->MaxStepHeight;
		AgentProperties.bCanWalk = true;

		FPathFindingQuery Query;
		Query.SetAllowPartialPaths(true);
		Query.SetNavAgentProperties(AgentProperties);
		Query.NavData = NavSystem->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfMissing::Create);
		Query.StartLocation = StartLocation.Location;
		Query.EndLocation = Location;
		Query.Owner = this;

		FNavPathQueryDelegate Delegate;
		Delegate.BindUObject(this, &ACoreCharacter::OnAsyncPathFound);

		AsyncPathIds.Add(NavSystem->FindPathAsync(AgentProperties, Query, Delegate, EPathFindingMode::Hierarchical));
	}
}

void ACoreCharacter::FollowActor(AActor* Target, float RangeForCallback /*= -1.f*/)
{
	FollowTarget = Target;
	FollowTargetRange = RangeForCallback > 0.f ? RangeForCallback : AutoWalkMinDistance;
	UpdateFollowTargetPath();
}

void ACoreCharacter::SetPlayerInputIgnored(bool bIsIgnored)
{
	bIsPlayerInputIgnored = bIsIgnored;
}

void ACoreCharacter::UpdateAutoWalk()
{
	if (FollowTarget)
	{
		float Now = GetWorld()->GetTimeSeconds();
		if (Now - FollowTargetIntervalTimestamp >= FollowTargetQueryInterval)
		{
			UpdateFollowTargetPath();
		}
	}

	if (bIsWalkingToLocation)
	{
		FVector NextPoint = CurrentPath[CurrentPathPointIndex];
		const FVector NextPoint2D(NextPoint.X, NextPoint.Y, 0.f);
		const FVector PawnLocation = GetActorLocation();
		const FVector PawnLocation2D(PawnLocation.X, PawnLocation.Y, 0.f);

		if (FVector::Distance(PawnLocation2D, NextPoint2D) <= FollowTargetRange && FMath::Abs(PawnLocation.Z - NextPoint.Z) <= AutoWalkMaxZDistance)
		{
			//Advance to next point
			if (CurrentPath.Num() <= ++CurrentPathPointIndex)
			{
				StopAutoWalk(true);
				return;
			}
		}

		const FVector WalkDirection = NextPoint2D - PawnLocation2D;
		CoreController->SetControlRotation(FRotationMatrix::MakeFromX(WalkDirection).Rotator());

		AddMovementInput(WalkDirection, 1.f);
	}
}

void ACoreCharacter::StopAutoWalk(bool bReachedTarget)
{
	if (bIsWalkingToLocation)
	{
		FVector LastPoint = CurrentPath.Last();
		AActor* CurrentTarget = FollowTarget;
		float CurrentRange = FollowTargetRange;

		CurrentPathPointIndex = 1;
		bIsWalkingToLocation = false;
		CurrentPath.Empty(CurrentPath.Max());
		AsyncPathIds.Empty(AsyncPathIds.Max());
		FollowTarget = nullptr;

		if (bReachedTarget)
		{
			OnFollowTargetReached.Broadcast(this, CurrentTarget, LastPoint, CurrentRange);
		}
		else
		{
			OnFollowTargetAborted.Broadcast(this, CurrentTarget, LastPoint, CurrentRange);
		}
	}
}

bool ACoreCharacter::RequestKnockDown(float Duration, AActor* RequestInstigator)
{
	if (!GetWorld()->IsServer())
		return false;

	if (ActorStateComponent)
	{
		if (UKnockdownActorState* KnockdownState = Cast<UKnockdownActorState>(ActorStateComponent->SetState(UCoreBlueprintLibrary::GetGameSettings(this)->KnockdownState, RequestInstigator)))
		{
			KnockdownState->SetDuration(Duration);
			return true;
		}
	}

	return false;
}

UGroup* ACoreCharacter::GetGroup()
{
	if (!Group)
	{
		Group = NewObject<UGroup>(this, UGroup::StaticClass());
	}

	return Group;
}

bool ACoreCharacter::CanDodge() const
{
	return true;
}

bool ACoreCharacter::Sv_Dodge_Validate(FVector Direction)
{
	return FMath::IsNearlyEqual(Direction.SizeSquared(), 1.0f, 0.05f);
}

void ACoreCharacter::Sv_Dodge_Implementation(FVector Direction)
{
	ADodgeHandler::Create(GetController(), Direction, DodgeSpeed, DodgeDistance);
}

void ACoreCharacter::Dodge(float ForwardBias, float RightBias)
{
	if (!CanDodge())
	{
		return;
	}

	float ForwardAxis = InputComponent->GetAxisValue("MoveForward") + ForwardBias;
	float RightAxis = InputComponent->GetAxisValue("MoveRight") + RightBias;

	FVector Forward = ThirdPersonCamera->GetFollowCamera()->GetForwardVector();
	Forward.Z = 0;
	Forward.Normalize();

	FVector Right = ThirdPersonCamera->GetFollowCamera()->GetRightVector();
	Right.Z = 0;
	Right.Normalize();

	FVector DodgeDirection = Forward * ForwardAxis + Right * RightAxis;
	DodgeDirection.Normalize();

	ADodgeHandler::Create(GetController(), DodgeDirection, DodgeSpeed, DodgeDistance);
	if (!GetWorld()->IsServer())
	{
		Sv_Dodge(DodgeDirection);
	}
}

void ACoreCharacter::DodgeForward()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (LastDodgeDirection == EDodgeDirection::Forward && ((CurrentTime - LastDodgeButtonPressTimestamp) <= 0.2f))
	{
		Dodge(1, 0);
	}

	LastDodgeDirection = EDodgeDirection::Forward;
	LastDodgeButtonPressTimestamp = CurrentTime;
}

void ACoreCharacter::DodgeBackwards()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (LastDodgeDirection == EDodgeDirection::Backwards && ((CurrentTime - LastDodgeButtonPressTimestamp) <= 0.2f))
	{
		Dodge(-1, 0);
	}

	LastDodgeDirection = EDodgeDirection::Backwards;
	LastDodgeButtonPressTimestamp = CurrentTime;
}

void ACoreCharacter::DodgeLeft()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (LastDodgeDirection == EDodgeDirection::Left && ((CurrentTime - LastDodgeButtonPressTimestamp) <= 0.2f))
	{
		Dodge(0, -1);
	}

	LastDodgeDirection = EDodgeDirection::Left;
	LastDodgeButtonPressTimestamp = CurrentTime;
}

void ACoreCharacter::DodgeRight()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (LastDodgeDirection == EDodgeDirection::Right && ((CurrentTime - LastDodgeButtonPressTimestamp) <= 0.2f))
	{
		Dodge(0, 1);
	}

	LastDodgeDirection = EDodgeDirection::Right;
	LastDodgeButtonPressTimestamp = CurrentTime;
}

void ACoreCharacter::AddCameraYaw(float AxisValue)
{
	if (bIsCameraModeEnabled)
	{
		ThirdPersonCamera->AddYaw(AxisValue);
	}
}

void ACoreCharacter::AddCameraPitch(float AxisValue)
{
	if (bIsCameraModeEnabled)
	{
		ThirdPersonCamera->AddPitch(AxisValue);
	}
}

void ACoreCharacter::MoveForward(float Value)
{
	const bool bIsActiveValue = Value != 0.0f;
	if ((Controller != NULL) && bIsActiveValue && !bIsPlayerInputIgnored)
	{
		Value = FMath::Max(Value, BackwardsSpeedMaxValue);

		StopAutoWalk(false);
		OnMovementInput.Broadcast(this);

		if (bIsCameraModeEnabled)
		{
			// find out which way is forward
			const FRotator Rotation = ThirdPersonCamera->GetCameraRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);

			CoreController->SetControlRotation(Rotation);
		}
		else
		{
			ThirdPersonCamera->SetYaw(GetActorRotation().Yaw);

			// find out which way is forward
			const FRotator Rotation = GetActorRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
	}
}

void ACoreCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && !bIsPlayerInputIgnored)
	{
		StopAutoWalk(false);
		OnMovementInput.Broadcast(this);

		if (bIsCameraModeEnabled)
		{
			// find out which way is right
			const FRotator Rotation = ThirdPersonCamera->GetCameraRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			// add movement in that direction
			AddMovementInput(Direction, Value);

			CoreController->SetControlRotation(Rotation);
		}
		else
		{
			ThirdPersonCamera->SetYaw(GetActorRotation().Yaw);
			AddControllerYawInput(Value);
		}
	}
}

void ACoreCharacter::EnableCameraMode()
{
	bIsCameraModeEnabled = true;
	ThirdPersonCamera->SetCameraRotationLag(CameraModeRotationLag);

	if (CoreController)
	{
		CoreController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		CoreController->SetInputMode(InputMode);
	}
}

void ACoreCharacter::DisableCameraMode()
{
	bIsCameraModeEnabled = false;
	ThirdPersonCamera->SetCameraRotationLag(DefaultCameraRotationLag);

	if (CoreController)
	{
		CoreController->bShowMouseCursor = true;

		FInputModeGameAndUI InputMode;
		CoreController->SetInputMode(InputMode);
	}
}
