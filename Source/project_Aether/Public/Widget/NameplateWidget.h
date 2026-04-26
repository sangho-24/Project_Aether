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
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingHPBar")
	float FadeDistance = 2000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingHPBar")
	float ScaleDistance = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FloatingHPBar")
	float MinScale = 0.3f;

public:
	void UpdateHP(const float CurrentHP, const float MaxHP) const;
	void UpdateHPText(const float CurrentHP, const float MaxHP) const;
	void UpdateName(const FString& InName) const;
	float GetFadeDistance() const;
	float GetScaleDistance() const;
	float GetMinScale() const;
};
