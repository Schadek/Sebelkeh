// Fill out your copyright notice in the Description page of Project Settings.


#include "CorePlayerController.h"
#include "CourtOfSebelkeh/Characters/CoreCharacter.h"
#include "CourtOfSebelkeh/Components/Selectable/SelectableComponent.h"

ACorePlayerController::ACorePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	bAutoManageActiveCameraTarget = false;
}

void ACorePlayerController::SelectTarget(USelectableComponent* Target)
{
	USelectableComponent* OldTarget = SelectedComponent;

	if (Target->GetOwner() == GetPawn())
	{
		//Cannot select own pawn
		Target = nullptr;
	}

	SelectedComponent = Target;

	ReceiveOnSelectedTargetChanged(OldTarget, SelectedComponent);
	OnSelectedTargetChanged.Broadcast(this, OldTarget, SelectedComponent);
}

void ACorePlayerController::AcknowledgePossession(APawn* aPawn)
{
	Super::AcknowledgePossession(aPawn);

	if (ACoreCharacter* CoreCharacter = Cast<ACoreCharacter>(aPawn))
	{
		CoreCharacter->BeginPlayLocal();
	}
}

void ACorePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("SelectTarget", IE_Pressed, this, &ACorePlayerController::SelectTargetFromInput).bConsumeInput = false;
	InputComponent->BindAction("WalkToDestination", IE_Pressed, this, &ACorePlayerController::WalkToDestination).bConsumeInput = false;
}

void ACorePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateSelection();
}

void ACorePlayerController::WalkToDestination()
{
	if (bIsValidLocation)
	{
		if (ACoreCharacter* ControlledCharacter = Cast<ACoreCharacter>(GetPawn()))
		{
			ControlledCharacter->StopAutoWalk(false);
			ControlledCharacter->WalkToLocation(HoveredLocation);
		}
	}
}

void ACorePlayerController::SelectTargetFromInput()
{
	if (HoveredComponent)
	{
		SelectTarget(HoveredComponent);
	}
}

void ACorePlayerController::UpdateSelection()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit && Hit.GetActor())
	{
		HoveredLocation = Hit.Location;

		if (USelectableComponent* HitComponent = Cast<USelectableComponent>(Hit.GetActor()->GetComponentByClass(USelectableComponent::StaticClass())))
		{
			bIsValidLocation = false;

			if (HoveredComponent != HitComponent)
			{
				USelectableComponent* CurrentComponent = HoveredComponent;
				if (HoveredComponent)
				{
					//Deselect Hovered
					HoveredComponent->OnHoverEnd(this);
					HoveredComponent = nullptr;
				}

				if (HitComponent->IsValidTarget(this))
				{
					HitComponent->OnHoverBegin(this);
					HoveredComponent = HitComponent;
				}

				OnHoveredTargetChanged.Broadcast(this, CurrentComponent, HoveredComponent);
				ReceiveOnHoveredTargetChanged(CurrentComponent, HoveredComponent);
			}
		}
		else
		{
			bIsValidLocation = true;

			//Deselect Hovered
			if (HoveredComponent)
			{
				USelectableComponent* CurrentComponent = HoveredComponent;

				//Deselect Hovered
				HoveredComponent->OnHoverEnd(this);
				HoveredComponent = nullptr;

				OnHoveredTargetChanged.Broadcast(this, CurrentComponent, nullptr);
				ReceiveOnHoveredTargetChanged(CurrentComponent, nullptr);
			}
		}
	}
	else if (HoveredComponent)
	{
		USelectableComponent* CurrentComponent = HoveredComponent;

		//Deselect Hovered
		HoveredComponent->OnHoverEnd(this);
		HoveredComponent = nullptr;

		OnHoveredTargetChanged.Broadcast(this, CurrentComponent, nullptr);
		ReceiveOnHoveredTargetChanged(CurrentComponent, nullptr);
	}
}