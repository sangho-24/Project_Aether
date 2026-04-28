#include "Abilites/GA_EnemyAttack.h"
#include "AbilitySystemComponent.h"
#include "Gameplay/IAnimationInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Actor/BaseProjectile.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UGA_EnemyAttack::UGA_EnemyAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Attacking"));
    AbilityTag = FGameplayTag::RequestGameplayTag("Ability.EnemyAttack");
}

void UGA_EnemyAttack::PostInitProperties()
{
    Super::PostInitProperties();
    if (AbilityTag.IsValid())
    {
        FGameplayTagContainer AssetTagContainer;
        AssetTagContainer.AddTag(AbilityTag);
        SetAssetTags(AssetTagContainer);
    }
}

void UGA_EnemyAttack::ActivateAbility(
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
    
    IAnimationInterface* AnimChar = Cast<IAnimationInterface>(AvatarActor);
    if (!AnimChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GA_EnemyAttack] %s 애니메이션 인터페이스 없음"),
            *AvatarActor->GetName());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    AttackTarget = AnimChar->GetLockedOnTarget();
    
    if (!AbilityTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[GA_EnemyAttack] AbilityTag 미설정"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const FAbilitySkillData SkillData = AnimChar->GetSkillDataForAbility(AbilityTag);
    if (!SkillData.Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GA_EnemyAttack] '%s' 몽타주 없음"),
            *AbilityTag.ToString());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[GA_EnemyAttack] 발동 → %s"), *SkillData.Montage->GetName());

    // 몽타주 재생
    PlayMontage(SkillData.Montage); 

    // 투사체 스폰 이벤트 리스너 (근접 공격이면 AN 노티파이 안쓰면 됨)
    auto* SpawnTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, FGameplayTag::RequestGameplayTag("Event.SpawnProjectile"));
    SpawnTask->EventReceived.AddDynamic(this, &UGA_EnemyAttack::OnSpawnProjectile);
    SpawnTask->ReadyForActivation();
}

void UGA_EnemyAttack::PlayMontage(UAnimMontage* Montage)
{
    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, Montage, 1.0f);

    MontageTask->OnCompleted.AddDynamic(this, &UGA_EnemyAttack::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_EnemyAttack::OnMontageCancelled);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_EnemyAttack::OnMontageCancelled);
    MontageTask->ReadyForActivation();
}

void UGA_EnemyAttack::OnMontageCompleted()
{
    UE_LOG(LogTemp, Log, TEXT("[GA_EnemyAttack] 몽타주 완료 → EndAbility"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyAttack::OnMontageCancelled()
{
    UE_LOG(LogTemp, Warning, TEXT("[GA_EnemyAttack] 몽타주 취소/중단 → EndAbility"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_EnemyAttack::OnSpawnProjectile(FGameplayEventData Payload)
{
    if (!CurrentActorInfo)
        return;

    AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
    IAnimationInterface* AnimChar = Cast<IAnimationInterface>(AvatarActor);
    if (!AnimChar)
        return;

    TSubclassOf<AActor> ProjectileClass = AnimChar->GetNextProjectileClass();
    if (!ProjectileClass)
        return;

    const float DamageMult = AnimChar->GetNextDamageMultiplier();
    const FName SocketName = AnimChar->GetNextSpawnSocketName();

    FVector SpawnLocation = AvatarActor->GetActorLocation();
    FRotator SpawnRotation = AvatarActor->GetActorRotation();

    // 소켓 위치
    if (!SocketName.IsNone())
    {
        if (USkeletalMeshComponent* Mesh =
            AvatarActor->FindComponentByClass<USkeletalMeshComponent>())
        {
            if (Mesh->DoesSocketExist(SocketName))
                SpawnLocation = Mesh->GetSocketLocation(SocketName);
            else
                UE_LOG(LogTemp, Warning,
                    TEXT("[GA_EnemyAttack] 소켓 '%s' 없음 → 캐릭터 위치 사용"),
                    *SocketName.ToString());
        }
    }
    // 타겟 설정
    AActor* Target = const_cast<AActor*>(Payload.Target.Get());
    if (!Target)
        Target = AttackTarget.Get();
    if (Target)
    {
        FVector ToTarget = (Target->GetActorLocation() - SpawnLocation).GetSafeNormal();
        SpawnRotation = ToTarget.Rotation();
    }

    ABaseProjectile* Proj = AvatarActor->GetWorld()->SpawnActorDeferred<ABaseProjectile>(
        ProjectileClass,
        FTransform(SpawnRotation, SpawnLocation),
        AvatarActor,
        Cast<APawn>(AvatarActor),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (Proj)
    {
        Proj->InitProjectile(CurrentActorInfo->AbilitySystemComponent.Get(), DamageMult);
        UGameplayStatics::FinishSpawningActor(Proj, FTransform(SpawnRotation, SpawnLocation));
    }
}

void UGA_EnemyAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}