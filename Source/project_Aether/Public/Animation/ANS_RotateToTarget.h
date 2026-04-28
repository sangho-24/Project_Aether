#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_RotateToTarget.generated.h"


UCLASS()
class PROJECT_AETHER_API UANS_RotateToTarget : public UAnimNotifyState
{
	GENERATED_BODY()
	
	public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	 float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		  float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;



protected:
	UPROPERTY(EditAnywhere, Category = "Tracking")
	float RotationSpeed = 2.f;
	
private:
	// Begin에서 캐싱
	TWeakObjectPtr<AActor> CachedOwner;
	TWeakObjectPtr<AActor> CachedTarget;
};
