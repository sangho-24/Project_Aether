// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NameplateWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class PROJECT_AETHER_API UNameplateWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPProgressBar;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingHPBar")
	float NearDistance = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingHPBar")
	float FarDistance = 3000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingHPBar")
	float MinScale = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingHPBar")
	float MaxScale = 1.5f;

public:
	void UpdateHP(const float CurrentHP, const float MaxHP) const;
	void UpdateScale(const float Distance);
};
