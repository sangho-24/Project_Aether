
#include "Abilites/GA_BasicSkill.h"
#include "Gameplay/IAnimationInterface.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_BasicSkill::UGA_BasicSkill()
{
	// 어빌리티 식별 태그
	FGameplayTagContainer AssetTagContainer;
	AssetTagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.BasicSkill")));
	SetAssetTags(AssetTagContainer);

	// 발동 중 상태 태그 부여
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));

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

	const FGameplayTag SkillTag = FGameplayTag::RequestGameplayTag("Ability.BasicSkill");
	const FAbilitySkillData SkillData = AnimChar->GetSkillDataForAbility(SkillTag);
	if (!SkillData.Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_BasicSkill] 몽타주 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 발동 → 몽타주: %s / 투사체: %s"),
			*SkillData.Montage->GetName(),
			SkillData.ProjectileClass ? *SkillData.ProjectileClass->GetName() : TEXT("없음"));
	
	// 2. 몽타주 재생 태스크
	UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, 
            NAME_None, 
            SkillData.Montage, 
            1.0f);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_BasicSkill::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_BasicSkill::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_BasicSkill::OnMontageCancelled);

	MontageTask->ReadyForActivation();

	// TODO: 투사체 생성은 AnimNotify(AN_SpawnProjectile) 시점에 처리 예정
}

void UGA_BasicSkill::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[GA_BasicSkill] 몽타주 완료 → EndAbility"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_BasicSkill::OnMontageCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_BasicSkill] 몽타주 취소/중단 → EndAbility"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_BasicSkill::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("[GA_BasicSkill] EndAbility - 취소 여부: %s"),
		bWasCancelled ? TEXT("YES") : TEXT("NO"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}