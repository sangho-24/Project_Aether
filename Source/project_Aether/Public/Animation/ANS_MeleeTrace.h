// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "ANS_MeleeTrace.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AETHER_API UANS_MeleeTrace : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeleeTrace")
	FName StartSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeleeTrace")
	FName EndSocketName = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeleeTrace", meta = (ClampMin = "1.0"))
	float TraceRadius = 20.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeleeTrace", meta = (ClampMin = "0.0"))
	float ExtraLength = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeleeTrace", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeleeTrace")
	FGameplayTag HitCueTag;
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation, float TotalDuration,
			const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
			const FAnimNotifyEventReference& EventReference) override;
};
