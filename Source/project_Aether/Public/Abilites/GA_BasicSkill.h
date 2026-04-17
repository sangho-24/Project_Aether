// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BasicSkill.generated.h"


UCLASS()
class PROJECT_AETHER_API UGA_BasicSkill : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_BasicSkill();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	// 선입력 버퍼: 콤보 윈도우 전 입력 저장
	bool bSaveCombo = false;
	// 현재 콤보 윈도우 열림 여부
	bool bIsComboWindowOpen = false;
	// 콤보 전환 중 플래그 (이전 몽타주 중단 시 EndAbility 방지
	bool bComboTransitioning = false;

	// 몽타주 재생 공통 함수
	void PlayMontage(UAnimMontage* Montage);
	// 콤보 실행 시도 (다음 몽타주 없으면 무시)
	void TryNextCombo();

	// 몽타주 콜백
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageCancelled();

	// 이벤트 콜백
	UFUNCTION() void OnSpawnProjectile(FGameplayEventData Payload);
	UFUNCTION() void OnComboInput(FGameplayEventData Payload);
	UFUNCTION() void OnComboWindowOpen(FGameplayEventData Payload);
	UFUNCTION() void OnComboWindowClose(FGameplayEventData Payload);
};
