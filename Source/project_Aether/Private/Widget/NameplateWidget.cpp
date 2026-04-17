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

void UNameplateWidget::UpdateName(const FString& InName) const
{
	if (NameText)
	{
		NameText->SetText(FText::FromString(InName));
	}
}

void UNameplateWidget::UpdateScale(const float Distance)
{
	// if (Distance < NearDistance || Distance > FarDistance)
	// {
	// 	return;
	// }
	// if (HPProgressBar)
	// {
	// 	float Scale = 1.0f;
	// 	if (Distance <= 1000.f)
	// 	{
	// 		Scale = FMath::GetMappedRangeValueClamped(
	// 			FVector2D(NearDistance, 1000.f),
	// 			FVector2D(MaxScale, 1.0f),
	// 			Distance);
	// 	}
	// 	else
	// 	{
	// 		Scale = FMath::GetMappedRangeValueClamped(
	// 			FVector2D(1000.f, FarDistance),
	// 			FVector2D(1.0f, MinScale),
	// 			Distance);
	// 	}
	// 	SetRenderScale(FVector2D(Scale, Scale));
	// }
}

float UNameplateWidget::GetFadeDistance() const
{
	return FadeDistance;
}
