#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ICombatInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_AETHER_API ICombatInterface
{
	GENERATED_BODY()
public:
	// 데미지/힐 플로팅
	virtual void SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical) {}
	// 사망 처리
	virtual void Death(AActor* Killer) {}
};