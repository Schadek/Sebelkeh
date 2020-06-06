// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "CourtOfSebelkeh/Characters/CoreCharacter.h"
#include "CourtOfSebelkeh/Inventory/InventoryDefinitions.h"
#include "CourtOfSebelkeh/Libraries/CoreBlueprintLibrary.h"
#include "CourtOfSebelkeh/Damage/DamageSystem.h"
#include "CourtOfSebelkeh/Projectiles/Projectile.h"
#include "CourtOfSebelkeh/Components/Stats/StatComponent.h"
#include "CourtOfSebelkeh/Components/State/ActorStateComponent.h"
#include "CourtOfSebelkeh/Settings/GameSettings.h"
#include "CourtOfSebelkeh/Controller/CorePlayerController.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UInventoryComponent::SetAttackTarget(AActor* Target)
{
	if (!Target && AttackTarget)
	{
		StopAttacking();
		return false;
	}

	if (EquippedWeapons.Num() > 0)
	{
		if (ActorStateComponent)
		{
			ActorStateComponent->SetState(UCoreBlueprintLibrary::GetGameSettings(this)->AttackingState);
		}

		float WeaponRange = EquippedWeapons[0]->GetDefinition().FireRange;
		float WeaponRangeSq = FMath::Pow(WeaponRange, 2);
		AttackTarget = Target;

		if (Target->GetSquaredDistanceTo(GetOwner()) >= WeaponRangeSq)
		{
			//Out of range, try walking closer
			if (ACoreCharacter* Character = Cast<ACoreCharacter>(GetOwner()))
			{
				bIsWaitingForAttackCallback = true;
				Character->FollowActor(Target, WeaponRange);
			}
			else
			{
				return false;
			}
		}
		else if (IsLookingAtTarget())
		{
			Server_SetAttackTarget(Target);
			return true;
		}
	}

	return false;
}

void UInventoryComponent::StopAttacking()
{
	if (GetWorld()->IsServer())
	{
		Multicast_StopAttacking(AttackTarget);
	}

	AttackTarget = nullptr;
}

void UInventoryComponent::EquipWeaponByDefinition(const FWeaponDefinition& Definition)
{
	EquipWeaponInternal(Definition);
}

void UInventoryComponent::EquipWeapon(UWeapon* Weapon)
{
	EquippedWeapons.Empty();
	EquippedWeapons.Add(Weapon);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<ACoreCharacter>(GetOwner());
	if (CharacterOwner)
	{
		FScriptDelegate ReachedDelegate;
		ReachedDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UInventoryComponent, OnOwnerReachedTarget));
		CharacterOwner->OnFollowTargetReached.Add(ReachedDelegate);

		FScriptDelegate AbortedDelegate;
		AbortedDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UInventoryComponent, OnOwnerAbortedWalk));
		CharacterOwner->OnFollowTargetAborted.Add(AbortedDelegate);
	}

	ActorStateComponent = GetOwner()->FindComponentByClass<UActorStateComponent>();
}

void UInventoryComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickAttackLogic(DeltaTime);
}

void UInventoryComponent::TickAttackLogic(float DeltaSeconds)
{
	if (AttackTarget && !bIsWaitingForAttackCallback)
	{
		const float WeaponRangeSq = FMath::Pow(EquippedWeapons[0]->GetDefinition().FireRange, 2.f);

		if (AttackCooldown >= 0)
		{
			AttackCooldown = FMath::Max(0.f, AttackCooldown - DeltaSeconds * CurrentCycleAttackSpeed);

			if (AttackCooldown == 0.f && IsInAttackState() && IsLookingAtTarget() && AttackTarget->GetSquaredDistanceTo(GetOwner()) <= WeaponRangeSq)
			{
				Attack(AttackTarget, EquippedWeapons[0]);
			}
		}
		else if (IsLookingAtTarget() && AttackTarget->GetSquaredDistanceTo(GetOwner()) <= WeaponRangeSq)
		{
			Attack(AttackTarget, EquippedWeapons[0]);
		}
	}
}

void UInventoryComponent::Server_SetAttackTarget_Implementation(AActor* Target)
{
	SetAttackTargetInternal(Target);
}

bool UInventoryComponent::Server_SetAttackTarget_Validate(AActor* Target)
{
	return true;
}

