// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CourtOfSebelkeh/ActorStates/ActorStateDefinitions.h"
#include "ActorStateComponent.generated.h"

class UActorState;
class UCallbackComponent;
class ADodgeHandler;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStateChangeSignature, UActorStateComponent*, Component, UActorState*, OldState, UActorState*, NewState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COURTOFSEBELKEH_API UActorStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UActorStateComponent();

	UFUNCTION(BlueprintCallable, meta = (ComponentClass = "ActorStateComponent"), meta = (DeterminesOutputType = "NewState"))
		UActorState* SetState(TSubclassOf<UActorState> NewState, AActor* Instigator = nullptr);

	UFUNCTION(BlueprintCallable)
		UActorState* GetState() const { return State; }

	UFUNCTION(BlueprintCallable)
		TSubclassOf<UActorState> GetStateClass() const { return ReplicatedStateClass; }

	UPROPERTY(BlueprintAssignable)
		FStateChangeSignature OnStateChanged;

	void SetDodgeHandler(ADodgeHandler* Handler) { CurrentDodgeHandler = Handler; }
	ADodgeHandler* GetDodgeHandler() const { return CurrentDodgeHandler; }

protected:

	UActorState* SetStateInternal(TSubclassOf<UActorState> NewStateClass);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
		void OnRep_StateClass();
	
	UPROPERTY(Replicated, ReplicatedUsing = "OnRep_StateClass")
		TSubclassOf<UActorState> ReplicatedStateClass;

	UPROPERTY()
		UActorState* State;

	UPROPERTY()
		UCallbackComponent* CallbackComponent;

	UPROPERTY()
		ADodgeHandler* CurrentDodgeHandler;

};
