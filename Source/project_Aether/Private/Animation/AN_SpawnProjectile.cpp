
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
	
	// 투사체 데이터를 캐릭터에 저장
	if (IAnimationInterface* AnimChar = Cast<IAnimationInterface>(Owner))
	{
		AnimChar->SetNextProjectileClass(ProjectileClass);
		AnimChar->SetNextDamageMultiplier(DamageMultiplier);
	}
	
	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		FGameplayTag::RequestGameplayTag("Event.BasicSkill.SpawnProjectile"),
		EventData);
}

