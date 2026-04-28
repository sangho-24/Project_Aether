#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "IAnimationInterface.generated.h"

USTRUCT(BlueprintType)
struct FAbilitySkillData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAnimMontage> Montage = nullptr;
};

UINTERFACE(MinimalAPI, BlueprintType)
class UAnimationInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_AETHER_API IAnimationInterface
{
	GENERATED_BODY()
public:
	// AbilityTag에 대응하는 몽타주 반환. 캐릭터 BP에서 AbilityMontageMap으로 설정 (GetAbilityMontage)
	virtual FAbilitySkillData GetSkillDataForAbility(FGameplayTag AbilityTag) = 0;
	// 콤보 체인: ANS_ComboWindow가 다음 몽타주를 캐릭터에 저장/해제
	virtual void SetNextComboMontage(UAnimMontage* Montage) {}
	virtual UAnimMontage* GetNextComboMontage() const { return nullptr; }
	// 투사체 스폰 데이터 (GA_BasicSkill 뿐만 아니라 다른 스킬도 재사용 가능)
	virtual void SetNextProjectileClass(TSubclassOf<AActor> ProjectileClass) {}
	virtual void SetNextDamageMultiplier(float DamageMultiplier) {}
	virtual TSubclassOf<AActor> GetNextProjectileClass() const { return nullptr; }
	virtual float GetNextDamageMultiplier() const { return 1.0f; }
	// 투사체 스폰 소켓
	virtual void SetNextSpawnSocketName(FName SocketName) {}
	virtual FName GetNextSpawnSocketName() const { return NAME_None; }
	// 투사체 타겟 액터
	virtual AActor* GetLockedOnTarget() const { return nullptr; }
	virtual AActor* GetNearestTarget() const { return nullptr; }
	virtual void SetNearestTarget() {}
};