#include "Character/AnimNotify/LCEffectNotify.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/BaseCharacter.h"

void ULCEffectNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    AActor* Owner = MeshComp->GetOwner();
    if (!IsValid(Owner))
    {
        return;
    }
    ABaseCharacter* Player = Cast<ABaseCharacter>(Owner);
    // 캐릭터로 캐스팅
    if (!IsValid(Player))
    {
        return;
    }
    if (!NiagaraEffect || !MeshComp)
    {
        return;
    }

    UWorld* World = MeshComp->GetWorld();
    if (World)
    {
        if (!Owner)
        {
            return;
        }

        FVector BaseLocation = Owner->GetActorLocation();
        FVector Forward = Owner->GetActorForwardVector();
        FVector Right = Owner->GetActorRightVector();
        FVector Up = Owner->GetActorUpVector();

        // 원하는 오프셋: 예를 들어 앞 100, 오른쪽 50, 위 30
        FVector Offset = Forward * 150.f;

        FVector SpawnLocation = BaseLocation + Offset;
        FRotator SpawnRotation = Owner->GetActorRotation();


        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            NiagaraEffect,
            SpawnLocation,
            SpawnRotation
        );
    }
}
