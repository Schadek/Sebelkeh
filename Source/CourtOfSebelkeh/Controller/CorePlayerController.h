// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CorePlayerController.generated.h"

class USelectableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSelectableChangedSignature, ACorePlayerController*, Controller, USelectableComponent*, OldTarget, USelectableComponent*, NewTarget);

UCLASS()
class COURTOFSEBELKEH_API ACorePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ACorePlayerController();

	UFUNCTION(BlueprintCallable)
		USelectableComponent* GetHovered() const { return HoveredComponent; }

	UFUNCTION(BlueprintCallable)
		USelectableComponent* GetSelected() const { return SelectedComponent; }

	UFUNCTION(BlueprintCallable)
		void SelectTarget(USelectableComponent* Target);

	UPROPERTY(BlueprintAssignable)
		FSelectableChangedSignature OnHoveredTargetChanged;

	UPROPERTY(BlueprintAssignable)
		FSelectableChangedSignature OnSelectedTargetChanged;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnHoveredTargetChanged"))
		void ReceiveOnHoveredTargetChanged(USelectableComponent* OldTarget, USelectableComponent* NewTarget);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnSelectedTargetChanged"))
		void ReceiveOnSelectedTargetChanged(USelectableComponent* OldTarget, USelectableComponent* NewTarget);

	virtual void AcknowledgePossession(APawn* aPawn) override;

protected:

	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	void WalkToDestination();
	void SelectTargetFromInput();
	void UpdateSelection();

	UPROPERTY()
		USelectableComponent* HoveredComponent;

	UPROPERTY()
		USelectableComponent* SelectedComponent;

	FVector HoveredLocation;
	bool bIsValidLocation;

};
