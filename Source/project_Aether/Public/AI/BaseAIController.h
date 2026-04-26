
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BaseAIController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;

UCLASS()
class PROJECT_AETHER_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABaseAIController();
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

protected:
	// Character의 BehaviorTree 프로퍼티를 참조하는 구조
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName TargetActorKey = TEXT("TargetActor");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName PatrolLocationKey = TEXT("PatrolLocation");
	
	// ===== Perception 설정 =====
	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
	float SightRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
	float LoseSightRadius = 1800.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
	float PeripheralVisionAngle = 90.f;
	
	// ===== 내부 컴포넌트 =====
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	UAIPerceptionComponent* AIPerception;

	UAISenseConfig_Sight* SightConfig;
	
	// ===== 감지 콜백 (파생 클래스에서 오버라이드 가능) =====
	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// 타겟 설정/해제
	void SetTargetActor(AActor* Target);
	void ClearTargetActor();
};