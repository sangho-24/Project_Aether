#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_EnemyAttack.generated.h"

UCLASS()
class PROJECT_AETHER_API UGA_EnemyAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_EnemyAttack();
	virtual void PostInitProperties() override;
	
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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Setup")
	FGameplayTag AbilityTag;

private:
	TWeakObjectPtr<AActor> AttackTarget;
	
	// 몽타주 재생 공통 함수
	void PlayMontage(UAnimMontage* Montage);
	
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageCancelled();
	UFUNCTION() void OnSpawnProjectile(FGameplayEventData Payload);
};