
#include "Gameplay/AetherPlayerController.h"
#include "Widget/HUDWidget.h"
#include "Character/BasePlayableCharacter.h"
#include "GAS/PlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
// #include "Blueprint/UserWidget.h"

void AAetherPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AAetherPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ABasePlayableCharacter* PlayableChar = Cast<ABasePlayableCharacter>(InPawn);
	if (!PlayableChar) 
		return;

	UAbilitySystemComponent* ASC = PlayableChar->GetAbilitySystemComponent();
	UPlayerAttributeSet* AS = PlayableChar->GetPlayerAttributeSet();
	if (!ASC || !AS) 
		return;
	
	// HUD 생성
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	if (!HUDWidget) // 생성 실패(nullptr)시 탈출, 실패하면 어차피 크래시남
		return;
	
	// 델리게이트 바인딩
	ASC->GetGameplayAttributeValueChangeDelegate(
		AS->GetCurrentHPAttribute()).AddUObject(this, &AAetherPlayerController::OnHPChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(
		AS->GetMaxHPAttribute()).AddUObject(this, &AAetherPlayerController::OnHPChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(
		AS->GetCurrentMPAttribute()).AddUObject(this, &AAetherPlayerController::OnMPChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(
		AS->GetMaxMPAttribute()).AddUObject(this, &AAetherPlayerController::OnMPChanged);

	ASC->GetGameplayAttributeValueChangeDelegate(
		AS->GetMagicPowerAttribute()).AddUObject(this, &AAetherPlayerController::OnStatsChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(
		AS->GetDefenseAttribute()).AddUObject(this, &AAetherPlayerController::OnStatsChanged);

	// 초기값 업데이트
	HUDWidget->UpdateHP(AS->GetCurrentHP(), AS->GetMaxHP());
	HUDWidget->UpdateMP(AS->GetCurrentMP(), AS->GetMaxMP());
	HUDWidget->UpdateStats(AS);
}

void AAetherPlayerController::OnHPChanged(const FOnAttributeChangeData& Data)
{
	if (!HUDWidget) 
		return;
	ABasePlayableCharacter* PlayableChar = Cast<ABasePlayableCharacter>(GetPawn());
	if (UPlayerAttributeSet* AS = PlayableChar ? PlayableChar->GetPlayerAttributeSet() : nullptr)
	{
		HUDWidget->UpdateHP(AS->GetCurrentHP(), AS->GetMaxHP());
	}
}

void AAetherPlayerController::OnMPChanged(const FOnAttributeChangeData& Data)
{
	if (!HUDWidget) return;
	ABasePlayableCharacter* PlayableChar = Cast<ABasePlayableCharacter>(GetPawn());
	if (UPlayerAttributeSet* AS = PlayableChar ? PlayableChar->GetPlayerAttributeSet() : nullptr)
	{
		HUDWidget->UpdateMP(AS->GetCurrentMP(), AS->GetMaxMP());
	}
}

void AAetherPlayerController::OnStatsChanged(const FOnAttributeChangeData& Data)
{
	if (!HUDWidget) return;
	ABasePlayableCharacter* PlayableChar = Cast<ABasePlayableCharacter>(GetPawn());
	if (UPlayerAttributeSet* AS = PlayableChar ? PlayableChar->GetPlayerAttributeSet() : nullptr)
	{
		HUDWidget->UpdateStats(AS);
	}
}
