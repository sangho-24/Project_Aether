
#include "Character/FloatingDamageActor.h"
#include "Components/WidgetComponent.h"
#include "Widget/FloatingDamageWidget.h"

AFloatingDamageActor::AFloatingDamageActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DamageWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	RootComponent = DamageWidgetComponent;
	DamageWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	DamageWidgetComponent->SetDrawAtDesiredSize(true);
}

// Called when the game starts or when spawned
void AFloatingDamageActor::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime);
}

void AFloatingDamageActor::Initialize(float DamageAmount, bool bIsHeal, bool bIsCritical)
{
	if (UFloatingDamageWidget* DamageWidget = Cast<UFloatingDamageWidget>(DamageWidgetComponent->GetUserWidgetObject()))
	{
		DamageWidget->PlayDamageTextAnimation(DamageAmount, LifeTime, bIsHeal, bIsCritical);
	}
}

