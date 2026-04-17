// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingDamageActor.generated.h"

class UWidgetComponent;

UCLASS()
class PROJECT_AETHER_API AFloatingDamageActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AFloatingDamageActor();
	
	void Initialize(float DamageAmount, bool bIsHeal = false, bool bIsCritical = false);

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float LifeTime = 1.5f;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Setup")
	UWidgetComponent* DamageWidgetComponent;

protected:
	virtual void BeginPlay() override;

};
