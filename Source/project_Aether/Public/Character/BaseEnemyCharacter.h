// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Gameplay/ICombatInterface.h"
#include "GameplayEffectTypes.h"
#include "BaseEnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UEnemyAttributeSet;
class UGameplayAbility;
class AFloatingDamageActor;
class UWidgetComponent;
class UNameplateWidget;  
class UBehaviorTree;

UCLASS()
class PROJECT_AETHER_API ABaseEnemyCharacter : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	ABaseEnemyCharacter();
	
protected:
	// ===== 이동 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 400.0f;

	// ===== 상태 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;
	
	// ===== GAS =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UEnemyAttributeSet* AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Abilities|Setup")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	// ===== 애니메이션 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Setup")
	UAnimMontage* DeathMontage;

	FTimerHandle DeathTimerHandle;
	FTimerHandle DestroyTimerHandle;
	
	// ===== UI =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Setup")
	TSubclassOf<AFloatingDamageActor> FloatingDamageActorClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* NameplateWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Setup")
	TSubclassOf<UUserWidget> NameplateWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info|Setup")
	FString EnemyName = TEXT("Enemy"); 
	
	float CachedFadeDist = 2000.f;
	float CachedSacleDist = 500.f;
	float CachedMinScale = 0.3f;
protected:
	virtual void BeginPlay() override;
	void DestroyEnemy();
	
	// ===== 콜백 =====
	void OnHPChanged(const FOnAttributeChangeData& Data);
	void OnDeathMontageEnded();

public:	
	virtual void Tick(float DeltaTime) override;
	// ===== GAS =====
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UEnemyAttributeSet* GetEnemyAttributeSet() const;
	bool GetIsDead() const;
	
	// ===== AI =====
    UPROPERTY(EditDefaultsOnly, Category = "AI|Setup")
    UBehaviorTree* BehaviorTree;

    UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
    
	// ===== ICombatInterface =====
	virtual void SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical) override;
	virtual void Death(AActor* Killer) override;
};
