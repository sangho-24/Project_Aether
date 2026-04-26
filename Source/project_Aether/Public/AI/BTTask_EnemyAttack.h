// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_EnemyAttack.generated.h"

UCLASS()
class PROJECT_AETHER_API UBTTask_EnemyAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_EnemyAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual FString GetStaticDescription() const override;

	// 에디터에서 어떤 어빌리티를 실행할지 지정
	UPROPERTY(EditAnywhere, Category = "Attack|Setup")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;
};