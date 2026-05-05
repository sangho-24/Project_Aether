#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Gameplay/IAnimationInterface.h"
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
	
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Setup")
	TSubclassOf<UGameplayEffect> MeleeDamageEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float MeleeBaseDamage = 15.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "0.016"))
	float TraceTickRate = 0.033f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack|Debug")
	bool bDrawDebugTrace = false;

private:
	TWeakObjectPtr<AActor> AttackTarget;
	
	//노티파이 데이터
	FProjectileData ActiveProjectileData;
	FMeleeTraceData ActiveTraceData;
	
	TArray<TWeakObjectPtr<AActor>> HitActors;   // 중복 피해 방지
	FTimerHandle TraceTimerHandle;
	
	// 몽타주 재생 공통 함수
	void PlayMontage(UAnimMontage* Montage);

	// 근접 공격(트레이스) 함수
	void StartMeleeTrace();
	void StopMeleeTrace();
	UFUNCTION() void DoMeleeTrace();
	void ApplyMeleeDamage(AActor* TargetActor, const FHitResult& HitResult);
	
	// 몽타주 콜백
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageCancelled();
	// 근접 콜백
	UFUNCTION() void OnMeleeTraceStart(FGameplayEventData Payload);
	UFUNCTION() void OnMeleeTraceEnd(FGameplayEventData Payload);
	// 투사체 콜백
	UFUNCTION() void OnSpawnProjectile(FGameplayEventData Payload);
};