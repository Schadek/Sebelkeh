// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SelectableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSelectableDelegateSignature, USelectableComponent*, SelectableComponent, APlayerController*, Controller);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COURTOFSEBELKEH_API USelectableComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	USelectableComponent();

	virtual bool IsValidTarget(APlayerController* Controller) const;

	virtual void OnHoverBegin(APlayerController* Controller);
	virtual void OnHoverEnd(APlayerController* Controller);

	virtual void OnSelect(APlayerController* Controller);
	virtual void OnDeselect(APlayerController* Controller);

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnHoverBegin"))
		FSelectableDelegateSignature OnHoverBeginDel;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnHoverEnd"))
		FSelectableDelegateSignature OnHoverEndDel;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnSelect"))
		FSelectableDelegateSignature OnSelectDel;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnDeselect"))
		FSelectableDelegateSignature OnDeselectDel;

protected:

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnHoverBegin"))
		void ReceiveOnHoverBegin(APlayerController* Controller);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnHoverEnd"))
		void ReceiveOnHoverEnd(APlayerController* Controller);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnSelect"))
		void ReceiveOnSelected(APlayerController* Controller);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDeselect"))
		void ReceiveOnDeselected(APlayerController* Controller);

};
