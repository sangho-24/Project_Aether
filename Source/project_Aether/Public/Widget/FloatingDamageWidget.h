// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloatingDamageWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AETHER_API UFloatingDamageWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void PlayDamageTextAnimation(float DamageValue, float LifeTime, bool bIsHeal, bool bIsCritical);
};