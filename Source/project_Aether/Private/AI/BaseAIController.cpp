#include "AI/BaseAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Character/BaseEnemyCharacter.h"


ABaseAIController::ABaseAIController()
{
    SetGenericTeamId(FGenericTeamId(1));
    
    // ===== Perception 설정 =====
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
    SightConfig->SetMaxAge(5.f);
    
    // 플레이어만 감지
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    AIPerception->ConfigureSense(*SightConfig);
    AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    SetPerceptionComponent(*AIPerception);

    AIPerception->OnTargetPerceptionUpdated.AddDynamic(
        this, &ABaseAIController::OnTargetPerceptionUpdated);
}

void ABaseAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(InPawn);
    if (!Enemy) return;

    // Character에 지정된 BT 에셋을 가져와서 실행
    if (UBehaviorTree* BT = Enemy->GetBehaviorTree())
    {
        RunBehaviorTree(BT);
    }
}

void ABaseAIController::OnUnPossess()
{
    Super::OnUnPossess();
    ClearTargetActor();
}

void ABaseAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) 
        return;
    
    if (Stimulus.WasSuccessfullySensed())
    {
        // 플레이어(또는 감지 대상)를 발견 → 블랙보드에 타겟 설정
        // GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, 
        //     FString::Printf(TEXT("Target SET: %s"), *Actor->GetName()));
        SetTargetActor(Actor);
    }
    else
    {
        // 시야에서 사라짐 → 타겟 해제
        ClearTargetActor();
    }
}

void ABaseAIController::SetTargetActor(AActor* Target)
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsObject(TargetActorKey, Target);
    }
}

void ABaseAIController::ClearTargetActor()
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->ClearValue(TargetActorKey);
    }
}