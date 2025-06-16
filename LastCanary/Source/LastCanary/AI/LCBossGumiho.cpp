#include "AI/LCBossGumiho.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"
#include "Math/UnrealMathUtility.h"
#include "AI/LCBaseBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NiagaraFunctionLibrary.h"

ALCBossGumiho::ALCBossGumiho()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ALCBossGumiho::BeginPlay()
{
    Super::BeginPlay();

    // Illusion Spawn
    GetWorldTimerManager().SetTimer(IllusionTimerHandle, this, &ALCBossGumiho::SpawnIllusions, IllusionInterval, true);

    // Charm Gaze
    GetWorldTimerManager().SetTimer(CharmTimerHandle, this, &ALCBossGumiho::ExecuteCharmGaze, CharmInterval, true);
}

void ALCBossGumiho::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateRage(DeltaTime);

    if (auto* AICon = Cast<ALCBaseBossAIController>(GetController()))
    {
        if (auto* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsFloat(TEXT("RagePercent"), Rage / MaxRage);
            BB->SetValueAsBool(TEXT("IsBerserkMode"), bIsBerserk);
        }
    }
}

void ALCBossGumiho::UpdateRage(float DeltaSeconds)
{
    Super::UpdateRage(DeltaSeconds);

    // (1) 살아있는 환영 수에 비례해 Rage 증가
    int32 IllCount = IllusionActors.Num();
    if (IllCount > 0)
    {
        AddRage(IllusionRagePerSecond * IllCount * DeltaSeconds);
    }

    // (2) 매혹된 플레이어 수에 비례해 Rage 감소 (SweepMultiByChannel 사용)
    TArray<FHitResult> HitResults;
    FVector Origin = GetActorLocation();
    FCollisionShape Sphere = FCollisionShape::MakeSphere(CharmRadius);

    // 시작과 끝을 동일하게 주면, 단순 범위 검사처럼 동작
    if (GetWorld()->SweepMultiByChannel(
        HitResults,
        Origin,
        Origin,
        FQuat::Identity,
        ECC_Pawn,
        Sphere))
    {
        int32 CharmCount = 0;
        for (const auto& Hit : HitResults)
        {
            if (APawn* P = Cast<APawn>(Hit.GetActor()))
            {
                if (P->Tags.Contains(FName("Charmed")))
                {
                    CharmCount++;
                }
            }
        }
        if (CharmCount > 0)
        {
            AddRage(-CharmRagePerSecond * CharmCount * DeltaSeconds);
        }
    }
}

void ALCBossGumiho::AddRage(float Amount)
{
    // Rage 갱신
    Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);

    // MaxRage 도달 시 광폭화
    if (Rage >= MaxRage && !bIsBerserk)
    {
        EnterBerserkState();
    }
}

void ALCBossGumiho::EnterBerserkState()
{
    Super::EnterBerserkState();
    UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Enter Berserk State"));

    // (1) 상태 이상 면역 처리 (예: StatusComponent가 있다면)
    // if (StatusComponent) StatusComponent->SetImmune(true);

    // (2) 이동 속도·공격력 버프 적용
    GetCharacterMovement()->MaxWalkSpeed *= BerserkSpeedMultiplier;
    NormalAttackDamage *= BerserkDamageMultiplier;
}

void ALCBossGumiho::StartBerserk()
{
    Super::StartBerserk();  // bIsBerserk = true, 멀티캐스트 호출 포함

    // (3) 광폭화 시 이펙트
    if (BerserkEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            BerserkEffectFX,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    // (4) 사운드 재생
    if (BerserkSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            BerserkSound,
            GetActorLocation()
        );
    }
}

void ALCBossGumiho::StartBerserk(float Duration)
{
    Super::StartBerserk(Duration);

    // 2) 클라이언트 연출: 위와 동일하지만 Super 호출은 별도
    if (BerserkEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            BerserkEffectFX,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    if (BerserkSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            BerserkSound,
            GetActorLocation()
        );
    }
}

void ALCBossGumiho::EndBerserk()
{
    Super::EndBerserk();
    UE_LOG(LogTemp, Warning, TEXT("[Gumiho] End Berserk State"));

    // (5) 면역 해제
    // if (StatusComponent) StatusComponent->SetImmune(false);

    // (6) 버프 수치 원상복구
    GetCharacterMovement()->MaxWalkSpeed /= BerserkSpeedMultiplier;
    NormalAttackDamage /= BerserkDamageMultiplier;
}

void ALCBossGumiho::SpawnIllusions()
{
    if (!HasAuthority() || !IllusionClass) return;

    // 이전 환영 정리
    for (AActor* I : IllusionActors)
    {
        if (I)
        {
            I->OnDestroyed.RemoveDynamic(this, &ALCBossGumiho::OnIllusionDestroyed);
            I->Destroy();
        }
    }
    IllusionActors.Empty();

    // 새 환영 생성
    for (int32 i = 0; i < NumIllusions; ++i)
    {
        FVector Loc = GetActorLocation() + FMath::RandPointInBox(FBox(FVector(-600, -600, 0), FVector(600, 600, 0)));
        if (AActor* Ill = GetWorld()->SpawnActor<AActor>(IllusionClass, Loc, GetActorRotation()))
        {
            Ill->OnDestroyed.AddDynamic(this, &ALCBossGumiho::OnIllusionDestroyed);
            IllusionActors.Add(Ill);
        }
    }
}

