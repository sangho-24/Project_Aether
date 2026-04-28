// 기타 Include
#include "Character/BasePlayableCharacter.h"
#include "Character/BaseEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actor/FloatingDamageActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
// 입력 Include
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
// 카메라 Include
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
// GAS Include
#include "AbilitySystemComponent.h"
#include "GAS/PlayerAttributeSet.h"
// UI Include



namespace BaseConstants
{
	constexpr float ZoomSnapTolerance = 1.0f;
}

ABasePlayableCharacter::ABasePlayableCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 이동 설정
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 300.0f;

	// 스프링암
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	CameraBoom->TargetArmLength = 400.0f; // 카메라 거리
	CameraBoom->bUsePawnControlRotation = true; // 마우스 입력으로 붐 회전
	// 카메라
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 붐이 회전하므로 카메라는 고정

	// GAS 초기화
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// AttributeSet은 나중에 Blueprint에서 설정하거나 여기서 생성 가능
	AttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("AttributeSet"));
}

void ABasePlayableCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (CameraBoom)
	{
		DesiredArmLength = CameraBoom->TargetArmLength;
	}
	// Enhanced Input System 설정
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		}
	}
	// GAS 초기화
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		// 기본 능력 부여
		for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
		{
			if (AbilityClass)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, 0));
			}
		}
	}
}

void ABasePlayableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsZoomInterpolating)
	{
		if (!CameraBoom)
		{
			bIsZoomInterpolating = false;
			return;
		}
		const float CurrentArmLength = CameraBoom->TargetArmLength;
		const float NewArmLength = FMath::FInterpTo(
			CurrentArmLength,
			DesiredArmLength,
			DeltaTime,
			ZoomInterpSpeed);
		CameraBoom->TargetArmLength = NewArmLength;
		if (FMath::IsNearlyEqual(
			NewArmLength,
			DesiredArmLength,
			BaseConstants::ZoomSnapTolerance))
		{
			CameraBoom->TargetArmLength = DesiredArmLength;
			bIsZoomInterpolating = false;
		}
	}
	// 락온 카메라 추적
	if (bIsLockedOn && LockOnTarget)
	{
		// 타겟이 죽었거나 유효하지 않으면 해제
		if (!IsValid(LockOnTarget) || LockOnTarget->GetIsDead())
		{
			bIsLockedOn = false;
			// SetLockedOnTarget(nullptr);
			LockOnTarget = nullptr;
			AbilitySystemComponent->RemoveLooseGameplayTag(
				FGameplayTag::RequestGameplayTag("State.LockedOn"));
			return;
		}
		// 거리가 멀어지면 해제
		if (FVector::Dist(GetActorLocation(), LockOnTarget->GetActorLocation()) > LockOnRadius)
		{
			bIsLockedOn = false;
			// SetLockedOnTarget(nullptr);
			LockOnTarget = nullptr;
			AbilitySystemComponent->RemoveLooseGameplayTag(
				FGameplayTag::RequestGameplayTag("State.LockedOn"));
			return;
		}
		
		// 플레이어 → 타겟 방향으로 컨트롤러 회전 보간
		FVector ToTarget = LockOnTarget->GetActorLocation() - GetActorLocation();
		FRotator TargetRot = ToTarget.Rotation();
		TargetRot.Pitch -= 30.f;
		FRotator CurrentRot = Controller->GetControlRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, LockOnInterpSpeed);
		Controller->SetControlRotation(NewRot);
	}
	
	// 공격 중 타겟 방향 회전 유지
	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(
		FGameplayTag::RequestGameplayTag("State.FaceTarget")))
	{
		// 락온 우선, 없으면 소프트 타겟
		AActor* RotTarget = LockOnTarget;
		if (!RotTarget)
			RotTarget = NearestTarget;

		if (RotTarget)
		{
			FVector ToTarget = RotTarget->GetActorLocation() - GetActorLocation();
			ToTarget.Z = 0.f;
			if (!ToTarget.IsNearlyZero())
			{
				FRotator NewRot = GetActorRotation();
				NewRot.Yaw = FMath::RInterpTo(
					GetActorRotation(), ToTarget.Rotation(), DeltaTime, 15.f).Yaw;
				SetActorRotation(NewRot);
			}
		}
	}
}

