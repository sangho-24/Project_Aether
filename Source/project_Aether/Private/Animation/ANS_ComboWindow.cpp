
#include "Animation/ANS_ComboWindow.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Gameplay/IAnimationInterface.h"

void UANS_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC)
		return;
	
	// 다음 콤보 몽타주를 캐릭터에 저장 (nullptr이면 마지막 콤보)
	if (IAnimationInterface* AnimChar = Cast<IAnimationInterface>(Owner))
		AnimChar->SetNextComboMontage(NextComboMontage);

	// GA에 윈도우 오픈 이벤트 전송
	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		FGameplayTag::RequestGameplayTag("Event.Combo.WindowOpen"),
		EventData);
}

void UANS_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC)
		return;
	
	// 다음 콤보 몽타주 해제
	if (IAnimationInterface* AnimChar = Cast<IAnimationInterface>(Owner))
		AnimChar->SetNextComboMontage(nullptr);

	// GA에 윈도우 클로즈 이벤트 전송
	FGameplayEventData EventData;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		FGameplayTag::RequestGameplayTag("Event.Combo.WindowClose"),
		EventData);
}