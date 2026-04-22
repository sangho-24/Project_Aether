#include "Abilites/GA_BasicSkill.h"
#include "AbilitySystemComponent.h"
#include "Gameplay/IAnimationInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actor/BaseProjectile.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


UGA_BasicSkill::UGA_BasicSkill()
{
	// 어빌리티 태그
	FGameplayTagContainer AssetTagContainer;
	AssetTagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.BasicSkill")));
	SetAssetTags(AssetTagContainer);

	// 발동 중 상태 태그 부여
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Combo")));

	// 인스턴스 정책: 콜백 바인딩을 위해 InstancedPerActor 필수
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_BasicSkill::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 인터페이스로 몽타주 요청
	IAnimationInterface* AnimChar = Cast<IAnimationInterface>(AvatarActor);
	if (!AnimChar)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_BasicSkill] %s 애니메이션 인터페이스 없음"),
			*AvatarActor->GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	const FAbilitySkillData SkillData = AnimChar->GetSkillDataForAbility(
	FGameplayTag::RequestGameplayTag("Ability.BasicSkill"));
	if (!SkillData.Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_BasicSkill] 몽타주 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 발동 → 몽타주: %s"), *SkillData.Montage->GetName());
	
	// 상태 초기화
	bSaveCombo = false;
	bIsComboWindowOpen = false;
	bComboTransitioning = false;
	
	// 공격 중 캐릭터가 바라보는 방향으로 회전 고정 (락온시에만)
	if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(
		FGameplayTag::RequestGameplayTag("State.LockedOn")))
	{
		if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
		{
			Character->GetCharacterMovement()->bOrientRotationToMovement = false;
			Character->bUseControllerRotationYaw = true;
		}
	}
	
	// 1타 몽타주 재생
	PlayMontage(SkillData.Montage);
	
	// ===== 이벤트 리스너 등록 (어빌리티 종료까지 유지) =====
	
	// 투사체 스폰 타이밍 (AN_SpawnProjectile → 이벤트 수신)
	auto* SpawnTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag("Event.BasicSkill.SpawnProjectile"));
	SpawnTask->EventReceived.AddDynamic(this, &UGA_BasicSkill::OnSpawnProjectile);
	SpawnTask->ReadyForActivation();

	// 콤보 입력 (캐릭터 BasicSkillAction → 이벤트 수신)
	auto* ComboInputTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag("Event.BasicSkill.ComboInput"));
	ComboInputTask->EventReceived.AddDynamic(this, &UGA_BasicSkill::OnComboInput);
	ComboInputTask->ReadyForActivation();

	// 콤보 윈도우 오픈 (ANS_ComboWindow NotifyBegin → 이벤트 수신)
	auto* WindowOpenTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag("Event.Combo.WindowOpen"));
	WindowOpenTask->EventReceived.AddDynamic(this, &UGA_BasicSkill::OnComboWindowOpen);
	WindowOpenTask->ReadyForActivation();

	// 콤보 윈도우 클로즈 (ANS_ComboWindow NotifyEnd → 이벤트 수신)
	auto* WindowCloseTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag("Event.Combo.WindowClose"));
	WindowCloseTask->EventReceived.AddDynamic(this, &UGA_BasicSkill::OnComboWindowClose);
	WindowCloseTask->ReadyForActivation();
}
	
void UGA_BasicSkill::PlayMontage(UAnimMontage* Montage)
{
	UAbilityTask_PlayMontageAndWait* MontageTask =
	UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, 1.0f);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_BasicSkill::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_BasicSkill::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_BasicSkill::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_BasicSkill::TryNextCombo()
{
	if (!CurrentActorInfo) 
		return;
	IAnimationInterface* AnimChar = Cast<IAnimationInterface>(
		CurrentActorInfo->AvatarActor.Get());
	
	if (!AnimChar) 
		return;
	UAnimMontage* NextMontage = AnimChar->GetNextComboMontage();
	if (!NextMontage)
	{
		// 마지막 콤보 → 입력 무시, 윈도우 종료 후 자연스럽게 EndAbility
		UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 마지막 콤보 → 이어지는 콤보 없음"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 콤보 전환 → %s"), *NextMontage->GetName());
	bIsComboWindowOpen = false;
	bSaveCombo = false;
	// 이전 몽타주 중단 시 OnMontageCancelled가 오지만 EndAbility 방지
	bComboTransitioning = true;

	PlayMontage(NextMontage);
	// PlayMontage 내부에서 이전 몽타주 중단 → OnMontageCancelled 동기 호출됨
	// OnMontageCancelled에서 bComboTransitioning 확인 후 플래그 해제
}

