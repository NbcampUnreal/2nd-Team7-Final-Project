#include "AI/LCBossLich.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

ALCBossLich::ALCBossLich()
{
    // 기본 몽타주 설정은 부모에서 상속
}

void ALCBossLich::BeginPlay()
{
    Super::BeginPlay();

    // 언데드 소환 타이머 시작
    if (UndeadClasses.Num() > 0)
    {
        GetWorldTimerManager().SetTimer(
            UndeadSpawnTimerHandle,
            this, &ALCBossLich::SpawnUndeadMinion,
            UndeadSpawnInterval, true);
    }

    // 마나 파동 타이머 시작
    GetWorldTimerManager().SetTimer(
        ManaPulseTimerHandle,
        this, &ALCBossLich::ExecuteManaPulse,
        ManaPulseInterval, true);
}

void ALCBossLich::SpawnUndeadMinion()
{
    if (!HasAuthority() || UndeadClasses.Num() == 0) return;

    // 랜덤 클래스 선택
    int32 Index = FMath::RandRange(0, UndeadClasses.Num() - 1);
    TSubclassOf<APawn> Cls = UndeadClasses[Index];
    if (!Cls) return;

    // 보스 주변 랜덤 위치
    FVector Loc = GetActorLocation()
        + FMath::RandPointInBox(FBox(FVector(-500, -500, 0), FVector(500, 500, 0)));
    FRotator Rot = FRotator::ZeroRotator;

    FActorSpawnParameters Params;
    Params.Owner = this;
    GetWorld()->SpawnActor<APawn>(Cls, Loc, Rot, Params);
}

void ALCBossLich::ExecuteManaPulse()
{
    if (!HasAuthority()) return;

    // 범위 내 플레이어에 데미지
    DealDamageInRange(ManaPulseDamage);

    // (선택) 시각 효과, 이펙트 추가 가능
}

void ALCBossLich::UpdateRage(float DeltaSeconds)
{
    Super::UpdateRage(DeltaSeconds);

    // Rage가 일정치 이상(PHYLACTERY 파괴 시) 즉시 Berserk 진입
    CheckPhylactery();
}

void ALCBossLich::CheckPhylactery()
{
    if (bIsBerserk || !PhylacteryActor) return;

    //// PhylacteryActor가 파괴("IsPendingKill")되면 즉시 Berserk
    //if (PhylacteryActor->IsPendingKill())
    //{
    //    StartBerserk();
    //}
}

void ALCBossLich::OnRep_IsBerserk()
{
    Super::OnRep_IsBerserk();

    if (bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Lich] Phylactery 파괴! Berserk 상태 진입"));
        // 추가 이펙트나 사운드 재생
    }
}

void ALCBossLich::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    Super::OnAttackMontageEnded(Montage, bInterrupted);

    // 일반 공격 완료 시 흡수량만큼 힐 (데미지의 50%)
    if (Montage == NormalAttackMontage)
    {
        float HealAmount = NormalAttackDamage * 0.5f;
        UGameplayStatics::ApplyDamage(
            this,
            -HealAmount,               // 음수 대미지 → 힐
            GetController(),
            this,
            UDamageType::StaticClass()
        );
        UE_LOG(LogTemp, Log, TEXT("[Lich] Soul Absorb: %f 회복"), HealAmount);
    }
}
