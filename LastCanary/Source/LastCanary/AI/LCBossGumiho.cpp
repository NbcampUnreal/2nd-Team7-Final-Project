#include "AI/LCBossGumiho.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/EngineTypes.h"
#include "Math/UnrealMathUtility.h"
#include "AI/LCBaseBossAIController.h"
#include "Character/BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/Summon/Illusion.h"

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
    if (!HasAuthority()) return;

    Super::UpdateRage(DeltaSeconds);

    // (1) 살아있는 환영 수에 비례해 Rage 증가
    int32 IllCount = IllusionActors.Num();

    if (IllCount > 0)
    {
        const float Delta = IllusionRagePerSecond * IllCount * DeltaSeconds;
        AddRage(Delta);
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


    // (6) 버프 수치 원상복구
    GetCharacterMovement()->MaxWalkSpeed /= BerserkSpeedMultiplier;
    NormalAttackDamage /= BerserkDamageMultiplier;
}

void ALCBossGumiho::OnRep_IsBerserk()
{
    Super::OnRep_IsBerserk();

    // 클라이언트 연출: 광폭화 시작 시 이펙트 및 사운드 재생
    if (bIsBerserk)
    {
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
    else
    {
        // (선택) 광폭화 해제 시 클라이언트 연출 추가 가능
    }
}


void ALCBossGumiho::SpawnIllusions()
{
    if (!HasAuthority() || !IllusionClass) return;

    int32 CurrentCount = IllusionActors.Num();
    if (CurrentCount >= NumIllusions)
    {
        return;
    }

    int32 ToSpawn = NumIllusions - CurrentCount;

    FActorSpawnParameters Params;
    Params.Owner = this;                  // ← 여기서 Owner 설정
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < ToSpawn; ++i)
    {
        FVector Loc = GetActorLocation() + FMath::RandPointInBox(FBox(FVector(-600, -600, 0), FVector(600, 600, 0)));
        if (AIllusion* Ill = Cast<AIllusion>(GetWorld()->SpawnActor<AIllusion>(
            IllusionClass, Loc, GetActorRotation(), Params)))
        {
            Ill->OnDestroyed.AddDynamic(this, &ALCBossGumiho::OnIllusionDestroyed);
            IllusionActors.Add(Ill);

            // ← 여기서 반드시 보스 자신을 알려줍니다!
            Ill->SetBossOwner(this);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[Gumiho] SpawnIllusions: %d마리 소환 (현재 %d/%d)"), ToSpawn, IllusionActors.Num(), NumIllusions);
}

void ALCBossGumiho::OnIllusionDestroyed(AActor* DestroyedActor)
{
    IllusionActors.Remove(DestroyedActor);
    AddRage(-IllusionDeathPenalty);
}

void ALCBossGumiho::ExecuteTailStrike()
{
    if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Tail Strike executed"));

    FVector O = GetActorLocation();
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(TailStrikeRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, O, O, FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& H : Hits)
        {
            if (auto* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                UGameplayStatics::ApplyDamage(C, TailStrikeDamage, GetController(), this, nullptr);
            }
        }
    }
}

void ALCBossGumiho::ExecuteFoxfireVolley()
{
    if (!HasAuthority() || !FoxfireClass) return;

    UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Foxfire Volley executed"));

    // 가로 방사 반경
    const float FoxfireRadius = 600.f;
    // Z축 분포 (스폰 높이 범위)
    const float MinZ = 50.f;
    const float MaxZ = 150.f;

    for (int32 i = 0; i < FoxfireCount; ++i)
    {
        // 0 ~ 2π 랜덤 각도
        const float Angle = FMath::RandRange(0.f, 2 * PI);
        // XY 평면에 원형 분포
        FVector OffsetXY = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * FoxfireRadius;
        // Z는 따로 랜덤
        OffsetXY.Z = FMath::RandRange(MinZ, MaxZ);

        const FVector SpawnLoc = GetActorLocation() + OffsetXY;
        const FRotator SpawnRot = OffsetXY.Rotation();

        GetWorld()->SpawnActor<AActor>(FoxfireClass, SpawnLoc, SpawnRot);
    }
}

void ALCBossGumiho::PerformIllusionSwap()
{
    if (!HasAuthority() || IllusionActors.Num() == 0) return;

    // (1) 환영 중 하나를 랜덤 선택
    int32 IllIdx = FMath::RandRange(0, IllusionActors.Num() - 1);
    AActor* Ill = IllusionActors[IllIdx];
    if (!Ill) return;

    // (2) 반경 내 SweepMulti로 플레이어 Pawn 수집
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(IllusionSwapRadius);

    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits,
        GetActorLocation(),
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );
    if (!bHit) return;

    // (3) 플레이어 Pawn 필터링
    TArray<APawn*> ValidPlayers;
    for (auto& Hit : Hits)
    {
        APawn* P = Cast<APawn>(Hit.GetActor());
        if (P && P->IsPlayerControlled())
        {
            ValidPlayers.Add(P);
        }
    }
    if (ValidPlayers.Num() == 0) return;

    // (4) 플레이어 중 하나 랜덤 선택
    int32 PlyIdx = FMath::RandRange(0, ValidPlayers.Num() - 1);
    APawn* TargetPlayer = ValidPlayers[PlyIdx];
    if (!TargetPlayer) return;

    // (5) 위치 스왑
    const FVector IllLoc = Ill->GetActorLocation();
    const FVector PlayerLoc = TargetPlayer->GetActorLocation();

    Ill->SetActorLocation(PlayerLoc);
    TargetPlayer->SetActorLocation(IllLoc);

    UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Swapped Illusion[%s] with Player[%s]"),
        *Ill->GetName(), *TargetPlayer->GetName());
}