// ===== 몽타주 콜백 =====
void UGA_BasicSkill::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 몽타주 완료 → EndAbility"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_BasicSkill::OnMontageCancelled()
{
    if (bComboTransitioning)
    {
        // 콤보 전환으로 인한 중단 → EndAbility 호출하지 않음
        bComboTransitioning = false;
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("[GA_BasicSkill] 몽타주 취소/중단 → EndAbility"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

// ===== 이벤트 콜백 =====
void UGA_BasicSkill::OnSpawnProjectile(FGameplayEventData Payload)
{
	if (!CurrentActorInfo) 
		return;
	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	IAnimationInterface* AnimChar = Cast<IAnimationInterface>(AvatarActor);
	if (!AnimChar) 
		return;
	
	TSubclassOf<AActor> ProjectileClass = AnimChar->GetNextProjectileClass();
	const float DamageMult = AnimChar->GetNextDamageMultiplier();
	const FName SocketName = AnimChar->GetNextSpawnSocketName();
	AActor* Target = AnimChar->GetLockedOnTarget();
	if (!ProjectileClass)
		return;
	
	FVector  SpawnLocation = AvatarActor->GetActorLocation();
	FRotator SpawnRotation = AvatarActor->GetActorRotation();
	if (!SocketName.IsNone())
	{
		if (USkeletalMeshComponent* Mesh =
			AvatarActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Mesh->DoesSocketExist(SocketName))
			{
				SpawnLocation = Mesh->GetSocketLocation(SocketName);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[GA_BasicSkill] 소켓 '%s' 없음 → 캐릭터 위치 사용"), *SocketName.ToString());
			}
		}
	}
	// 타겟이 있으면 타겟을 향하도록
	if (Target)
	{
		FVector ToTarget = (Target->GetActorLocation() - SpawnLocation).GetSafeNormal();
		SpawnRotation = ToTarget.Rotation();
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.Owner      = AvatarActor;

	AActor* SpawnedActor = AvatarActor->GetWorld()->SpawnActor<AActor>(
		ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);


	// ── 투사체 초기화 (ABaseProjectile 파생 클래스인 경우) ──
	if (ABaseProjectile* Proj = Cast<ABaseProjectile>(SpawnedActor))
	{
		UAbilitySystemComponent* SourceASC =
			CurrentActorInfo->AbilitySystemComponent.Get();
		
		Proj->InitProjectile(SourceASC, DamageMult);
	}
}

void UGA_BasicSkill::OnComboInput(FGameplayEventData Payload)
{
    if (bIsComboWindowOpen)
    {
        UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 콤보 윈도우 내 입력 → 즉시 콤보"));
        TryNextCombo();
    }
    else
    {
        // 윈도우 열리기 전 선입력
        UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 선입력 저장 (bSaveCombo=true)"));
        bSaveCombo = true;
    }
}

void UGA_BasicSkill::OnComboWindowOpen(FGameplayEventData Payload)
{
    bIsComboWindowOpen = true;
    UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 콤보 윈도우 오픈"));

    if (bSaveCombo)
    {
        UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 선입력 감지 → 즉시 콤보 실행"));
        bSaveCombo = false;
        TryNextCombo();
    }
}

void UGA_BasicSkill::OnComboWindowClose(FGameplayEventData Payload)
{
    bIsComboWindowOpen = false;
    bSaveCombo = false;
    UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 콤보 윈도우 닫힘"));
    // 입력 없이 윈도우가 닫히면 몽타주 끝까지 재생 후 OnMontageCompleted → EndAbility
}

void UGA_BasicSkill::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 공격 종료 시 무브먼트 회전 복원
	if (ActorInfo)
	{
		if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			Character->GetCharacterMovement()->bOrientRotationToMovement = true;
			Character->bUseControllerRotationYaw = false;
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}