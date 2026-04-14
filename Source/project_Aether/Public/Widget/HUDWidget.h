// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UPlayerAttributeSet;

UCLASS()
class PROJECT_AETHER_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeDestruct() override;

public:
	// 체력바 업데이트 함수
	void UpdateHP(float CurrentHP, float MaxHP);
	void UpdateMP(float CurrentMP, float MaxMP);
	void UpdateStats(const UPlayerAttributeSet* AttributeSet);
	
private:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* MPProgressBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MPText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MagicPowerText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefenseText;
};
