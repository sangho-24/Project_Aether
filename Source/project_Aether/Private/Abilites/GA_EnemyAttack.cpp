#include "Abilites/GA_EnemyAttack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actor/BaseProjectile.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GAS/BaseAttributeSet.h"
#include "Utility/AetherGASLibrary.h"

#define ECC_Damageable ECC_GameTraceChannel3

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
    
    // 근접 공격 이벤트 리스너
    auto* MeleeStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
       this, FGameplayTag::RequestGameplayTag("Event.MeleeTrace.Start"),
       nullptr, false,true);
    MeleeStartTask->EventReceived.AddDynamic(this, &UGA_EnemyAttack::OnMeleeTraceStart);
    MeleeStartTask->ReadyForActivation();

    auto* MeleeEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, FGameplayTag::RequestGameplayTag("Event.MeleeTrace.End"),
        nullptr, false, true);
    MeleeEndTask->EventReceived.AddDynamic(this, &UGA_EnemyAttack::OnMeleeTraceEnd);
    MeleeEndTask->ReadyForActivation();
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

void UGA_EnemyAttack::StartMeleeTrace()
{
    UE_LOG(LogTemp, Log, TEXT("[GA_EnemyAttack] 근접 공격 트레이스 시작"));
    if (!CurrentActorInfo) 
        return;
    UWorld* World = CurrentActorInfo->AvatarActor.Get()
        ? CurrentActorInfo->AvatarActor->GetWorld() : nullptr;
    if (!World) 
        return;

    World->GetTimerManager().ClearTimer(TraceTimerHandle);
    World->GetTimerManager().SetTimer(
        TraceTimerHandle,
        this,
        &UGA_EnemyAttack::DoMeleeTrace,
        TraceTickRate,
        true,
        0.0f); 
}

void UGA_EnemyAttack::StopMeleeTrace()
{
    if (!CurrentActorInfo) 
        return;
    AActor* Avatar = CurrentActorInfo->AvatarActor.Get();
    if (Avatar)
        Avatar->GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
}

void UGA_EnemyAttack::DoMeleeTrace()
{
    if (!CurrentActorInfo) 
        return;

    AActor* Avatar = CurrentActorInfo->AvatarActor.Get();
    if (!Avatar) 
        return;

    USkeletalMeshComponent* Mesh = Avatar->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh) 
        return;

    UWorld* World = Avatar->GetWorld();

    // 소켓 위치 설정
    FVector TraceStart = Avatar->GetActorLocation();
    FVector TraceEnd   = Avatar->GetActorLocation();

    if (!ActiveTraceData.StartSocketName.IsNone()
        && Mesh->DoesSocketExist(ActiveTraceData.StartSocketName))
        TraceStart = Mesh->GetSocketLocation(ActiveTraceData.StartSocketName);
    if (!ActiveTraceData.EndSocketName.IsNone()
        && Mesh->DoesSocketExist(ActiveTraceData.EndSocketName))
        TraceEnd = Mesh->GetSocketLocation(ActiveTraceData.EndSocketName);
    // ExtraLength만큼 연장
    if (ActiveTraceData.ExtraLength > 0.0f)
    {
        FVector Dir = (TraceEnd - TraceStart);
        Dir = Dir.IsNearlyZero() ? Avatar->GetActorForwardVector() : Dir.GetSafeNormal();
        TraceEnd += Dir * ActiveTraceData.ExtraLength;
    }
    

    // SphereTrace, 멀티 채널로 모든 타겟 탐색(단일 대상이면 SweepSingleByChannel을 사용)
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Avatar);
    const bool bHit = World->SweepMultiByChannel(
        Hits,
        TraceStart,
        TraceEnd,
        FQuat::Identity,
        ECC_Damageable,
        FCollisionShape::MakeSphere(ActiveTraceData.TraceRadius),
        Params);

    if (bDrawDebugTrace)
    {
        const FColor Color = bHit ? FColor::Red : FColor::Green;
        DrawDebugLine(World, TraceStart, TraceEnd, Color, false, TraceTickRate * 2.0f, 0, 2.0f);
        DrawDebugSphere(World, TraceStart, ActiveTraceData.TraceRadius, 8, Color, false, TraceTickRate * 2.0f);
        DrawDebugSphere(World, TraceEnd,   ActiveTraceData.TraceRadius, 8, Color, false, TraceTickRate * 2.0f);
    }

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActor == Avatar) 
            continue;
        
        // 중복 방지
        if (HitActors.Contains(HitActor)) 
            continue;

        HitActors.Add(HitActor);
        ApplyMeleeDamage(HitActor, Hit);
    }
}

