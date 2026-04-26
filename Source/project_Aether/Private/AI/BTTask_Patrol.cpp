#include "AI/BTTask_Patrol.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_Patrol::UBTTask_Patrol()
{
	NodeName = TEXT("Set Patrol Location");
	// Vector 키만 선택 가능하도록 필터
	PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Patrol, PatrolLocationKey));
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) 
		return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) 
		return EBTNodeResult::Failed;

	// 현재 위치 기준 NavMesh 위 랜덤 포인트 탐색
	FNavLocation NavLocation;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
	if (!NavSys) 
		return EBTNodeResult::Failed;

	const bool bFound = NavSys->GetRandomReachablePointInRadius(
		Pawn->GetActorLocation(), PatrolRadius, NavLocation);

	if (!bFound) return EBTNodeResult::Failed;

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(
		PatrolLocationKey.SelectedKeyName, NavLocation.Location);

	return EBTNodeResult::Succeeded;
}