#include "AI/BaseBossMonsterCharacter.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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
    if (!HasAuthority())
        return false;

    const float Now = GetWorld()->GetTimeSeconds();

    // (1) 강공격 우선
    if (StrongAttackMontage &&
        FMath::FRand() < StrongAttackChance &&
        (Now - LastStrongTime) >= StrongAttackCooldown)
    {
        LastStrongTime = Now;
        // Berserk 상태라면 DamageMultiplier_Berserk을 참고하여 공격 세기를 조절할 수 있음
        PlayStrongAttack();
        return true;
    }

    // (2) 일반 공격
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
        Anim->OnMontageEnded.AddDynamic(this, &ABaseBossMonsterCharacter::OnAttackMontageEnded);
    }
}


void ABaseBossMonsterCharacter::PlayStrongAttack()
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        // Berserk 상태라면, 몽타주 재생 속도를 빠르게 할 수도 있습니다. 예시:
        // float PlayRate = bIsBerserk ? 1.5f : 1.0f;
        // Anim->Montage_Play(StrongAttackMontage, PlayRate);
        Anim->Montage_Play(StrongAttackMontage);
        Anim->OnMontageEnded.AddDynamic(this, &ABaseBossMonsterCharacter::OnAttackMontageEnded);
    }
}


void ABaseBossMonsterCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // 예: 파생 클래스에서 이곳을 override하여 이펙트/다음 행동을 트리거할 수 있음
}

// Tick이나 다른 타이밍에 호출하여 Rage를 갱신하고 싶다면 여기서 Berserk 배수를 적용
void ABaseBossMonsterCharacter::UpdateRage(float DeltaSeconds)
{

}


void ABaseBossMonsterCharacter::EnterBerserkState()
{
    if (!HasAuthority())
        return;

    // 이미 Berserk 중이거나 쿨타임 중이면 무시
    if (bIsBerserk || GetWorldTimerManager().IsTimerActive(BerserkCooldownTimerHandle))
    {
        return;
    }

    StartBerserk();
}


void ABaseBossMonsterCharacter::StartBerserk()
{
    bIsBerserk = true;

    // 서버가 결정한 Berserk 시작을 클라이언트 전체에 알림
    Multicast_StartBerserk();

    // Berserk 지속 시간이 지나면 EndBerserk 호출 예약
    GetWorldTimerManager().SetTimer(
        BerserkTimerHandle,
        this,
        &ABaseBossMonsterCharacter::EndBerserk,
        BerserkDuration,
        false
    );

    // Berserk 종료 후 BerserkCooldown만큼 대기시키기 위한 타이머(콜백 람다 식으로 별도 로직 없음)
    GetWorldTimerManager().SetTimer(
        BerserkCooldownTimerHandle,
        FTimerDelegate::CreateLambda([]() {}),
        BerserkDuration + BerserkCooldown,
        false
    );
}


void ABaseBossMonsterCharacter::EndBerserk()
{
    if (!bIsBerserk)
        return;

    bIsBerserk = false;
    Multicast_EndBerserk();
}


void ABaseBossMonsterCharacter::OnRep_IsBerserk()
{
    if (bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Berserk] 클라이언트: Berserk 시작"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Berserk] 클라이언트: Berserk 종료"));
    }
}


void ABaseBossMonsterCharacter::Multicast_StartBerserk_Implementation()
{
    OnRep_IsBerserk();
    UE_LOG(LogTemp, Warning, TEXT("→ Multicast_StartBerserk 호출 (서버→클라이언트 전체)"));
}


void ABaseBossMonsterCharacter::Multicast_EndBerserk_Implementation()
{
    OnRep_IsBerserk();
    UE_LOG(LogTemp, Warning, TEXT("→ Multicast_EndBerserk 호출 (서버→클라이언트 전체)"));
}


void ABaseBossMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Rage 복제
    DOREPLIFETIME(ABaseBossMonsterCharacter, Rage);

    // Berserk 상태 복제
    DOREPLIFETIME(ABaseBossMonsterCharacter, bIsBerserk);
}
