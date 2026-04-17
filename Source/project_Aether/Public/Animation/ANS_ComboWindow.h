
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_ComboWindow.generated.h"

UCLASS()
class PROJECT_AETHER_API UANS_ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	// 에디터에서 설정. 마지막 콤보면 비워둠 → 입력해도 무시
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Setup")
	TObjectPtr<UAnimMontage> NextComboMontage;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
