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
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
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
		float WeaponRange = EquippedWeapons[0]->GetDefinition().FireRange;
		float WeaponRangeSq = FMath::Pow(WeaponRange, 2);
		AttackTarget = Target;

		if (Target->GetSquaredDistanceTo(GetOwner()) >= WeaponRangeSq)
		{
			//Out of range, try walking closer
			if (ACoreCharacter* Character = Cast<ACoreCharacter>(GetOwner()))
			{
				bIsWaitingForAttackCallback = true;
				Character->FollowActor(Target, WeaponRange - 20.f);
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
	if (!GetWorld()->IsServer())
	{
		return;
	}

	EquipWeaponInternal(Definition);
}

void UInventoryComponent::EquipWeaponInternal(const FWeaponDefinition& WeaponDefinition)
{
	UWeapon* NewWeapon = NewObject<UWeapon>(this, UWeapon::StaticClass());
	NewWeapon->SetDefinition(WeaponDefinition);
	EquippedWeapons.Add(NewWeapon);

	if (GetWorld()->IsServer())
	{
		EquippedWeaponDefinitions.Add(WeaponDefinition);
	}
}

void UInventoryComponent::UnequipWeaponInternal(UWeapon* Weapon)
{
	EquippedWeapons.Remove(Weapon);
	EquippedWeaponDefinitions.Remove(Weapon->GetDefinition());
}

void UInventoryComponent::OnRep_EquippedWeaponDefinitions()
{
	EquippedWeapons.Empty();

	for (const FWeaponDefinition& Definition : EquippedWeaponDefinitions)
	{
		EquipWeaponInternal(Definition);
	}
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

	SetComponentTickEnabled(GetWorld()->IsServer());
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
				Attack(AttackTarget, 0);
			}
		}
		else if (IsLookingAtTarget() && AttackTarget->GetSquaredDistanceTo(GetOwner()) <= WeaponRangeSq)
		{
			Attack(AttackTarget, 0);
		}
	}
}

void UInventoryComponent::Server_SetAttackTarget_Implementation(AActor* Target)
{
	if (ActorStateComponent)
	{
		ActorStateComponent->SetState(UCoreBlueprintLibrary::GetGameSettings(this)->AttackingState);
	}

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

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, EquippedWeaponDefinitions);
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

void UInventoryComponent::Attack_Implementation(AActor* Target, uint8 WeaponIndex)
{
	if (WeaponIndex >= EquippedWeapons.Num())
	{
		return;
	}

	const FWeaponDefinition& Definition = EquippedWeapons[WeaponIndex]->GetDefinition();
	if (Definition.bLaunchesProjectile)
	{
		const FVector SpawnLocation = GetOwner()->GetActorLocation();
		if (AProjectile* Projectile = AProjectile::SpawnProjectile(Definition.ProjectileClass, GetOwner(), EquippedWeapons[WeaponIndex], SpawnLocation))
		{
			Projectile->SetProperties(Target, FVector::ZeroVector, Definition.Range, 25.f, 1.f);

			if (GetWorld()->IsServer())
			{
				auto OnHit = [=](AProjectile* Projectile, bool bHit)
				{
					if (bHit)
					{
						FDamageInfo DamageInfo;
						DamageInfo.Amount = FMath::RandRange(Definition.MinimumDamage, Definition.MaximumDamage);
						DamageInfo.Instigator = GetOwner();
						DamageInfo.Source = EquippedWeapons[WeaponIndex];
						DamageInfo.Target = Target;
						DamageInfo.Type = Definition.DamageClass;
						DamageInfo.DamageFlags |= (1 << static_cast<int32>(EDamageFlag::AutoAttack));

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
		const FVector SpawnLocation = GetOwner()->GetActorLocation();
		if (AProjectile* Projectile = AProjectile::SpawnProjectile(Definition.ProjectileClass, GetOwner(), EquippedWeapons[WeaponIndex], SpawnLocation))
		{
			Projectile->SetProperties(Target, FVector::ZeroVector, Definition.Range, 25.f, 1.f);

			if (GetWorld()->IsServer())
			{
				auto OnHit = [=](AProjectile* Projectile, bool bHit)
				{
					if (bHit)
					{
						FDamageInfo DamageInfo;
						DamageInfo.Amount = FMath::RandRange(Definition.MinimumDamage, Definition.MaximumDamage);
						DamageInfo.Instigator = GetOwner();
						DamageInfo.Source = EquippedWeapons[WeaponIndex];
						DamageInfo.Target = Target;
						DamageInfo.Type = Definition.DamageClass;
						DamageInfo.DamageFlags |= (1 << static_cast<int32>(EDamageFlag::AutoAttack));
						DamageInfo.DamageFlags |= (1 << static_cast<int32>(EDamageFlag::Melee));

						UCoreBlueprintLibrary::GetCoreGameState(this)->GetDamageSystem()->ProcessDamage(DamageInfo);
					}
				};
				Projectile->OnProjectileEndNative.AddLambda(OnHit);

				AttackCooldown = 2.f;
				CurrentCycleAttackSpeed = EvaluateAttackSpeed();
			}
		}
	}
}