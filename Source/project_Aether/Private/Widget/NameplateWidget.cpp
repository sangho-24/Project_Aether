#include "Widget/NameplateWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h" 

void UNameplateWidget::UpdateHP(const float CurrentHP, const float MaxHP) const
{
	if (HPProgressBar && MaxHP > 0.0f)
	{
		HPProgressBar->SetPercent(CurrentHP / MaxHP);
	}
}

void UNameplateWidget::UpdateHPText(const float CurrentHP, const float MaxHP) const
{
	if (HPText)
	{
		FNumberFormattingOptions Opts;
		Opts.UseGrouping = true;       // 천 단위 콤마
		Opts.MaximumFractionalDigits = 0; // 소수점 제거

		FText NewHPText = FText::Format(
			FText::FromString(TEXT("{0} / {1}")),
			FText::AsNumber(FMath::FloorToInt(CurrentHP), &Opts),
			FText::AsNumber(FMath::FloorToInt(MaxHP), &Opts)
		);

		HPText->SetText(NewHPText);
	}
}

void UNameplateWidget::UpdateName(const FString& InName) const
{
	if (NameText)
	{
		NameText->SetText(FText::FromString(InName));
	}
}



float UNameplateWidget::GetFadeDistance() const
{
	return FadeDistance;
}

float UNameplateWidget::GetScaleDistance() const
{
	return ScaleDistance;
}

float UNameplateWidget::GetMinScale() const
{
	return MinScale;
}