void UGA_EnemyAttack::ApplyMeleeDamage(AActor* TargetActor, const FHitResult& HitResult)
{
    UE_LOG(LogTemp, Log, TEXT("[애너미 어택] 0. 시작"));
    UE_LOG(LogTemp, Warning, TEXT("[ApplyMeleeDamage] TargetActor=%s / MeleeDamageEffect=%s / CurrentActorInfo=%s"),
       TargetActor ? *TargetActor->GetName() : TEXT("NULL"),
       MeleeDamageEffect ? TEXT("OK") : TEXT("NULL"),
       CurrentActorInfo ? TEXT("OK") : TEXT("NULL"));
    if (!TargetActor || !MeleeDamageEffect || !CurrentActorInfo) 
        return;
    UE_LOG(LogTemp, Log, TEXT("[애너미 어택] 1. GE랑 액터인포는 있음"));
    UAbilitySystemComponent* SourceASC = CurrentActorInfo->AbilitySystemComponent.Get();
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!TargetASC) 
        return;
    UE_LOG(LogTemp, Log, TEXT("[애너미 어택] 2. 타겟 ASC도 있음"));
    FGameplayEffectContextHandle ContextHandle = SourceASC
        ? SourceASC->MakeEffectContext()
        : TargetASC->MakeEffectContext();
    ContextHandle.AddHitResult(HitResult);
    ContextHandle.AddInstigator(CurrentActorInfo->AvatarActor.Get(), CurrentActorInfo->AvatarActor.Get());

    FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(MeleeDamageEffect, 1.0f, ContextHandle);
    if (!SpecHandle.IsValid()) 
        return;
    UE_LOG(LogTemp, Log, TEXT("[애너미 어택] 3. GE 스펙 유효"));
    // 최종 데미지 = Base × (1 + AttackPower/100) × ANS의 DamageMultiplier
    float FinalDamage = MeleeBaseDamage;
    if (SourceASC)
    {
        if (const UBaseAttributeSet* SourceAttributeSet = SourceASC->GetSet<UBaseAttributeSet>())
            FinalDamage *= (1.0f + SourceAttributeSet->GetAttackPower() / 100.0f);
    }
    FinalDamage *= ActiveTraceData.DamageMultiplier;

    FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"));
    SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, -FinalDamage);

    TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

    UE_LOG(LogTemp, Log, TEXT("[GA_EnemyAttack] 근접 히트 → %s (%.1f 데미지)"),
        *TargetActor->GetName(), FinalDamage);
    
    UAetherGASLibrary::ExecuteGameplayCueWithHitResult(
        CurrentActorInfo->AvatarActor.Get(), TargetActor, ActiveTraceData.HitCueTag, HitResult);
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

void UGA_EnemyAttack::OnMeleeTraceStart(FGameplayEventData Payload)
{
    if (!CurrentActorInfo) 
        return;

    AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
    IAnimationInterface* AnimChar = Cast<IAnimationInterface>(AvatarActor);
    if (!AnimChar)
        return;
    
    ActiveTraceData = AnimChar->GetMeleeTraceData();
    HitActors.Empty();
    StartMeleeTrace();
}

void UGA_EnemyAttack::OnMeleeTraceEnd(FGameplayEventData Payload)
{
    StopMeleeTrace();
    HitActors.Empty();
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
    StopMeleeTrace();
    HitActors.Empty();
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}