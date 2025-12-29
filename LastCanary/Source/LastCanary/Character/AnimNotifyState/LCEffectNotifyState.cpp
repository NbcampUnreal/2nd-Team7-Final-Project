#include "Character/AnimNotifyState/LCEffectNotifyState.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Character/BaseCharacter.h"

void ULCEffectNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (!IsValid(MeshComp) || !IsValid(NiagaraEffect)) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;

	FVector SpawnLocation = Owner->GetActorLocation() +
		Owner->GetActorForwardVector() * Offset.X +
		Owner->GetActorRightVector() * Offset.Y +
		Owner->GetActorUpVector() * Offset.Z;

	FRotator SpawnRotation = Owner->GetActorRotation();

	// Spawn Niagara Component attached to actor (for auto-destroying when ended)
	SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		Owner,
		NiagaraEffect,
		SpawnLocation,
		SpawnRotation,
		FVector(1.f),
		true,  // bAutoDestroy
		true,  // bAutoActivate
		ENCPoolMethod::None,
		true   // bPreCullCheck
	);
}

void ULCEffectNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (IsValid(SpawnedEffect))
	{
		SpawnedEffect->Deactivate();
		SpawnedEffect = nullptr;
	}
}
