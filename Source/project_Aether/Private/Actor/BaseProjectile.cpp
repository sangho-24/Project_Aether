#include "Actor/BaseProjectile.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "GAS/BaseAttributeSet.h"
#include "Utility/AetherGASLibrary.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 콜리전 컴포넌트
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	CollisionComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComponent;

	// 메시 컴포넌트
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 나이아가라 컴포넌트
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(CollisionComponent);
	NiagaraComponent->bAutoActivate = true;

	// 프로젝타일 무브먼트 컴포넌트
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// 수명
	InitialLifeSpan = LifeSpan;
}

void ABaseProjectile::InitProjectile(UAbilitySystemComponent* InSourceASC, float InDamageMultiplier)
{
	SourceASC = InSourceASC;
	DamageMultiplier = InDamageMultiplier;
}

// Called when the game starts or when spawned
void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	// if (ProjectileMovement)
	// {
	// 	if (!ProjectileMovement->Velocity.IsNearlyZero())
	// 	{
	// 		ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * Speed;
	// 	}
	// }
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnOverlapBegin);
		if (AActor* OwnerActor = GetOwner())
		{
			CollisionComponent->IgnoreActorWhenMoving(OwnerActor, true);
		}
	}
}

void ABaseProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	UAetherGASLibrary::ExecuteGameplayCueWithHitResult(
		GetOwner(), OtherActor, HitCueTag, Hit);
	Destroy();
}

void ABaseProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                     const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	// GAS 데미지 적용
	if (OtherActor && DamageEffect)
	{
		// 대상 ASC 가져오기
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			// Context 생성(공격자 ASC우선, 없으면 타겟ASC에 생성)
			FGameplayEffectContextHandle ContextHandle = SourceASC
				                                             ? SourceASC->MakeEffectContext()
				                                             : TargetASC->MakeEffectContext();
			ContextHandle.AddHitResult(SweepResult);
			ContextHandle.AddInstigator(GetOwner(), this);

			// Spec 생성
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				float FinalDamage = BaseDamage;
				if (SourceASC)
				{
					const UBaseAttributeSet* SourceAttributeSet = SourceASC->GetSet<UBaseAttributeSet>();
					if (SourceAttributeSet)
					{
						FinalDamage *= (1.0f + SourceAttributeSet->GetAttackPower() / 100.0f);
					}
				}
				FinalDamage *= DamageMultiplier;
				FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, -FinalDamage);

				// GE 적용
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	// Gameplay Cue 실행
	UAetherGASLibrary::ExecuteGameplayCueWithHitResult(
		GetOwner(), OtherActor, OverlapCueTag, SweepResult);
	Destroy();
}