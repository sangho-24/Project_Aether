
#include "Character/EnemyStoneGolem.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyStoneGolem::AEnemyStoneGolem()
{
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;
}