// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActorMetaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNameChangeSignature, UActorMetaComponent*, Component, const FText&, NewName);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COURTOFSEBELKEH_API UActorMetaComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UActorMetaComponent();

	UFUNCTION(BlueprintCallable)
		void SetName(const FText& NewName);

	UFUNCTION(BlueprintCallable)
		const FText& GetName() const { return Name; }

	UFUNCTION(BlueprintAuthorityOnly)
		void SetFaction(uint8 NewFaction);

	UFUNCTION(BlueprintCallable)
		uint8 GetFaction() const { return Faction; }

	UFUNCTION(BlueprintCallable)
		bool ShouldShowTargetPortrait() const { return bShowTargetPortrait; }

	UFUNCTION(BlueprintCallable)
		float GetTargetPreviewCameraDistance() const { return TargetPreviewCameraDistance; }

	UFUNCTION(BlueprintCallable)
		FName GetTargetPreviewCameraSocket() const { return TargetPreviewCameraSocket; }

	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveOnNameChanged(const FText& NewName);

	UPROPERTY(BlueprintAssignable)
		FNameChangeSignature OnNameChangedDel;

protected:

	UFUNCTION()
		void OnRep_Name();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	virtual void OnNameChanged(const FText& NewName);

	UPROPERTY(EditAnywhere)
		bool bShowTargetPortrait = true;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bShowTargetPortrait"))
		float TargetPreviewCameraDistance = 120.0f;
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bShowTargetPortrait"))
		FName TargetPreviewCameraSocket = "previewAnchor";

	UPROPERTY(EditAnywhere, Replicated)
		uint8 Faction;

	UPROPERTY(EditAnywhere, Replicated, ReplicatedUsing = "OnRep_Name")
		FText Name;

};
