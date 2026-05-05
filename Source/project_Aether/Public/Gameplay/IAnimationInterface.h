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

USTRUCT(BlueprintType)
struct FMeleeTraceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StartSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EndSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceRadius = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExtraLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag HitCueTag;
};

USTRUCT(BlueprintType)
struct FProjectileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SpawnSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;
	
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

	// 타겟 액터
	virtual AActor* GetLockedOnTarget() const { return nullptr; }
	virtual AActor* GetNearestTarget() const { return nullptr; }
	virtual void SetNearestTarget() {}
	// 투사체
	virtual void SetProjectileData(const FProjectileData& Data) {}
	virtual FProjectileData GetProjectileData() const { return FProjectileData(); }
	// 근접 트레이스
	virtual void SetMeleeTraceData(const FMeleeTraceData& Data) {}
	virtual FMeleeTraceData GetMeleeTraceData() const { return FMeleeTraceData(); }
};