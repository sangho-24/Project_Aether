#include "GAS/BaseAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Gameplay/ICombatInterface.h"

UBaseAttributeSet::UBaseAttributeSet()
{
	InitCurrentHP(100.0f);
	InitMaxHP(100.0f);
	InitDefense(0.0f);
}

void UBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	// 체력 클램핑
	if (Attribute == GetCurrentHPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHP());
	}
	// 체력 퍼센트 유지
	else if (Attribute == GetMaxHPAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
		CachedHPPercent = GetCurrentHP() / GetMaxHP();
	}
	// 방어력 음수 방지
	else if (Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		SetCurrentHP(FMath::Clamp(GetCurrentHP(), 0.0f, GetMaxHP()));

		AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		if (!TargetActor) 
			return;

		// 인터페이스로 — 플레이어/몬스터 모두 동작
		ICombatInterface* CombatTarget = Cast<ICombatInterface>(TargetActor);
		if (!CombatTarget) 
			return;

		const float DamageDone = Data.EvaluatedData.Magnitude;

		if (DamageDone < 0.0f) // 데미지
		{
			CombatTarget->SpawnFloatingDamage(FMath::Abs(DamageDone), false, false);

			if (GetCurrentHP() <= 0.0f)
			{
				AActor* Killer = Data.EffectSpec.GetContext().GetInstigator();
				CombatTarget->Death(Killer);
			}
		}
		else if (DamageDone > 0.0f) // 힐
		{
			CombatTarget->SpawnFloatingDamage(DamageDone, true, false);
		}
	}
}

void UBaseAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	// 최대체력 변경 후 비율에 맞춰 현재체력 재설정
	if (Attribute == GetMaxHPAttribute())
	{
		SetCurrentHP(GetMaxHP() * CachedHPPercent);
	}
}
