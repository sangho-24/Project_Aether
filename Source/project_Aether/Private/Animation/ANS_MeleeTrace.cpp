
#include "Animation/ANS_MeleeTrace.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Gameplay/IAnimationInterface.h"

void UANS_MeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) 
		return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) 
		return;

	// 파라미터를 캐릭터에 저장 → GA가 읽어감
	if (IAnimationInterface* AnimChar = Cast<IAnimationInterface>(Owner))
	{
		FMeleeTraceData Data;
		Data.StartSocketName	= StartSocketName;
		Data.EndSocketName		= EndSocketName;
		Data.TraceRadius		= TraceRadius;
		Data.ExtraLength		= ExtraLength;
		Data.DamageMultiplier	= DamageMultiplier;
		Data.HitCueTag			= HitCueTag;
		AnimChar->SetMeleeTraceData(Data);
	}

	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		FGameplayTag::RequestGameplayTag("Event.MeleeTrace.Start"),
		EventData);
}

void UANS_MeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) 
		return;
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) 
		return;

	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		FGameplayTag::RequestGameplayTag("Event.MeleeTrace.End"),
		EventData);
}