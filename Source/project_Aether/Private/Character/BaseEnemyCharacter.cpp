
#include "Character/BaseEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h" 
// GAS Include
#include "AbilitySystemComponent.h"
#include "GAS/EnemyAttributeSet.h"
#include "GameplayTagContainer.h"
// UI Include
#include "Actor/FloatingDamageActor.h"
// AI Include
#include "AIController.h"

ABaseEnemyCharacter::ABaseEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ===== AI 컨트롤러 자동 빙의 설정 =====
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
}

void ABaseEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
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
	}
}

void ABaseEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UAbilitySystemComponent* ABaseEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UEnemyAttributeSet* ABaseEnemyCharacter::GetEnemyAttributeSet() const
{
	return AttributeSet;
}

void ABaseEnemyCharacter::SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical)
{
	UE_LOG(LogTemp, Log, TEXT("SpawnFloatingDamage: Amount=%.0f, IsHeal=%s"), Amount, bIsHeal ? TEXT("true") : TEXT("false"));
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
	FloatingDamageActorClass, SpawnLocation,FRotator::ZeroRotator);
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

	// TODO: 사망 몽타주 재생 또는 일정 시간 뒤 Destroy() 호출
	// SetLifeSpan(3.0f);
}

