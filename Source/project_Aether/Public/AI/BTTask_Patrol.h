// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_Patrol.generated.h"

UCLASS()
class PROJECT_AETHER_API UBTTask_Patrol : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Patrol();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 순찰 반경 (스폰 위치 기준)
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolRadius = 1000.f;

	// 블랙보드에 결과 위치를 쓸 키
	UPROPERTY(EditAnywhere, Category = "Patrol|Setup")
	FBlackboardKeySelector PatrolLocationKey;
};