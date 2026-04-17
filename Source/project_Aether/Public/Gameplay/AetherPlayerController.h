// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffectTypes.h"
#include "AetherPlayerController.generated.h"

class UHUDWidget;


UCLASS()
class PROJECT_AETHER_API AAetherPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Setup")
	TSubclassOf<UHUDWidget> HUDWidgetClass;
	
private:
	UPROPERTY()
	UHUDWidget* HUDWidget;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override; 
	
private:
	// 델리게이트 콜백
	void OnHPChanged(const FOnAttributeChangeData& Data);
	void OnMPChanged(const FOnAttributeChangeData& Data);
	void OnStatsChanged(const FOnAttributeChangeData& Data);
};
