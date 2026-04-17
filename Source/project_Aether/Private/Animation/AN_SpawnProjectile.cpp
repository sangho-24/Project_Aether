
#include "Animation/AN_SpawnProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "Gameplay/IAnimationInterface.h"

void UAN_SpawnProjectile::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) 
		return;
	
	// 프리뷰 액터(에디터) 등 ASC 없는 경우 무시
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC)
		return;
	
	// 투사체 데이터를 캐릭터에 저장
	if (IAnimationInterface* AnimChar = Cast<IAnimationInterface>(Owner))
	{
		AnimChar->SetNextProjectileClass(ProjectileClass);
		AnimChar->SetNextDamageMultiplier(DamageMultiplier);
		AnimChar->SetNextSpawnSocketName(SpawnSocketName);
	}
	
	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		FGameplayTag::RequestGameplayTag("Event.BasicSkill.SpawnProjectile"),
		EventData);
}

