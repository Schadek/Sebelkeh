// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Group.generated.h"

/**
 *
 */
UCLASS()
class COURTOFSEBELKEH_API UGroup : public UObject
{
	GENERATED_BODY()


public:

	UFUNCTION(BlueprintCallable)
		const TArray<AActor*> GetMembers() const { return Members; }
	UFUNCTION(BlueprintCallable)
		void AddMember(AActor* Member);
	UFUNCTION(BlueprintCallable)
		void RemoveMember(AActor* Member);

protected:

	TArray<AActor*> Members;
};