// Called to bind functionality to input
void ABasePlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 이동
		if (MoveInput)
		{
			EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Triggered, this,
			                                   &ABasePlayableCharacter::MoveAction);
		}

		// 카메라 시점
		if (LookInput)
		{
			EnhancedInputComponent->BindAction(LookInput, ETriggerEvent::Triggered, this,
			                                   &ABasePlayableCharacter::LookAction);
		}
		if (ZoomInput)
		{
			EnhancedInputComponent->BindAction(ZoomInput, ETriggerEvent::Triggered, this,
			                                   &ABasePlayableCharacter::ZoomAction);
		}
		// 락온
		if (LockOnInput)
		{
			EnhancedInputComponent->BindAction(LockOnInput, ETriggerEvent::Triggered, this,
											   &ABasePlayableCharacter::LockOnAction);
		}
		// 점프 (GAS)
		if (JumpInput)
		{
			EnhancedInputComponent->BindAction(JumpInput, ETriggerEvent::Started, this,
			                                   &ABasePlayableCharacter::JumpAction);
			EnhancedInputComponent->BindAction(JumpInput, ETriggerEvent::Completed, this,
			                                   &ABasePlayableCharacter::StopJumpingAction);
		}
		// 스킬 (GAS)
		if (BasicSkillInput)
		{
			EnhancedInputComponent->BindAction(BasicSkillInput, ETriggerEvent::Triggered, this,
			                                   &ABasePlayableCharacter::BasicSkillAction);
		}
	}
}

void ABasePlayableCharacter::MoveAction(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D MovementVector = Value.Get<FVector2D>();

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (MovementVector.Y != 0.0f)
			AddMovementInput(ForwardDir, MovementVector.Y);
		if (MovementVector.X != 0.0f)
			AddMovementInput(RightDir, MovementVector.X);
	}
}

void ABasePlayableCharacter::LookAction(const FInputActionValue& Value)
{
	if (bIsLockedOn)
		return;

	if (Controller != nullptr)
	{
		const FVector2D LookAxisVector = Value.Get<FVector2D>();
		// 상/하 회전
		if (LookAxisVector.Y != 0.0f)
			AddControllerPitchInput(LookAxisVector.Y);
		// 좌/우 회전
		if (LookAxisVector.X != 0.0f)
			AddControllerYawInput(LookAxisVector.X);
	}
}

void ABasePlayableCharacter::ZoomAction(const FInputActionValue& Value)
{
	if (!Controller || !CameraBoom)
		return;

	const float ZoomAxis = Value.Get<float>();
	if (FMath::IsNearlyZero(ZoomAxis))
		return;

	DesiredArmLength = FMath::Clamp(
		DesiredArmLength - ZoomAxis * ZoomStep,
		MinArmLength,
		MaxArmLength);
	StartZoomInterp();
}

void ABasePlayableCharacter::StartZoomInterp()
{
	if (!CameraBoom)
		return;

	// 이미 목표에 도달했다면 즉시 스냅하고 보간 종료
	if (FMath::IsNearlyEqual(
		CameraBoom->TargetArmLength,
		DesiredArmLength,
		BaseConstants::ZoomSnapTolerance))
	{
		CameraBoom->TargetArmLength = DesiredArmLength;
		bIsZoomInterpolating = false;
		return;
	}
	bIsZoomInterpolating = true;
}

