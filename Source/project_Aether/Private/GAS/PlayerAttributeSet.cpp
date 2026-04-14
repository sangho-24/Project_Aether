
#include "GAS/PlayerAttributeSet.h"

UPlayerAttributeSet::UPlayerAttributeSet()
{
	InitCurrentMP(100.0f);
	InitMaxMP(100.0f);
	InitMagicPower(0.0f);
}

void UPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// MP 클램핑
	if (Attribute == GetCurrentMPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMP());
	}
	// 최대 MP 변경 시 비율 캐싱
	else if (Attribute == GetMaxMPAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
		CachedMPPercent = GetCurrentMP() / GetMaxMP();
	}
	// 마법 공격력 음수 방지
	else if (Attribute == GetMagicPowerAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UPlayerAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{ 
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// 최대 MP 변경 후 비율에 맞춰 현재 MP 재설정
	if (Attribute == GetMaxMPAttribute())
	{
		SetCurrentMP(GetMaxMP() * CachedMPPercent);
	}
}