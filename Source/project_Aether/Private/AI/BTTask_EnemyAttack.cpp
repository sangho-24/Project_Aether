#include "AI/BTTask_EnemyAttack.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Character/BaseEnemyCharacter.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
	NodeName = TEXT("Enemy Attack");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_EnemyAttack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(
		OwnerComp.GetAIOwner()->GetPawn());

	if (!Enemy || !AttackAbilityClass) 
		return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
	if (!ASC) 
		return EBTNodeResult::Failed;

	// 어빌리티 종료 콜백 등록
	AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(
		this, &UBTTask_EnemyAttack::OnAbilityEnded, &OwnerComp);

	const bool bActivated = ASC->TryActivateAbilityByClass(AttackAbilityClass);
	if (!bActivated)
	{
		ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
		return EBTNodeResult::Failed;
	}
	
	// InProgress 반환 → BT가 이 태스크에서 멈춤
	return EBTNodeResult::InProgress;
}

void UBTTask_EnemyAttack::OnAbilityEnded(
	const FAbilityEndedData& EndedData, UBehaviorTreeComponent* BTComp)
{
	if (!EndedData.AbilityThatEnded)
		return;

	// 내가 발동한 어빌리티 클래스인지 확인
	if (EndedData.AbilityThatEnded->GetClass() != AttackAbilityClass)
		return;

	if (BTComp)
	{
		// 어빌리티 정상 종료 → BT 태스크 완료
		FinishLatentTask(*BTComp, EBTNodeResult::Succeeded);
	}

	// 델리게이트 해제
	if (ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(
		BTComp->GetAIOwner()->GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent())
		{
			ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
			AbilityEndedHandle.Reset();
		}
	}
}

void UBTTask_EnemyAttack::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	// 이미 해제됐으면 스킵
	if (!AbilityEndedHandle.IsValid()) 
	{
		Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
		return;
	}
	if (ABaseEnemyCharacter* Enemy = Cast<ABaseEnemyCharacter>(
		OwnerComp.GetAIOwner()->GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent())
		{
			ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
		}
	}
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

// 어빌리티 등록된거 보여줌
FString UBTTask_EnemyAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Activate: %s"),
		AttackAbilityClass ? *AttackAbilityClass->GetName() : TEXT("None"));
}