// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/BaseAttributeSet.h"
#include "PlayerAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AETHER_API UPlayerAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()
	
public:
	UPlayerAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, CurrentMP)
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxMP)
	ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MagicPower)

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CurrentMP;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxMP;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MagicPower;

private:
	float CachedMPPercent = 1.0f;
};