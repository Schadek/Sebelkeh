// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryDefinitions.h"

bool FWeaponDefinition::operator==(const FWeaponDefinition& Rhs) const
{
	return MinimumDamage == Rhs.MinimumDamage
		&& MaximumDamage == Rhs.MaximumDamage
		&& Range == Rhs.Range
		&& FireRange == Rhs.FireRange
		&& bLaunchesProjectile == Rhs.bLaunchesProjectile
		&& ProjectileClass == Rhs.ProjectileClass
		&& DamageClass == Rhs.DamageClass;
}
