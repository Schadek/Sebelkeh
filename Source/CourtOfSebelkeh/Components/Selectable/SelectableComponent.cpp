// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectableComponent.h"
#include "GameFramework/PlayerController.h"

USelectableComponent::USelectableComponent()
{

}

bool USelectableComponent::IsValidTarget(APlayerController* Controller) const
{
	if (GetOwner() == Controller->GetPawn())
	{
		return false;
	}

	return true;
}

void USelectableComponent::OnHoverBegin(APlayerController* Controller)
{
	ReceiveOnHoverBegin(Controller);
	OnHoverBeginDel.Broadcast(this, Controller);
}

void USelectableComponent::OnHoverEnd(APlayerController* Controller)
{
	ReceiveOnHoverEnd(Controller);
	OnHoverEndDel.Broadcast(this, Controller);
}

void USelectableComponent::OnSelect(APlayerController* Controller)
{
	ReceiveOnSelected(Controller);
	OnSelectDel.Broadcast(this, Controller);
}

void USelectableComponent::OnDeselect(APlayerController* Controller)
{
	ReceiveOnDeselected(Controller);
	OnDeselectDel.Broadcast(this, Controller);
}