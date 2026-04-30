// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "AetherGASLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AETHER_API UAetherGASLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void ExecuteGameplayCueWithHitResult(
		AActor* Instigator,
		AActor* TargetActor,
		const FGameplayTag& CueTag,
		const FHitResult& HitResult);
};
