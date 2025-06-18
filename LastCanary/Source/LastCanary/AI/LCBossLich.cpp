#include "AI/LCBossLich.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/BaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "AI/Projectile/ArcaneBolt.h"
#include "AI/LCBaseBossAIController.h"

ALCBossLich::ALCBossLich()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ALCBossLich::BeginPlay()
{
    Super::BeginPlay();

    if (UndeadClasses.Num())
    {
        GetWorldTimerManager().SetTimer(
            UndeadSpawnTimerHandle,
            this, &ALCBossLich::SpawnUndeadMinion,
            UndeadSpawnInterval, true
        );
    }

    GetWorldTimerManager().SetTimer(
        ManaPulseTimerHandle,
        this, &ALCBossLich::ExecuteManaPulse,
        ManaPulseInterval, true
    );
}

void ALCBossLich::Tick(float DeltaTime)
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

void ALCBossLich::UpdateRage(float DeltaSeconds)
{
    Super::UpdateRage(DeltaSeconds);

    // 살아있는 소환 언데드 수
    int32 AliveCount = SpawnedUndeadMinions.Num();

    if (AliveCount > 0)
    {
        // 초당 UndeadRagePerSecond × Count 만큼 Rage 증가
        AddRage(UndeadRagePerSecond * AliveCount * DeltaSeconds);
    }
    else
    {
        // 소환된 언데드가 하나도 없으면 초당 UndeadRagePerSecond 만큼 Rage 감소
        AddRage(-UndeadRagePerSecond * DeltaSeconds);
    }

    // Death Nova 발동 조건
    if (!bHasUsedDeathNova && Rage >= DeathNovaThreshold)
    {
        bHasUsedDeathNova = true;
        ExecuteDeathNova();
    }
}

void ALCBossLich::AddRage(float Amount)
{
    Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);
    if (Rage >= MaxRage && !bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Lich] Rage MAX → Berserk 진입"));
        EnterBerserkState();
    }
}


// ── 영구 Berserk 진입 시 로그만 ─────────────────
void ALCBossLich::EnterBerserkState()
{
    Super::EnterBerserkState();
    UE_LOG(LogTemp, Warning, TEXT("[Lich] 영구 Berserk 진입"));
}