void ALCBossGumiho::OnIllusionDestroyed(AActor* DestroyedActor)
{
    IllusionActors.Remove(DestroyedActor);
    AddRage(-IllusionDeathPenalty);
}

void ALCBossGumiho::ExecuteTailStrike()
{
    if (!HasAuthority()) return;

    FVector O = GetActorLocation();
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(TailStrikeRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, O, O, FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& H : Hits)
        {
            if (ACharacter* C = Cast<ACharacter>(H.GetActor()))
            {
                UGameplayStatics::ApplyDamage(C, TailStrikeDamage, GetController(), this, nullptr);
            }
        }
    }
}

void ALCBossGumiho::ExecuteFoxfireVolley()
{
    if (!HasAuthority() || !FoxfireProjectileClass) return;

    for (int32 i = 0; i < FoxfireCount; ++i)
    {
        FVector Dir = FMath::VRand();
        Dir.Z = FMath::Abs(Dir.Z);
        FVector SpawnLoc = GetActorLocation() + Dir * 200.f + FVector(0, 0, 100);
        FRotator SpawnRot = Dir.Rotation();
        GetWorld()->SpawnActor<AActor>(FoxfireProjectileClass, SpawnLoc, SpawnRot);
    }
}

void ALCBossGumiho::PerformIllusionSwap()
{
    if (!HasAuthority() || IllusionActors.Num() == 0) return;

    int32 Idx = FMath::RandRange(0, IllusionActors.Num() - 1);
    AActor* Ill = IllusionActors[Idx];
    if (!Ill) return;

    FVector BossLoc = GetActorLocation();
    FVector IllLoc = Ill->GetActorLocation();

    SetActorLocation(IllLoc);
    Ill->SetActorLocation(BossLoc);
}

void ALCBossGumiho::ExecuteCharmGaze()
{
    if (!HasAuthority()) return;

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(CharmRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& H : Hits)
        {
            if (APawn* P = Cast<APawn>(H.GetActor()))
            {
                if (!P->Tags.Contains(FName("Charmed")))
                {
                    P->Tags.Add(FName("Charmed"));
                    UE_LOG(LogTemp, Log, TEXT("[Gumiho] %s is Charmed"), *P->GetName());
                }
            }
        }
    }
}

void ALCBossGumiho::ExecuteNineTailBurst()
{
    if (HasAuthority() && !bHasUsedNineTail && Rage <= 0.5f)
    {
        DealDamageInRange(NormalAttackDamage * 2.f);
        bHasUsedNineTail = true;
        UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Nine-Tail Burst executed"));
    }
}

void ALCBossGumiho::OnRep_DivineGrace()
{
    if (bIsDivineGrace)
    {
        GetCharacterMovement()->MaxWalkSpeed *= 1.5f;
        UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Divine Grace activated"));
    }
}

void ALCBossGumiho::Multicast_StartDivineGrace_Implementation()
{
    OnRep_DivineGrace();
    // 추가 이펙트/사운드 재생 가능
}

bool ALCBossGumiho::RequestAttack(float TargetDistance)
{
    if (!HasAuthority()) return false;

    const float Now = GetWorld()->GetTimeSeconds();

    // 1) 타겟 가져오기 (블랙보드에 TargetActor 키 사용 가정)
    AActor* Target = nullptr;
    if (auto* AC = Cast<AAIController>(GetController()))
        Target = Cast<AActor>(AC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));

    struct FEntry { float Weight; TFunction<void()> Action; };
    TArray<FEntry> Entries;

    // 2) Nine-Tail Burst: Rage 값 일정 이상시
    if (!bHasUsedNineTail && Rage >= 0.5f)
    {
        Entries.Add({ 4.f, [this]() {
            bHasUsedNineTail = true;
            ExecuteNineTailBurst();
        } });
    }

    // 3) Foxfire Volley: 원거리에서, 쿨다운 지난 경우
    {
        float CD = FoxfireInterval;
        if (Target && TargetDistance > TailStrikeRadius && Now - LastFoxfireTime >= CD)
        {
            Entries.Add({ 3.f, [this, Now]() {
                LastFoxfireTime = Now;
                ExecuteFoxfireVolley();
            } });
        }
    }

    // 4) Tail Strike: 근거리에서, 쿨다운 지난 경우
    {
        float CD = TailStrikeCooldown;
        if (Target && TargetDistance <= TailStrikeRadius && Now - LastTailStrikeTime >= CD)
        {
            Entries.Add({ 2.f, [this, Now]() {
                LastTailStrikeTime = Now;
                ExecuteTailStrike();
            } });
        }
    }

    // 5) Illusion Swap: fallback
    {
        float CD = IllusionSwapInterval;
        if (Now - LastIllusionSwapTime >= CD)
        {
            Entries.Add({ 1.f, [this, Now]() {
                LastIllusionSwapTime = Now;
                PerformIllusionSwap();
            } });
        }
    }

    // 6) 가중치 합산 및 랜덤 선택
    float TotalW = 0.f;
    for (auto& E : Entries) TotalW += E.Weight;
    if (TotalW <= 0.f) return false;

    float Pick = FMath::FRandRange(0.f, TotalW), Acc = 0.f;
    for (auto& E : Entries)
    {
        Acc += E.Weight;
        if (Pick <= Acc)
        {
            E.Action();
            return true;
        }
    }

    return false;
}



void ALCBossGumiho::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossGumiho, bIsDivineGrace);
}
