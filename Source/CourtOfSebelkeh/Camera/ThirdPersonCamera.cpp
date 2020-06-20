// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonCamera.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

AThirdPersonCamera::AThirdPersonCamera()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostUpdateWork;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void AThirdPersonCamera::SetTarget(USceneComponent* NewTarget)
{
	Target = NewTarget;

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		EnableInput(PlayerController);
		InputComponent->BindAxis("Zoom", this, &AThirdPersonCamera::Zoom);
	}
}

void AThirdPersonCamera::SetCameraRotationLag(float Value)
{
	CameraBoom->bEnableCameraRotationLag = Value > 0.0f;
	CameraBoom->CameraRotationLagSpeed = Value;
}

void AThirdPersonCamera::SetYaw(float NewYaw)
{
	FRotator Rotation = GetCameraBoom()->GetComponentRotation();
	Rotation.Yaw = NewYaw;
	GetCameraBoom()->SetWorldRotation(Rotation);
}

void AThirdPersonCamera::AddYaw(float YawInput)
{
	APlayerCameraManager* Manager = LocalPlayerController->PlayerCameraManager;

	FRotator BoomRotation = GetCameraBoom()->GetComponentRotation();
	BoomRotation.Yaw += YawInput * LocalPlayerController->InputYawScale;
	Manager->LimitViewYaw(BoomRotation, Manager->ViewYawMin, Manager->ViewYawMax);
	GetCameraBoom()->SetWorldRotation(BoomRotation);
}

void AThirdPersonCamera::AddPitch(float PitchInput)
{
	APlayerCameraManager* Manager = LocalPlayerController->PlayerCameraManager;

	FRotator BoomRotation = GetCameraBoom()->GetComponentRotation();
	BoomRotation.Pitch += PitchInput * LocalPlayerController->InputPitchScale;
	Manager->LimitViewPitch(BoomRotation, Manager->ViewPitchMin, Manager->ViewPitchMax);
	GetCameraBoom()->SetWorldRotation(BoomRotation);
}

FRotator AThirdPersonCamera::GetCameraRotation() const
{
	return CameraBoom->GetComponentRotation();
}

void AThirdPersonCamera::Zoom(float Value)
{
	CameraBoom->TargetArmLength += Value * ZoomSpeed;
}

void AThirdPersonCamera::BeginPlay()
{
	Super::BeginPlay();

	LocalPlayerController = UGameplayStatics::GetPlayerController(this, 0);
}

void AThirdPersonCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Target)
	{
		SetActorLocation(Target->GetComponentLocation());
	}
}

