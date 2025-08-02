#include "Character/AnimNotify/LCSoundNotify.h"
#include "Kismet/GameplayStatics.h"
#include "Character/BaseCharacter.h"

void ULCSoundNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
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
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(Player, SoundToPlay, Player->GetActorLocation());
    }
}
