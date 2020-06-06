// Fill out your copyright notice in the Description page of Project Settings.

#include "DodgeHandler.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "CourtOfSebelkeh/Components/State/ActorStateComponent.h"

ADodgeHandler::ADodgeHandler()
{
	PrimaryActorTick.bCanEverTick = true;
}

ADodgeHandler* ADodgeHandler::Create(AController* TargetController, const FVector& DodgeDirection, const float Speed, const float Distance)
{
	if (!TargetController)
		return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADodgeHandler* Handler = TargetController->GetWorld()->SpawnActor<ADodgeHandler>(ADodgeHandler::StaticClass(), FTransform(), Params);
	if (Handler)
	{
		Handler->TargetController = TargetController;
		Handler->DodgeDirection = DodgeDirection.GetSafeNormal2D();
		Handler->Speed = Speed;
		Handler->RemainingDistance = Distance;

		TargetController->SetIgnoreMoveInput(true);

		if (UActorStateComponent* ActorStateComponent = Cast<UActorStateComponent>(TargetController->GetPawn()->GetComponentByClass(UActorStateComponent::StaticClass())))
		{
			if (Handler->GetWorld()->IsServer())
			{
				ActorStateComponent->SetState(UCoreBlueprintLibrary::GetGameSettings(Handler)->DodgingState);
			}

			ActorStateComponent->SetDodgeHandler(Handler);
		}
	}

	return Handler;
}

void ADodgeHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ACharacter* Character = TargetController->GetCharacter())
	{
		Character->AddMovementInput(DodgeDirection, 1.f, true);

		RemainingDistance -= DeltaTime * Speed;
		if (RemainingDistance <= 0.0f)
		{
			Destroy();
		}
	}
}

void ADodgeHandler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (TargetController)
	{
		TargetController->SetIgnoreMoveInput(false);

		if (APawn* Pawn = TargetController->GetPawn())
		{
			if (UActorStateComponent* ActorStateComponent = Cast<UActorStateComponent>(Pawn->GetComponentByClass(UActorStateComponent::StaticClass())))
			{
				ActorStateComponent->SetDodgeHandler(nullptr);

				if (GetWorld()->IsServer())
				{
					//If the state has been overwritten already, don't replace that
					if (ActorStateComponent->GetStateClass() == UCoreBlueprintLibrary::GetGameSettings(this)->DodgingState)
					{
						ActorStateComponent->SetState(UCoreBlueprintLibrary::GetGameSettings(this)->FreeActorState);
					}
				}
			}
		}
	}
}

