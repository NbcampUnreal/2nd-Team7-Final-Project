#include "AI/LCBossMonsterCharacter.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "AI/LCBaseBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"

ALCBossMonsterCharacter::ALCBossMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ALCBossMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurScale = MaxScale;
    SetActorScale3D(FVector(CurScale));
}

void ALCBossMonsterCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 서버에서만, 체력이 0 이하가 아니면 로직 실행
    if (!HasAuthority())
    {
        return;
    }

    const bool bLooked = IsLookedAtByAnyPlayer();

    UpdateRage(DeltaSeconds, bLooked);
    UpdateScale(DeltaSeconds, bLooked);
    TryTriggerDarkness();

    if (ALCBaseBossAIController* AICon = Cast<ALCBaseBossAIController>(GetController()))
    {
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsFloat(TEXT("RagePercent"), Rage / MaxRage);
            BB->SetValueAsBool(TEXT("IsDarknessActive"), bDarknessActive);
        }
    }
}

bool ALCBossMonsterCharacter::IsLookedAtByAnyPlayer() const
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (IsPlayerLooking(It->Get()))
        {
            return true;
        }
    }
    return false;
}

bool ALCBossMonsterCharacter::IsPlayerLooking(APlayerController* PC) const
{
    if (!PC) return false;

    FVector ViewLoc;
    FRotator ViewRot;
    PC->GetPlayerViewPoint(ViewLoc, ViewRot);

    const FVector DirToBoss = (GetActorLocation() - ViewLoc).GetSafeNormal();
    const float Deg = FMath::RadiansToDegrees(
        acosf(FVector::DotProduct(ViewRot.Vector(), DirToBoss))
    );

    return Deg <= LookAngleDeg && PC->LineOfSightTo(this, ViewLoc, true);
}

void ALCBossMonsterCharacter::UpdateRage(float DeltaSeconds, bool bLooked)
{
    const float Delta = (bLooked ? -RageLossPerSec : RageGainPerSec) * DeltaSeconds;
    Rage = FMath::Clamp(Rage + Delta, 0.f, MaxRage);
}

void ALCBossMonsterCharacter::UpdateScale(float DeltaSeconds, bool bLooked)
{
    const float Target = bLooked ? MinScale : MaxScale;
    CurScale = FMath::FInterpTo(CurScale, Target, DeltaSeconds, ScaleInterpSpeed);
    SetActorScale3D(FVector(CurScale));
}

void ALCBossMonsterCharacter::TryTriggerDarkness()
{
    if (bDarknessActive || Rage < DarknessRage)
    {
        return;
    }

    bDarknessActive = true;
    Multicast_StartDarkness();

    GetWorldTimerManager().SetTimer(
        DarknessTimer, this,
        &ALCBossMonsterCharacter::EndDarkness,
        DarknessDuration, false
    );
}

void ALCBossMonsterCharacter::EndDarkness()
{
    if (!bDarknessActive) return;

    bDarknessActive = false;
    Multicast_EndDarkness();
}

bool ALCBossMonsterCharacter::RequestAttack()
{
    // 서버에서만, 체력이 남아있어야 공격 가능
    if (!HasAuthority())
    {
        return false;
    }

    const float Now = GetWorld()->GetTimeSeconds();

    const bool bDoStrong =
        FMath::FRand() < StrongAttackChance &&
        (Now - LastStrongTime) >= StrongAttackCooldown &&
        Rage / MaxRage >= (DarknessRage / MaxRage);

    if (bDoStrong && StrongAttackMontage)
    {
        LastStrongTime = Now;
        PlayStrong();
        return true;
    }

    if ((Now - LastNormalTime) >= NormalAttackCooldown && NormalAttackMontage)
    {
        LastNormalTime = Now;
        PlayNormal();
        return true;
    }

    return false;
}

void ALCBossMonsterCharacter::PerformAttack()
{
    if (RequestAttack())
    {
        // 공격 성공 시 추가 로직
    }
    else
    {
        // 공격 실패(쿨다운 등) 시
    }
}

void ALCBossMonsterCharacter::PlayNormal()
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->Montage_Play(NormalAttackMontage);
    }
    Rage = FMath::Clamp(Rage + RageGain_Normal, 0.f, MaxRage);
}

void ALCBossMonsterCharacter::PlayStrong()
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->Montage_Play(StrongAttackMontage);
    }

    UGameplayStatics::ApplyRadialDamage(
        this, RageLoss_Strong, GetActorLocation(),
        300.f, nullptr, TArray<AActor*>(),
        this, GetController(), true
    );

    Rage = FMath::Clamp(Rage - RageLoss_Strong, 0.f, MaxRage);
}

void ALCBossMonsterCharacter::OnRep_Rage()
{
    // 클라이언트 UI 업데이트 등
}

void ALCBossMonsterCharacter::OnRep_Scale()
{
    SetActorScale3D(FVector(CurScale));
}

void ALCBossMonsterCharacter::Multicast_StartDarkness_Implementation()
{
    BP_StartDarknessEffect();
}

void ALCBossMonsterCharacter::Multicast_EndDarkness_Implementation()
{
    BP_EndDarknessEffect();
}

void ALCBossMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALCBossMonsterCharacter, Rage);
    DOREPLIFETIME(ALCBossMonsterCharacter, CurScale);
    DOREPLIFETIME(ALCBossMonsterCharacter, bDarknessActive);
}