// ── 서버 로직 수행 후 멀티캐스트 포함 ─────────────────
void ALCBossLich::StartBerserk()
{
    Super::StartBerserk();  // bIsBerserk = true 및 Multicast 호출 포함

    // 이펙트 및 사운드 재생
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

// ── 지속시간 지정 버전 (StartBerserk(float)) ─────────────────
void ALCBossLich::StartBerserk(float Duration)
{
    Super::StartBerserk(Duration);

    // 동일 이펙트·사운드 재생
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

// ── Berserk 종료 시 로그만 (후처리 필요 시 여기에 추가) ─────────────────
void ALCBossLich::EndBerserk()
{
    Super::EndBerserk();
    UE_LOG(LogTemp, Warning, TEXT("[Lich] Berserk 종료"));
}

// ── 레플리케이션 갱신 시에도 클라이언트에서 이펙트·사운드 재생 ─────────────────
void ALCBossLich::OnRep_IsBerserk()
{
    Super::OnRep_IsBerserk();

    if (bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Lich] OnRep → Berserk 이펙트 클라에서 재생"));

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
}

void ALCBossLich::SpawnUndeadMinion()
{
    if (!HasAuthority() || UndeadClasses.Num() == 0) return;

    // --- 소환 제한 체크 ---
    // 이미 살아있는(리스트에 남아있는) 언데드가 최대치 이상이면 스폰하지 않음
    if (SpawnedUndeadMinions.Num() >= MaxUndeadMinions)
        return;

    int32 idx = FMath::RandRange(0, UndeadClasses.Num() - 1);
    auto Cls = UndeadClasses[idx];
    if (!Cls) return;

    FVector Loc = GetActorLocation()
        + FMath::RandPointInBox(FBox(FVector(-500, -500, 0), FVector(500, 500, 0)));
    FActorSpawnParameters Params;
    Params.Owner = this;

    if (APawn* M = GetWorld()->SpawnActor<APawn>(Cls, Loc, FRotator::ZeroRotator, Params))
    {
        // 1) 소환된 리스트에 추가
        SpawnedUndeadMinions.Add(M);

        // 2) 파괴 콜백 바인딩
        M->OnDestroyed.AddDynamic(this, &ALCBossLich::OnUndeadDestroyed);

        // 3) 즉시 Rage 상승 (선택 사항)
        AddRage(UndeadRagePerSecond);
    }
}

void ALCBossLich::OnUndeadDestroyed(AActor* DestroyedActor)
{
    // 소환 리스트에 있던 언데드라면
    if (APawn* Pawn = Cast<APawn>(DestroyedActor))
    {
        if (SpawnedUndeadMinions.Remove(Pawn) > 0)
        {
            // 잡혔을 때 페널티 적용
            AddRage(-UndeadDeathRagePenalty);

            // 더 이상 바인딩 필요 없으니 제거
            Pawn->OnDestroyed.RemoveDynamic(this, &ALCBossLich::OnUndeadDestroyed);
        }
    }
}

void ALCBossLich::ExecuteManaPulse()
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteManaPulse 시작: Damage=%.1f, Radius=%.1f"), ManaPulseDamage, ManaPulseRadius);

    FVector O = GetActorLocation();
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(ManaPulseRadius);

    // 1) Debug: 폭발 위치에 보라색 구 그리기 (2초간, 두께 5)
    DrawDebugSphere(
        GetWorld(),
        O,
        ManaPulseRadius,
        16,
        FColor::Blue,
        false,
        2.0f,
        0,
        5.0f
    );

    if (GetWorld()->SweepMultiByChannel(Hits, O, O, FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& H : Hits)
        {
            if (auto C = Cast<ABaseCharacter>(H.GetActor()))
            {
                UE_LOG(LogTemp, Warning, TEXT("[Lich] ManaPulse → %s 입혔습니다"), *C->GetName());
                UGameplayStatics::ApplyDamage(C, ManaPulseDamage, GetController(), this, nullptr);
            }
        }
    }
}

void ALCBossLich::ExecuteArcaneBolt(AActor* Target)
{
    if (!HasAuthority() || !Target || !ArcaneBoltClass)
        return;

    UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteArcaneBolt 시작 → 대상: %s"), *Target->GetName());

    // (1) 스폰 위치: 캐릭터 위치 + 전방 100 + 위로 200
    const float VerticalOffset = 100.f;
    const float ForwardOffset = 100.f;
    FVector SpawnLoc = GetActorLocation()
        + GetActorForwardVector() * ForwardOffset
        + FVector(0.f, 0.f, VerticalOffset);
    // (2) 회전: 그대로 타겟을 향하게
    FRotator SpawnRot = (Target->GetActorLocation() - SpawnLoc).Rotation();
    FVector Dir = SpawnRot.Vector();

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = Cast<APawn>(GetController()->GetPawn());

    if (AArcaneBolt* Bolt = GetWorld()->SpawnActor<AArcaneBolt>(
        ArcaneBoltClass, SpawnLoc, SpawnRot, Params))
    {
        float Speed = bIsBerserk ? ArcaneBoltSpeed * BerserkCooldownFactor : ArcaneBoltSpeed;
        float Damage = bIsBerserk ? NormalAttackDamage * BerserkDamageFactor : NormalAttackDamage;

        UE_LOG(LogTemp, Warning, TEXT("[Lich] ArcaneBolt 스폰 → Speed=%.1f, Damage=%.1f"), Speed, Damage);
        Bolt->InitProjectile(Dir, Speed, Damage, GetController());
    }
}

void ALCBossLich::ExecuteSoulBind(AActor* Target)
{
    if (!HasAuthority() || !Target) return;

    UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteSoulBind 시작 → 대상: %s"), *Target->GetName());

    if (auto C = Cast<ACharacter>(Target))
    {
        C->DisableInput(nullptr);
        FTimerHandle Handle;
        GetWorldTimerManager().SetTimer(Handle, [C]() {
            if (IsValid(C)) C->EnableInput(nullptr);
            }, 3.f, false);

        if (SoulBindFX)
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SoulBindFX, C->GetActorLocation());
        if (SoulBindSound)
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoulBindSound, C->GetActorLocation());
    }
}

