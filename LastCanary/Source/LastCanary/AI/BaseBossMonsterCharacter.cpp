#include "AI/BaseBossMonsterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ABaseBossMonsterCharacter::ABaseBossMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    NavGenerationradius = 1000.0f;
    NavRemovalradius = 500.0f;

    NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
    NavInvoker->SetGenerationRadii(NavGenerationradius, NavRemovalradius);
}

bool ABaseBossMonsterCharacter::RequestAttack()
{
    if (!HasAuthority()) return false;

    const float Now = GetWorld()->GetTimeSeconds();

    // 강공격 우선
    if (StrongAttackMontage &&
        FMath::FRand() < StrongAttackChance &&
        (Now - LastStrongTime) >= StrongAttackCooldown)
    {
        LastStrongTime = Now;
        PlayStrongAttack();
        return true;
    }

    // 일반 공격
    if (NormalAttackMontage &&
        (Now - LastNormalTime) >= NormalAttackCooldown)
    {
        LastNormalTime = Now;
        PlayNormalAttack();
        return true;
    }

    return false;
}

void ABaseBossMonsterCharacter::PlayNormalAttack()
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->Montage_Play(NormalAttackMontage);
        // End delegate 바인딩
        Anim->OnMontageEnded.AddDynamic(this, &ABaseBossMonsterCharacter::OnAttackMontageEnded);
    }
}

void ABaseBossMonsterCharacter::PlayStrongAttack()
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->Montage_Play(StrongAttackMontage);
        Anim->OnMontageEnded.AddDynamic(this, &ABaseBossMonsterCharacter::OnAttackMontageEnded);
    }
}

void ABaseBossMonsterCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // 파생 클래스에서 이펙트나 다음 행동 트리거용으로 override 가능
}

void ABaseBossMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABaseBossMonsterCharacter, Rage);
}
