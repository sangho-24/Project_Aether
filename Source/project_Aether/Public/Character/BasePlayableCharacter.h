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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;
	
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
	
	// ===== GAS =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UPlayerAttributeSet* AttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 1000.0f;
	
	// ===== 애니메이션 ===== (BP에서 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Abilities")
	TMap<FGameplayTag, FAbilitySkillData> AbilitySkillDataMap;  // 몽타주 + 투사체 한번에 관리

	// ===== UI =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<AFloatingDamageActor> FloatingDamageActorClass;
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// Enhanced Input 콜백
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);
	void ZoomInput(const FInputActionValue& Value);
	void StartZoomInterp();
	void JumpInput();
	void StopJumpingInput();

public:	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UPlayerAttributeSet* GetPlayerAttributeSet() const;
	// ===== 인터페이스 함수 =====
	// ICombatInterface
	virtual void SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical) override;
	virtual void Death(AActor* Killer) override;
	// IAnimationInterface
	virtual FAbilitySkillData GetSkillDataForAbility(FGameplayTag AbilityTag) override;

};
