#include "Character/BaseEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
// GAS Include
#include "AbilitySystemComponent.h"
#include "GAS/EnemyAttributeSet.h"
#include "GameplayTagContainer.h"
// UI Include
#include "Actor/FloatingDamageActor.h"
#include "Widget/NameplateWidget.h"
#include "Components/WidgetComponent.h"
// AI Include
#include "AIController.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// ===== 이동 설정 =====
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// ===== GAS 초기화 =====
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UEnemyAttributeSet>(TEXT("AttributeSet"));

	// ===== Nameplate WidgetComponent 생성 =====
	NameplateWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameplateWidgetComponent"));
	NameplateWidgetComponent->SetupAttachment(GetMesh());
	// Screen Space로 설정하면 항상 카메라를 바라봄 TODO 이거 월드로 하면 더 자연스럽지 않을까?
	NameplateWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	NameplateWidgetComponent->SetDrawSize(FVector2D(200.f, 50.f));
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	const float CapsuleHalf = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	NameplateWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, CapsuleHalf * 2.4f));

	// GAS ActorInfo 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		// 기본 어빌리티 부여
		for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
		{
			if (AbilityClass)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, 0));
			}
		}
		// ===== HP 델리게이트 등록 =====
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			                      UBaseAttributeSet::GetCurrentHPAttribute())
		                      .AddUObject(this, &ABaseEnemyCharacter::OnHPChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			                      UBaseAttributeSet::GetMaxHPAttribute())
		                      .AddUObject(this, &ABaseEnemyCharacter::OnHPChanged);
	}

	// ===== Nameplate 위젯 초기화 =====
	if (NameplateWidgetClass)
	{
		NameplateWidgetComponent->SetWidgetClass(NameplateWidgetClass);
	}
	if (UNameplateWidget* Nameplate = Cast<UNameplateWidget>(NameplateWidgetComponent->GetWidget()))
	{
		Nameplate->UpdateName(EnemyName);
		CachedFadeDist = Nameplate->GetFadeDistance();
		CachedSacleDist = Nameplate->GetScaleDistance();
		CachedMinScale = Nameplate->GetMinScale();
		if (AttributeSet)
		{
			Nameplate->UpdateHP(AttributeSet->GetCurrentHP(), AttributeSet->GetMaxHP());
		}
	}
}

void ABaseEnemyCharacter::OnHPChanged(const FOnAttributeChangeData& Data)
{
	if (UNameplateWidget* Nameplate = Cast<UNameplateWidget>(NameplateWidgetComponent->GetWidget()))
	{
		if (AttributeSet)
		{
			Nameplate->UpdateHP(AttributeSet->GetCurrentHP(), AttributeSet->GetMaxHP());
		}
	}
}

void ABaseEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (NameplateWidgetComponent)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC || !PC->GetPawn())
			return;
	
		// 일정 거리 이상이면 숨김
		const float Dist = FVector::Dist(GetActorLocation(), PC->GetPawn()->GetActorLocation());
		const bool bShouldShow = Dist <= CachedFadeDist;
		NameplateWidgetComponent->SetVisibility(bShouldShow);
		if (bShouldShow)
		{
			// 카메라를 바라보도록 빌보드 처리
			FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();
			FRotator WidgetRotation = FRotator(CameraRotation.Pitch * -1.0f, CameraRotation.Yaw + 180.0f, 0.0f);
			NameplateWidgetComponent->SetWorldRotation(WidgetRotation);
			// 거리 기반 스케일 조절
			const float DistScale = FMath::Clamp(Dist / CachedSacleDist, CachedMinScale, 1.0f);
			NameplateWidgetComponent->SetWorldScale3D(FVector(DistScale));
		}
	}
}

UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UEnemyAttributeSet* ABaseEnemyCharacter::GetEnemyAttributeSet() const
{
	return AttributeSet;
}

bool ABaseEnemyCharacter::GetIsDead() const
{
	return bIsDead;
}

void ABaseEnemyCharacter::SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical)
{
	UE_LOG(LogTemp, Log, TEXT("SpawnFloatingDamage: Amount=%.0f, IsHeal=%s"), Amount,
	       bIsHeal ? TEXT("true") : TEXT("false"));
	if (!FloatingDamageActorClass)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, CapsuleHalfHeight);
	SpawnLocation += FVector(
		FMath::RandRange(-30.f, 30.f),
		FMath::RandRange(-30.f, 30.f),
		0.f);

	AFloatingDamageActor* DamageActor = World->SpawnActor<AFloatingDamageActor>(
		FloatingDamageActorClass, SpawnLocation, FRotator::ZeroRotator);
	if (DamageActor)
	{
		DamageActor->Initialize(Amount, bIsHeal, bIsCritical);
	}
}

void ABaseEnemyCharacter::Death(AActor* Killer)
{
	if (bIsDead) return;
	bIsDead = true;

	UE_LOG(LogTemp, Log, TEXT("[Enemy] %s died. Killer: %s"),
	       *GetName(),
	       Killer ? *Killer->GetName() : TEXT("None"));

	// AI 컨트롤러 정지
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
	}

	// 콜리전 비활성화 & 물리 활성화 (래그돌 or 단순 비활성화)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 죽는 몽타주 재생
	if (DeathMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			float MontageDuration = AnimInstance->Montage_Play(DeathMontage);
			if (MontageDuration > 0.f)
			{
				// 델리게이트 대신 타이머로 몽타주 길이만큼 대기
				GetWorldTimerManager().SetTimer(
					DeathTimerHandle,
					this,
					&ABaseEnemyCharacter::OnDeathMontageEnded,
					MontageDuration,
					false);
				return;
			}
		}
	}
	OnDeathMontageEnded();
}

void ABaseEnemyCharacter::OnDeathMontageEnded()
{
	// 네임플레이트 숨김
	if (NameplateWidgetComponent)
	{
		NameplateWidgetComponent->SetVisibility(false);
	}

	// 2초 후 Destroy
	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle, 
		this, 
		&ABaseEnemyCharacter::DestroyEnemy, 
		2.0f, 
		false);
}

void ABaseEnemyCharacter::DestroyEnemy()
{
	Destroy();
}