void ABasePlayableCharacter::JumpAction()
{
	// GAS를 사용하여 점프 활성화
	if (AbilitySystemComponent)
	{
		const bool bActivated = AbilitySystemComponent->TryActivateAbilitiesByTag(
			FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Jump"))));
		UE_LOG(LogTemp, Warning, TEXT("Jump TryActivate result: %s"), bActivated ? TEXT("true") : TEXT("false"));
	}
	else
	{
		Super::Jump();
		UE_LOG(LogTemp, Warning, TEXT("AbilitySystemComponent 없음. 기본 점프."));
	}
}

void ABasePlayableCharacter::StopJumpingAction()
{
	Super::StopJumping();
}

void ABasePlayableCharacter::BasicSkillAction()
{
	if (!AbilitySystemComponent)
		return;

	if (AbilitySystemComponent->HasMatchingGameplayTag(
		FGameplayTag::RequestGameplayTag("State.Combo")))
	{
		// 이미 공격 중 → 콤보 입력 이벤트 전송
		FGameplayEventData EventData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this,
			FGameplayTag::RequestGameplayTag("Event.BasicSkill.ComboInput"),
			EventData);
	}
	else
	{
		// 공격 중 아님 → 어빌리티 새로 발동
		AbilitySystemComponent->TryActivateAbilitiesByTag(
			FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.BasicSkill")));
	}
}

void ABasePlayableCharacter::LockOnAction()
{
	if (bIsLockedOn)
	{
		// 락온 해제
		bIsLockedOn = false;
		// SetLockedOnTarget(nullptr);
		LockOnTarget = nullptr;
		// GetCharacterMovement()->bOrientRotationToMovement = true;
		UE_LOG(LogTemp, Log, TEXT("LockOn: 해제"));
		AbilitySystemComponent->RemoveLooseGameplayTag(
			FGameplayTag::RequestGameplayTag("State.LockedOn"));
		return;
	}

	ABaseEnemyCharacter* BestTarget = FindLockOnTarget();
	if (BestTarget)
	{
		bIsLockedOn = true;
		// SetLockedOnTarget(BestTarget);
		LockOnTarget = BestTarget;
		// GetCharacterMovement()->bOrientRotationToMovement = false;
		UE_LOG(LogTemp, Log, TEXT("LockOn: %s"), *BestTarget->GetName());
		AbilitySystemComponent->AddLooseGameplayTag(
			FGameplayTag::RequestGameplayTag("State.LockedOn"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("LockOn: 주변에 타겟 없음"));
	}
}

ABaseEnemyCharacter* ABasePlayableCharacter::FindLockOnTarget()
{
	if (!GetWorld()) return nullptr;

	// 반경 내 모든 캐릭터 탐색
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(LockOnRadius),
		Params);

	// 화면 중앙에 가장 가까운(카메라 정면) 적 선택
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (!PC) 
		return nullptr;

	ABaseEnemyCharacter* BestActor = nullptr;
	float BestScore = -1.f; // dot product 기준 (1에 가까울수록 정면)

	for (const FOverlapResult& Result : OverlapResults)
	{
		ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(Result.GetActor());
		if (!Enemy) 
			continue;

		// 카메라 정면 방향과의 각도 계산 (dot product)
		FVector CamForward = FollowCamera->GetForwardVector();
		FVector ToActor = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(CamForward, ToActor);

		if (Dot > BestScore)
		{
			BestScore = Dot;
			BestActor = Enemy;
		}
	}
	return BestActor;
}


UAbilitySystemComponent* ABasePlayableCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPlayerAttributeSet* ABasePlayableCharacter::GetPlayerAttributeSet() const
{
	return AttributeSet;
}

void ABasePlayableCharacter::SpawnFloatingDamage(const float Amount, const bool bIsHeal, const bool bIsCritical)
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

void ABasePlayableCharacter::Death(AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("Character died. Killer: %s"), Killer ? *Killer->GetName() : TEXT("None"));
}

FAbilitySkillData ABasePlayableCharacter::GetSkillDataForAbility(FGameplayTag AbilityTag)
{
	FAbilitySkillData* Found = AbilitySkillDataMap.Find(AbilityTag);
	if (!Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimInterface] '%s' 태그에 대한 스킬 데이터 없음"),
		       *AbilityTag.ToString());
		return FAbilitySkillData{}; // 빈 구조체 반환
	}
	return *Found;
}

void ABasePlayableCharacter::SetNextComboMontage(UAnimMontage* Montage)
{
	NextComboMontage = Montage;
}

UAnimMontage* ABasePlayableCharacter::GetNextComboMontage() const
{
	return NextComboMontage;
}

void ABasePlayableCharacter::SetNextProjectileClass(TSubclassOf<AActor> ProjectileClass)
{
	NextProjectileClass = ProjectileClass;
}
TSubclassOf<AActor> ABasePlayableCharacter::GetNextProjectileClass() const
{
	return NextProjectileClass;
}

void ABasePlayableCharacter::SetNextDamageMultiplier(float DamageMultiplier)
{
	NextDamageMultiplier = DamageMultiplier;
}
float ABasePlayableCharacter::GetNextDamageMultiplier() const
{
	return NextDamageMultiplier;
}

void ABasePlayableCharacter::SetNextSpawnSocketName(FName SocketName)
{
	NextSpawnSocketName = SocketName;
}
FName ABasePlayableCharacter::GetNextSpawnSocketName() const
{
	return NextSpawnSocketName;
}

AActor* ABasePlayableCharacter::GetLockedOnTarget() const
{
	return LockOnTarget;
}

AActor* ABasePlayableCharacter::GetNearestTarget() const
{
	return NearestTarget;         // 범위 내 소프트 타겟
}

void ABasePlayableCharacter::SetNearestTarget()
{
		if (!GetWorld())
		{
			NearestTarget = nullptr;
			return;
		}

		// 반경 내 모든 캐릭터 탐색
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		GetWorld()->OverlapMultiByObjectType(
			OverlapResults,
			GetActorLocation(),
			FQuat::Identity,
			FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
			FCollisionShape::MakeSphere(LockOnRadius),
			Params);

		// 거리 기준 가장 가까운 적 선택
		ABaseEnemyCharacter* NearestActor = nullptr;
		float NearestDistSq = FLT_MAX;  // 제곱 거리 비교 (sqrt 불필요)

		for (const FOverlapResult& Result : OverlapResults)
		{
			ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(Result.GetActor());
			if (!Enemy || Enemy->GetIsDead())
				continue;

			const float DistSq = FVector::DistSquared(GetActorLocation(), Enemy->GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestActor = Enemy;
			}
		}
		NearestTarget = NearestActor;
}
