#include "AI/BTTask_EnemyAttack.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Character/BaseEnemyCharacter.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
	NodeName = TEXT("Enemy Attack");
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

	const bool bActivated = ASC->TryActivateAbilityByClass(AttackAbilityClass);
	return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_EnemyAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Activate: %s"),
		AttackAbilityClass ? *AttackAbilityClass->GetName() : TEXT("None"));
}