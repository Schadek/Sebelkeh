// Fill out your copyright notice in the Description page of Project Settings.


#include "Group.h"

void UGroup::AddMember(AActor* Member)
{
	Members.Add(Member);
}

void UGroup::RemoveMember(AActor* Member)
{
	Members.Remove(Member);
}
