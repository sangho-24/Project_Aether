#include "Widget/HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GAS/PlayerAttributeSet.h"

void UHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UHUDWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HPProgressBar && MaxHP > 0.0f)
	{
		const float Percent = CurrentHP / MaxHP;
		HPProgressBar->SetPercent(Percent);
	}
	if (HPText)
	{
		FString HPString = FString::Printf(TEXT("%.0f / %.0f"), CurrentHP, MaxHP);
		HPText->SetText(FText::FromString(HPString));
	}
}

void UHUDWidget::UpdateMP(float CurrentMP, float MaxMP)
{
	if (MPProgressBar && MaxMP > 0.0f)
	{
		const float Percent = CurrentMP / MaxMP;
		MPProgressBar->SetPercent(Percent);
	}
	if (MPText)
	{
		FString MPString = FString::Printf(TEXT("%.0f / %.0f"), CurrentMP, MaxMP);
		HPText->SetText(FText::FromString(MPString));
	}
}

void UHUDWidget::UpdateStats(const UPlayerAttributeSet* AttributeSet)
{
	if (!AttributeSet)
	{
		return;
	}

	// const float MagicPower = AttributeSet->GetMagicPower();
	const float Defense = AttributeSet->GetDefense();

	if (MagicPowerText)
	{
		// const FString MagicPowerString = FString::Printf(TEXT("%.0f"), MagicPower);
		// MagicPowerText->SetText(FText::FromString(MagicPowerString));
	}

	if (DefenseText)
	{
		const FString DefenseString = FString::Printf(TEXT("%.0f  (%.0f%%)"), Defense, Defense / (Defense + 100) * 100);
		DefenseText->SetText(FText::FromString(DefenseString));
	}
}
