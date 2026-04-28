#include "Abilites/GA_Jump.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_Jump::UGA_Jump()
{
	FGameplayTagContainer AssetTagContainer;
	AssetTagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Jump")));
	SetAssetTags(AssetTagContainer);
	// AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Jump")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Jumping")));
}
void UGA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_LOG(LogTemp, Warning, TEXT("UGA_Jump::ActivateAbility 실행."));
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character && Character->GetCharacterMovement())
	{
		Character->ACharacter::Jump();
		UE_LOG(LogTemp, Warning, TEXT("캐릭터 점프 실행."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}
void UGA_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}