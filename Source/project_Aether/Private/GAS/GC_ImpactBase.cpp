
#include "GAS/GC_ImpactBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

bool UGC_ImpactBase::OnExecute_Implementation(
	AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget || !MyTarget->GetWorld())
		return false;

	// AetherGASLibrary에서 이미 HitResult 기반으로 채워서 전달하므로 그대로 사용
	const FVector Location = Parameters.Location;
	const FRotator Rotation = Parameters.Normal.Rotation();
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			MyTarget->GetWorld(),HitEffect, Location, Rotation, HitEffectScale,
			true,true, ENCPoolMethod::AutoRelease);
	}
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(MyTarget,HitSound, Location);
	}
	return true;
}