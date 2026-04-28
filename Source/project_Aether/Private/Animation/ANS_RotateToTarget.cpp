
#include "Animation/ANS_RotateToTarget.h"
#include "Gameplay/IAnimationInterface.h"


void UANS_RotateToTarget::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	CachedOwner = nullptr;
	CachedTarget = nullptr;

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner) 
		return;

	CachedOwner = Owner;

	IAnimationInterface* AnimChar = Cast<IAnimationInterface>(Owner);
	if (!AnimChar) 
		return;

	AActor* Target = AnimChar->GetLockedOnTarget();
	if (!Target)
		Target = AnimChar->GetNearestTarget();

	CachedTarget = Target;
	UE_LOG(LogTemp, Log, TEXT("[ANS_RotateToTarget] NotifyBegin: Owner=%s, Target=%s"), 
		*Owner->GetName(), Target ? *Target->GetName() : TEXT("None"));
}

void UANS_RotateToTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!CachedOwner.IsValid() || !CachedTarget.IsValid()) 
		return;

	FVector ToTarget = (CachedTarget->GetActorLocation() - CachedOwner->GetActorLocation());
	ToTarget.Z = 0.f;
	if (ToTarget.IsNearlyZero()) 
		return;

	FRotator NewRot = FMath::RInterpTo(
		CachedOwner->GetActorRotation(),
		ToTarget.GetSafeNormal().Rotation(),
		FrameDeltaTime,
		RotationSpeed);

	CachedOwner->SetActorRotation(NewRot);
	UE_LOG(LogTemp, Log, TEXT("회전중"));
}

