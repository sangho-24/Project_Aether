// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "Gameplay/ICombatInterface.h"
#include "Gameplay/IAnimationInterface.h"
#include "BasePlayableCharacter.generated.h"

class UInputComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class UAbilitySystemComponent;
class UPlayerAttributeSet;
class UGameplayAbility;
class AFloatingDamageActor;
class ABaseEnemyCharacter;

UCLASS()
class PROJECT_AETHER_API ABasePlayableCharacter 
	: public ACharacter, public IAbilitySystemInterface, 
		public ICombatInterface, public IAnimationInterface 
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABasePlayableCharacter();

protected:
	// ===== 향상된 입력 =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	UInputMappingContext* DefaultInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	UInputAction* MoveInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	UInputAction* LookInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	UInputAction* ZoomInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	UInputAction* LockOnInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	UInputAction* JumpInput;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Setup")
	UInputAction* BasicSkillInput;
	
	// ===== 카메라 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;
	
	// ===== 카메라 줌 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float MinArmLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float MaxArmLength = 600.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Zoom")
	float DesiredArmLength = 400.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomStep = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Zoom")
	float ZoomInterpSpeed = 12.0f;
	
	bool bIsZoomInterpolating = false;
	
	// ===== 락온 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float LockOnRadius = 1500.0f;           // 락온 탐색 반경

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float LockOnInterpSpeed = 10.0f; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<ABaseEnemyCharacter> LockOnTarget = nullptr;  // 현재 락온 타겟

	bool bIsLockedOn = false;
	
	// ===== GAS =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UPlayerAttributeSet* AttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Abilities|Setup")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 1000.0f;
	
	// ===== 애니메이션 ===== (BP에서 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Abilities|Setup")
	TMap<FGameplayTag, FAbilitySkillData> AbilitySkillDataMap;  // 몽타주 + 투사체 한번에 관리

	UPROPERTY()
	TObjectPtr<UAnimMontage> NextComboMontage = nullptr;
	
	UPROPERTY()
	TSubclassOf<AActor> NextProjectileClass;
	
	UPROPERTY()
	FName NextSpawnSocketName;
	
	float NextDamageMultiplier = 1.0f;
	
	// ===== UI =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Setup")
	TSubclassOf<AFloatingDamageActor> FloatingDamageActorClass;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// Enhanced Input 콜백
	void MoveAction(const FInputActionValue& Value);
	void LookAction(const FInputActionValue& Value);
	void ZoomAction(const FInputActionValue& Value);
	void StartZoomInterp();
	void JumpAction();
	void StopJumpingAction();
	void BasicSkillAction();
	void LockOnAction();
	ABaseEnemyCharacter* FindLockOnTarget();

public:	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UPlayerAttributeSet* GetPlayerAttributeSet() const;
	
	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLockedOn() const { return bIsLockedOn; }

	UFUNCTION(BlueprintPure, Category = "LockOn")
	ABaseEnemyCharacter* GetLockOnTarget() const { return LockOnTarget; }
	
	// ===== 인터페이스 함수 =====
	// ICombatInterface
	virtual void SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical) override;
	virtual void Death(AActor* Killer) override;
	
	// IAnimationInterface
	virtual FAbilitySkillData GetSkillDataForAbility(FGameplayTag AbilityTag) override;
	virtual void SetNextComboMontage(UAnimMontage* Montage) override;
	virtual UAnimMontage* GetNextComboMontage() const override;
	virtual void SetNextProjectileClass(TSubclassOf<AActor> ProjectileClass) override;
	virtual void SetNextDamageMultiplier(float DamageMultiplier) override;
	virtual TSubclassOf<AActor> GetNextProjectileClass() const override;
	virtual float GetNextDamageMultiplier() const override;
	virtual void SetNextSpawnSocketName(FName SocketName) override;
	virtual FName GetNextSpawnSocketName() const override;
	virtual AActor* GetLockedOnTarget() const override;
};
