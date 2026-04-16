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

	// 투사체 없는 스킬은 비워둠
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<AActor> ProjectileClass = nullptr;
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
};