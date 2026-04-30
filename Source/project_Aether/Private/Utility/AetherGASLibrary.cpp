
#include "Utility/AetherGASLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameplayCueManager.h"

void UAetherGASLibrary::ExecuteGameplayCueWithHitResult(AActor* Instigator, AActor* TargetActor, const FGameplayTag& CueTag,
	const FHitResult& HitResult)
{
	if (!TargetActor || !CueTag.IsValid())
		return;

	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint.IsZero()
		? TargetActor->GetActorLocation()
		: FVector(HitResult.ImpactPoint);
	CueParams.Normal = HitResult.ImpactNormal.IsZero()
		? (Instigator ? Instigator->GetActorForwardVector() : FVector::UpVector)
		: FVector(HitResult.ImpactNormal);
	CueParams.PhysicalMaterial = HitResult.PhysMaterial;
	CueParams.Instigator = Cast<APawn>(Instigator);
	CueParams.EffectCauser = Instigator;

	FGameplayEffectContextHandle ContextHandle = FGameplayEffectContextHandle(
		UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
	ContextHandle.AddHitResult(HitResult);
	CueParams.EffectContext = ContextHandle;

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC)
	{
		TargetASC->ExecuteGameplayCue(CueTag, CueParams);
	}
	else
	{
		if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
		{
			CueManager->HandleGameplayCue(TargetActor, CueTag,
				EGameplayCueEvent::Executed, CueParams);
		}
	}
}
