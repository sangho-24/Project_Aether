// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Gameplay/ICombatInterface.h"
#include "BaseEnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UEnemyAttributeSet;
class UGameplayAbility;

UCLASS()
class PROJECT_AETHER_API ABaseEnemyCharacter : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	ABaseEnemyCharacter();
	
protected:
	// ===== GAS =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UEnemyAttributeSet* AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// ===== 이동 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 400.0f;

	// ===== 상태 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// ===== GAS =====
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UEnemyAttributeSet* GetEnemyAttributeSet() const;

	// ===== ICombatInterface =====
	virtual void SpawnFloatingDamage(float Amount, bool bIsHeal) override;
	virtual void Death(AActor* Killer) override;
};