void ALCBossGumiho::ExecuteCharmGaze()
{
    if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Charm Gaze executed"));

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(CharmRadius);

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        CharmRadius,
        16,
        FColor::Red,
        false,
        2.0f,
        0,
        5.0f
    );

    if (GetWorld()->SweepMultiByChannel(
        Hits,
        GetActorLocation(),
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere))
    {
        for (auto& H : Hits)
        {
            if (ABaseCharacter* P = Cast<ABaseCharacter>(H.GetActor()))
            {
                const FName CharmTag = FName("Charmed");
                if (!P->Tags.Contains(CharmTag))
                {
                    // 1) 태그 추가
                    P->Tags.Add(CharmTag);
                    UE_LOG(LogTemp, Log, TEXT("[Gumiho] %s is Charmed"), *P->GetName());

                    // 2) 일정 시간 후 태그 제거
                    FTimerHandle UnusedHandle;
                    FTimerDelegate RemoveDel = FTimerDelegate::CreateLambda([P, CharmTag]()
                        {
                            if (IsValid(P))
                            {
                                P->Tags.Remove(CharmTag);
                                UE_LOG(LogTemp, Log, TEXT("[Gumiho] %s is Uncharmed"), *P->GetName());
                            }
                        });
                    GetWorldTimerManager().SetTimer(
                        UnusedHandle,
                        RemoveDel,
                        CharmInterval,
                        false
                    );
                }
            }
        }
    }
}

void ALCBossGumiho::ExecuteNineTailBurst()
{
    if (HasAuthority() && !bHasUsedNineTail && Rage <= 0.5f)
    {
		UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Nine-Tail Burst activated"));

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

    if(TargetDistance > 600.f)
		return false; // 너무 멀면 공격하지 않음

    AActor* Target = nullptr;
    if (auto* AC = Cast<AAIController>(GetController()))
        Target = Cast<AActor>(AC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));

    struct FEntry { float Weight; TFunction<void()> Action; };
    TArray<FEntry> Entries;

    // Nine-Tail Burst
    if (!bHasUsedNineTail && Rage >= 0.5f)
    {
        Entries.Add({ 4.f, [this]() {
            bHasUsedNineTail = true;
            ExecuteNineTailBurst();
        } });
    }

    // Foxfire Volley (거리 무관, 쿨다운만)
    {
        float CD = FoxfireInterval;
        if (Target && Now - LastFoxfireTime >= CD)
        {
            Entries.Add({ 3.f, [this, Now]() {
                LastFoxfireTime = Now;
                ExecuteFoxfireVolley();
            } });
        }
    }

    // Tail Strike
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

    // Illusion Swap
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

    // Random 선택
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