void ALCBossLich::ExecuteSoulAbsorb(AActor* Target)
{
    if (!HasAuthority() || !Target) return;

    UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteSoulAbsorb 시작 → 대상: %s, Damage=%.1f"), *Target->GetName(), SoulAbsorbDamage);

    UGameplayStatics::ApplyDamage(Target, SoulAbsorbDamage, GetController(), this, nullptr);

    if (SoulAbsorbFX)
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SoulAbsorbFX, GetActorLocation());
    if (SoulAbsorbSound)
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoulAbsorbSound, GetActorLocation());
}

void ALCBossLich::ExecuteDeathNova()
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("[Lich] ExecuteDeathNova 시작 → Damage=%.1f (전범위), StunDuration=%.1f"), DeathNovaDamage, DeathNovaStunDuration);

    // (1) 모든 Pawn 가져오기
    TArray<AActor*> AllPawns;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

    for (AActor* Actor : AllPawns)
    {
        if (ABaseCharacter* C = Cast<ABaseCharacter>(Actor))
        {
            // (2) 대미지 적용
            UGameplayStatics::ApplyDamage(C, DeathNovaDamage, GetController(), this, nullptr);
            UE_LOG(LogTemp, Warning, TEXT("[Lich] DeathNova → %s 입혔습니다"), *C->GetName());

            // (3) 스턴:  이동 비활성화
            if (UCharacterMovementComponent* MoveComp = C->GetCharacterMovement())
            {
                MoveComp->DisableMovement();

                // (4) 일정 시간 후 이동 복구
                FTimerHandle StunHandle;
                FTimerDelegate RestoreDel = FTimerDelegate::CreateLambda([MoveComp]() {
                    MoveComp->SetMovementMode(MOVE_Walking);
                    });
                GetWorldTimerManager().SetTimer(
                    StunHandle,
                    RestoreDel,
                    DeathNovaStunDuration,
                    false
                );
            }
        }
    }
}

bool ALCBossLich::RequestAttack(float TargetDistance)
{
    if (!HasAuthority()) return false;
    const float Now = GetWorld()->GetTimeSeconds();

    AActor* Target = nullptr;
    if (auto AC = Cast<AAIController>(GetController()))
        Target = Cast<AActor>(AC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));

    struct FEntry { float W; TFunction<void()> A; };
    TArray<FEntry> Entries;

    // Arcane Bolt
    {
        float CD = bIsBerserk
            ? ArcaneBoltCooldown * BerserkCooldownFactor
            : ArcaneBoltCooldown;
        if (Target && TargetDistance <= ArcaneBoltRange && Now - LastArcaneBoltTime >= CD)
        {
            Entries.Add({
                3.f,
                [this, Now, Target]() {
                    UE_LOG(LogTemp, Warning, TEXT("[Lich] 선택된 공격 → ArcaneBolt"));
                    LastArcaneBoltTime = Now;
                    ExecuteArcaneBolt(Target);
                }
                });
        }
    }

    // Soul Bind
    {
        float CD = bIsBerserk
            ? SoulBindCooldown * BerserkCooldownFactor
            : SoulBindCooldown;
        if (Target && TargetDistance <= SoulBindRange && Now - LastSoulBindTime >= CD)
        {
            Entries.Add({
                2.f,
                [this, Now, Target]() {
                    UE_LOG(LogTemp, Warning, TEXT("[Lich] 선택된 공격 → SoulBind"));
                    LastSoulBindTime = Now;
                    ExecuteSoulBind(Target);
                }
                });
        }
    }

    // Soul Absorb (fallback)
    {
        Entries.Add({
            1.f,
            [this, Target]() {
                UE_LOG(LogTemp, Warning, TEXT("[Lich] 선택된 공격 → SoulAbsorb"));
                ExecuteSoulAbsorb(Target);
            }
            });
    }

    // 가중치 합산 및 랜덤 선택
    float Total = 0.f;
    for (auto& e : Entries) Total += e.W;
    float Pick = FMath::FRandRange(0.f, Total), Acc = 0.f;
    for (auto& e : Entries)
    {
        Acc += e.W;
        if (Pick <= Acc)
        {
            e.A();
            return true;
        }
    }
    return false;
}