void UInventoryComponent::Multicast_StopAttacking_Implementation(AActor* Target)
{
	if (AttackTarget == Target)
	{
		AttackTarget = nullptr;
	}
}

void UInventoryComponent::SetAttackTargetInternal(AActor* Target)
{
	AttackTarget = Target;
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (CharacterOwner)
	{
		CharacterOwner->OnFollowTargetReached.RemoveAll(this);
		CharacterOwner->OnFollowTargetAborted.RemoveAll(this);
	}
}

float UInventoryComponent::EvaluateAttackSpeed() const
{
	if (UStatComponent* StatComponent = GetOwner()->FindComponentByClass<UStatComponent>())
	{
		int32 Value;
		if (StatComponent->GetStatReal(EStat::AttackSpeed, Value))
		{
			return Value / 10000.f;
		}
	}

	return 1.f;
}

bool UInventoryComponent::IsInAttackState() const
{
	if (ActorStateComponent)
	{
		if (UActorState* State = ActorStateComponent->GetState())
		{
			return State->IsA<UAttackingActorState>();
		}
	}

	return false;
}

bool UInventoryComponent::IsLookingAtTarget() const
{
	FVector OwnerForward = GetOwner()->GetActorForwardVector();
	FVector ToTarget = (AttackTarget->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D();

	float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(OwnerForward, ToTarget)));
	return Angle <= LookAtAngle;
}

void UInventoryComponent::OnOwnerReachedTarget(ACoreCharacter* Character, AActor* Target, const FVector& Location, float TargetRange)
{
	if (bIsWaitingForAttackCallback)
	{
		Server_SetAttackTarget(Target);
	}

	bIsWaitingForAttackCallback = false;
}

void UInventoryComponent::OnOwnerAbortedWalk(ACoreCharacter* Character, AActor* Target, const FVector& Location, float TargetRange)
{
	if (bIsWaitingForAttackCallback)
	{
		AttackTarget = nullptr;
	}

	bIsWaitingForAttackCallback = false;
}

void UInventoryComponent::Attack_Implementation(AActor* Target, UWeapon* Weapon)
{
	const FWeaponDefinition& Definition = Weapon->GetDefinition();
	if (Definition.bLaunchesProjectile)
	{
		const FVector SpawnLocation = GetOwner()->GetActorLocation();
		if (AProjectile* Projectile = AProjectile::SpawnProjectile(Weapon->GetDefinition().ProjectileClass, GetOwner(), Weapon, SpawnLocation))
		{
			Projectile->SetProperties(Target, FVector::ZeroVector, Weapon->GetDefinition().Range, 25.f, 1.f);

			if (GetWorld()->IsServer())
			{
				auto OnHit = [=](AProjectile* Projectile, bool bHit)
				{
					if (bHit)
					{
						FDamageInfo DamageInfo;
						DamageInfo.Amount = FMath::RandRange(Weapon->GetDefinition().MinimumDamage, Weapon->GetDefinition().MaximumDamage);
						DamageInfo.Instigator = GetOwner();
						DamageInfo.Source = Weapon;
						DamageInfo.Target = Target;
						DamageInfo.Type = Weapon->GetDefinition().DamageClass;
						UCoreBlueprintLibrary::GetCoreGameState(this)->GetDamageSystem()->ProcessDamage(DamageInfo);
					}
				};
				Projectile->OnProjectileEndNative.AddLambda(OnHit);

				AttackCooldown = 2.f;
				CurrentCycleAttackSpeed = EvaluateAttackSpeed();
			}
		}
	}
	else
	{

	}
}

void UInventoryComponent::EquipWeaponInternal_Implementation(const FWeaponDefinition& Definition)
{
	UWeapon* NewWeapon = NewObject<UWeapon>(this, UWeapon::StaticClass());
	NewWeapon->SetDefinition(Definition);

	EquipWeapon(NewWeapon);
}

void UInventoryComponent::UnequipWeaponInternal(UWeapon* Weapon)
{
	EquippedWeapons.Remove(Weapon);
}

UInventoryComponent::OnRep_EquippedWeaponDefinitions()
{
	for (UWeapon* Weapon : EquippedWeapons)
	{
		UnequipWeaponInternal(Weapon);
	}

	for (const FWeaponDefinition& Definition : EquippedWeaponDefinitions)
	{
		EquipWeaponByDefinition(Definition);
	}
}
