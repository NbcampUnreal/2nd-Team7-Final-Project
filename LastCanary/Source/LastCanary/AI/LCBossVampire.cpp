#include "AI/LCBossVampire.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/BaseCharacter.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "AI/LCBaseBossAIController.h"
#include "DrawDebugHelpers.h"


ALCBossVampire::ALCBossVampire()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ALCBossVampire::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateRage(DeltaSeconds);

    if (auto* AICon = Cast<ALCBaseBossAIController>(GetController()))
    {
        if (auto* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsFloat(TEXT("RagePercent"), Rage / MaxRage);
            BB->SetValueAsBool(TEXT("IsBerserkMode"), bIsBerserk);
        }
    }
}

void ALCBossVampire::BeginPlay()
{
    Super::BeginPlay();

    /*GetWorldTimerManager().SetTimer(BatSwarmTimerHandle, this, &ALCBossVampire::ExecuteBatSwarm, BatSwarmInterval, true);*/
    GetWorldTimerManager().SetTimer(GazeTimerHandle, this, &ALCBossVampire::ExecuteNightmareGaze, GazeInterval, true);
    GetWorldTimerManager().SetTimer(CrimsonChainsTimerHandle, this, &ALCBossVampire::ExecuteCrimsonChains, CrimsonChainsCooldown, true);
    GetWorldTimerManager().SetTimer(RainTimerHandle, this, &ALCBossVampire::ExecuteSanguineRain, SanguineBurstCooldown /*or 원하는 간격*/, true);

    GetWorldTimerManager().SetTimer(
        MistFormTimerHandle,
        [this]()
        {
            if (bCanUseMist)
            {
                EnterMistForm();
            }
        },
        MistFormInterval,
        true
    );
}

void ALCBossVampire::UpdateRage(float DeltaSeconds)
{
    // (1) 필요하다면 기본 누적 로직
    // 예: 전투 중이라면 자연회복 등
    // AddRage(PassiveRagePerSecond * DeltaSeconds);

    // (2) MistForm 중이라면 MistFormRageTimer를 쓰셨으니 굳이 여기선 안 해도 됩니다.
    // 다만 Tick 내부에서 원하는 추가 로직이 있으면 여기에 넣습니다.
}

void ALCBossVampire::AddRage(float Amount)
{
    if (!HasAuthority()) return;

    // (1) Rage 값 갱신 및 클램프
    Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);

    // (2) 최대치 도달 시 한 번만 Berserk 진입
    if (Rage >= MaxRage && !bIsBerserk)
    {
        EnterBerserkState();
    }
}

void ALCBossVampire::ExecuteBatSwarm()
{
    if (!HasAuthority() || !BatSwarmClass) return;
    for (int32 i = 0; i < BatCount; ++i)
    {
        FVector Dir = FMath::VRand();
        Dir.Z = 0.5f;
        FVector Loc = GetActorLocation() + Dir * 300.f + FVector(0, 0, 100);
        GetWorld()->SpawnActor<AActor>(BatSwarmClass, Loc, Dir.Rotation());
    }
}

void ALCBossVampire::ExecuteNightmareGaze()
{
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] NightmareGaze"));

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(GazeRadius);

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        GazeRadius,
        16,
        FColor::Purple,
        false,
        2.0f,
        0,
        5.0f
    );


    if (GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& H : Hits)
        {
            if (ACharacter* Ch = Cast<ABaseCharacter>(H.GetActor()))
            {
                if (Ch->IsPlayerControlled())
                {
                    float Orig = Ch->GetCharacterMovement()->MaxWalkSpeed;
                    Ch->GetCharacterMovement()->MaxWalkSpeed = Orig * 0.5f;
                    //  데미지 없음, 디버프만
                    FTimerHandle Tmp;
                    FTimerDelegate D = FTimerDelegate::CreateLambda([Ch, Orig]() { Ch->GetCharacterMovement()->MaxWalkSpeed = Orig; });
                    GetWorldTimerManager().SetTimer(Tmp, D, GazeDebuffDuration, false);
                }
            }
        }
    }
}

void ALCBossVampire::ExecuteCrimsonChains()
{
    if (!HasAuthority() || !CrimsonChainsEffectFX) return;

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        CrimsonChainsRadius,   // 원형 반경
        32,           // 세그먼트 수
        FColor::Red,
        false,        // PersistentLines
        CrimsonChainsDuration, // 지속 시간
        0,            // DepthPriority
        3.0f          // 두께
    );

    // (1) FX
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        CrimsonChainsEffectFX,
        GetActorLocation(),
        FRotator::ZeroRotator
    );

    // (2) 범위 내 Pawn 수집
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(CrimsonChainsRadius);
    GetWorld()->SweepMultiByChannel(
        Hits,
        GetActorLocation(), GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );

    // (3) 끌어당기기
    for (auto& H : Hits)
    {
        if (APawn* P = Cast<APawn>(H.GetActor()))
        {
            FVector Dir = (GetActorLocation() - P->GetActorLocation()).GetSafeNormal();
            if (UCharacterMovementComponent* Move = P->FindComponentByClass<UCharacterMovementComponent>())
            {
                Move->AddImpulse(Dir * CrimsonChainsPullStrength, true);
            }
        }
    }

    // (4) 지속시간 후 연쇄 폭발 (간단히 두 번째 FX만)
    FTimerHandle Tmp;
    GetWorldTimerManager().SetTimer(Tmp, [this]()
        {
            // 연쇄 폭발 FX, 데미지 및 출혈 디버프 로직 추가 가능
            // 예시: SanguineBurst를 호출해도 됩니다.
            ExecuteSanguineBurst();
        }, CrimsonChainsDuration, false);
}

void ALCBossVampire::ExecuteCrimsonSlash()
{
    GetWorldTimerManager().SetTimer(CrimsonSlashHandle, CrimsonSlashCooldown, false);

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] CrimsonSlash"));

    TArray<FHitResult> Hits;
    FCollisionShape S = FCollisionShape::MakeSphere(CrimsonSlashRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, S))
    {
        float Heal = CrimsonSlashDamage * BloodDrainEfficiency;
        for (auto& H : Hits)
        {
            if (ACharacter* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                UGameplayStatics::ApplyDamage(C, CrimsonSlashDamage, GetController(), this, nullptr);
            }
        }
        // 피흡
        UGameplayStatics::ApplyDamage(this, -Heal, GetController(), this, nullptr);
    }
}



void ALCBossVampire::ExecuteSanguineRain()
{
    if (!HasAuthority() || !SanguineRainEffectFX) return;

    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        RainRadius,   // 원형 반경
        32,           // 세그먼트 수
        FColor::Red,
        false,        // PersistentLines
        RainDuration, // 지속 시간
        0,            // DepthPriority
        3.0f          // 두께
    );
    UE_LOG(LogTemp, Warning,
        TEXT("[Vampire] ExecuteSanguineRain 실행: Radius=%.1f, Duration=%.1f"),
        RainRadius, RainDuration);

    // (1) 비 FX
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        SanguineRainEffectFX,
        GetActorLocation(),
        FRotator::ZeroRotator
    );

    // (2) 주기적 데미지·감속 시작
    GetWorldTimerManager().SetTimer(RainTickHandle, this, &ALCBossVampire::TickRainDamage, 1.f, true);

    // (3) 지속시간 후 종료
    GetWorldTimerManager().SetTimer(RainTimerHandle, this, &ALCBossVampire::EndSanguineRain, RainDuration, false);
}

void ALCBossVampire::TickRainDamage()
{
    // 영역 내 플레이어에게 출혈·감속
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(RainRadius);
    GetWorld()->SweepMultiByChannel(
        Hits,
        GetActorLocation(), GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );

    for (auto& H : Hits)
    {
        if (ABaseCharacter* C = Cast<ABaseCharacter>(H.GetActor()))
        {
            if (C->IsPlayerControlled())
            {
                // 출혈 데미지
                UGameplayStatics::ApplyDamage(C, RainDotDamage, GetController(), this, nullptr);

                // 속도 감소
                auto* Move = C->GetCharacterMovement();
                float Orig = Move->MaxWalkSpeed;
                Move->MaxWalkSpeed = Orig * (1.f - RainSlowPercent);

                // 일정시간 후 속도 복구
                FTimerHandle Tmp;
                GetWorldTimerManager().SetTimer(Tmp, [Move, Orig]() {
                    Move->MaxWalkSpeed = Orig;
                    }, 1.f, false);
            }
        }
    }
}

void ALCBossVampire::EndSanguineRain()
{
    // (1) 주기 타이머 해제
    GetWorldTimerManager().ClearTimer(RainTickHandle);

    // (2) 피 웅덩이 소환
    if (BloodPuddleClass)
    {
        GetWorld()->SpawnActor<AActor>(
            BloodPuddleClass,
            GetActorLocation(),
            FRotator::ZeroRotator
        );
    }
}

void ALCBossVampire::ExecuteSanguineBurst()
{
    GetWorldTimerManager().SetTimer(BurstHandle, SanguineBurstCooldown, false);

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] SanguineBurst"));

    TArray<FHitResult> Hits;
    FCollisionShape S = FCollisionShape::MakeSphere(SanguineBurstRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, S))
    {
        for (auto& H : Hits)
        {
            if (ACharacter* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                UGameplayStatics::ApplyDamage(C, SanguineBurstDamage, GetController(), this, nullptr);
            }
        }

    }
}

void ALCBossVampire::EnterMistForm()
{
    if (!HasAuthority() || !bCanUseMist) return;

    // ── 디버그: MistForm 범위 시각화 및 로그 ──
    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        MistRadius,   // 원형 반경
        32,           // 세그먼트 수
        FColor::Purple,
        false,        // PersistentLines
        MistDuration, // 지속 시간
        0,            // DepthPriority
        3.0f          // 두께
    );
    UE_LOG(LogTemp, Warning,
        TEXT("[Vampire] EnterMistForm 실행: Radius=%.1f, Duration=%.1f"),
        MistRadius, MistDuration);

    bCanUseMist = false;
    bIsMistForm = true;

    // 이펙트·사운드 재생
    Multicast_StartMistForm();
    SetCanBeDamaged(false);

    // 1) 일정 주기로 Rage 회복 및 플레이어 체크
    GetWorldTimerManager().SetTimer(
        MistFormRageTimerHandle,
        this, &ALCBossVampire::TickMistFormRage,
        1.0f, true
    );

    // 2) MistDuration 후 종료 예약
    GetWorldTimerManager().SetTimer(
        MistDurationHandle,
        this, &ALCBossVampire::EndMistForm,
        MistDuration, false
    );
}

void ALCBossVampire::TickMistFormRage()
{
    // (1) 기본 Rage 회복
    AddRage(MistRagePerSecond);

    // (2) 범위 내 플레이어 찾기
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(MistRadius);
    GetWorld()->SweepMultiByChannel(
        Hits,
        GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, Sphere
    );

    for (auto& H : Hits)
    {
        if (APawn* P = Cast<APawn>(H.GetActor()))
        {
            if (P->IsPlayerControlled())
            {
                // a) 추가 Rage
                AddRage(MistPlayerBonusRagePerSecond);

                // b) 플레이어 카메라에 포스트프로세스 적용
                if (UCameraComponent* Cam = P->FindComponentByClass<UCameraComponent>())
                {
                    Cam->PostProcessSettings.AddBlendable(
                        MistPostProcessMaterial,
                        MistPostProcessWeight
                    );
                }
            }
        }
    }
}

void ALCBossVampire::EndMistForm()
{
    // (1) MistForm 상태 해제
    bIsMistForm = false;
    OnRep_MistForm();   // 해제 이펙트·사운드

    // (2) Rage 회복 타이머 해제
    GetWorldTimerManager().ClearTimer(MistFormRageTimerHandle);

    // (3) 플레이어들의 포스트프로세스 제거
    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), Players);
    for (AActor* A : Players)
    {
        if (APawn* P = Cast<APawn>(A))
        {
            if (P->IsPlayerControlled())
            {
                if (UCameraComponent* Cam = P->FindComponentByClass<UCameraComponent>())
                {
                    Cam->PostProcessSettings.RemoveBlendable(MistPostProcessMaterial);
                }
            }
        }
    }

    // (4) 무형 해제 이펙트·사운드 재생
    if (MistExitEffectFX)
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), MistExitEffectFX,
            GetActorLocation(), GetActorRotation()
        );
    if (MistExitSound)
        UGameplayStatics::PlaySoundAtLocation(
            this, MistExitSound, GetActorLocation()
        );

    // (5) Mist 재사용 쿨다운 시작
    GetWorldTimerManager().SetTimer(
        MistResetHandle,
        [this]() { bCanUseMist = true; },
        MistCooldown, false
    );
}

void ALCBossVampire::OnRep_MistForm()
{
    if (bIsMistForm)
    {
        // 1) 무형 진입 이펙트 재생 (Niagara)
        if (MistEnterEffectFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                MistEnterEffectFX,
                GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }

        // 2) 무형 진입 사운드 재생
        if (MistEnterSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                MistEnterSound,
                GetActorLocation()
            );
        }
    }
    else
    {
        // 1) 무형 해제 이펙트 재생
        if (MistExitEffectFX)
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                MistExitEffectFX,
                GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }

        // 2) 무형 해제 사운드 재생
        if (MistExitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                MistExitSound,
                GetActorLocation()
            );
        }
    }
}

void ALCBossVampire::Multicast_StartMistForm_Implementation()
{
    OnRep_MistForm();
}

void ALCBossVampire::OnRep_Bloodlust()
{
    if (bIsBloodlust)
    {
        // Mist Form 즉시 사용 가능
        bCanUseMist = true;
        BloodDrainEfficiency = 2.0f;

        // 캐시
        OriginalMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;
        OriginalCrimsonCooldown = CrimsonSlashCooldown;
        OriginalBurstCooldown = SanguineBurstCooldown;

        // 속도 증가
        GetCharacterMovement()->MaxWalkSpeed *= 1.3f;
        // 쿨다운 감소
        CrimsonSlashCooldown *= 0.7f;
        SanguineBurstCooldown *= 0.7f;

        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Eternal Bloodlust Activated"));
    }
    else
    {
        // 복구
        BloodDrainEfficiency = 1.0f;
        GetCharacterMovement()->MaxWalkSpeed = OriginalMoveSpeed;
        CrimsonSlashCooldown = OriginalCrimsonCooldown;
        SanguineBurstCooldown = OriginalBurstCooldown;

        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Eternal Bloodlust Ended"));
    }
}

void ALCBossVampire::Multicast_StartBloodlust_Implementation()
{
    bIsBloodlust = true;
    OnRep_Bloodlust();

    // 지속시간 후 bIsBloodlust=false → OnRep_Bloodlust() 호출
    GetWorldTimerManager().ClearTimer(BloodlustDurationHandle);
    GetWorldTimerManager().SetTimer(BloodlustDurationHandle, [this]()
        {
            bIsBloodlust = false;
            OnRep_Bloodlust();
        }, BloodlustDuration, false);
}

// --- RepNotify 에서 클라이언트에서도 연출/해제 ---
void ALCBossVampire::OnRep_IsBerserk()
{
    if (bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Berserk 시작 클라 연출"));

        // 1) 붉은 안개 FX 스폰 & 저장
        if (BloodlustEffectFX)
        {
            BerserkEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
                BloodlustEffectFX,
                GetRootComponent(),
                NAME_None,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }

        // 2) 심장 박동음 AudioComponent로 스폰 & 저장
        if (BloodlustSound)
        {
            BerserkAudioComp = UGameplayStatics::SpawnSoundAttached(
                BloodlustSound,
                GetRootComponent()
            );
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Berserk 종료 클라 연출 해제"));

        // 1) FX 컴포넌트 정리
        if (BerserkEffectComp)
        {
            BerserkEffectComp->DestroyComponent();
            BerserkEffectComp = nullptr;
        }

        // 2) Audio 컴포넌트 정리
        if (BerserkAudioComp)
        {
            BerserkAudioComp->Stop();
            BerserkAudioComp->DestroyComponent();
            BerserkAudioComp = nullptr;
        }
    }
}

// --- 서버에서 Rage 가 Max 에 도달했을 때 최초 진입 (입장직후, Enter→Start 자동 실행) ---
void ALCBossVampire::EnterBerserkState()
{
    Super::EnterBerserkState();
    UE_LOG(LogTemp, Warning, TEXT("[Vampire] Enter Eternal Bloodlust"));

    // 이때 특수 상태 플래그 켜기 → 기존 Multicast_StartBloodlust 에서
    Multicast_StartBloodlust();
}

// --- bIsBerserk=true 직후(클라이언트/서버 공통) 이펙트 & 사운드 ---
void ALCBossVampire::StartBerserk()
{
    Super::StartBerserk();

	MistRadius = BerserkMistRadius; // 광폭화 시 MistForm 거리 증가

    // (1) 붉은 안개 FX
    if (BloodlustEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            BloodlustEffectFX,
            GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }

    // (2) 심장 박동음 재생
    if (BloodlustSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            BloodlustSound,
            GetActorLocation()
        );
    }
}

void ALCBossVampire::StartBerserk(float Duration)
{
    // 기본 StartBerserk() 의 연출 재생
    StartBerserk();

    // Duration 초 뒤 EndBerserk() 호출 예약
    GetWorldTimerManager().ClearTimer(BerserkTimerHandle);
    GetWorldTimerManager().SetTimer(
        BerserkTimerHandle,
        this, &ALCBossVampire::EndBerserk,
        Duration, false
    );
}

// --- bIsBerserk=false 로 전환(클라이언트/서버) 시 실행 ---
void ALCBossVampire::EndBerserk()
{
    Super::EndBerserk();
    UE_LOG(LogTemp, Warning, TEXT("[Vampire] Exit Eternal Bloodlust"));

    // (선택) 추가 연출이 필요하면 여기에
    // 예: 붉은 안개 FX 제거, 사운드 정지 등
}

bool ALCBossVampire::RequestAttack(float TargetDistance)
{
    if (!HasAuthority()) return false;

    struct FEntry { float W; TFunction<void()> A; };
    TArray<FEntry> Entries;

    // Crimson Slash (근접)
    if (TargetDistance <= CrimsonSlashRadius && !GetWorldTimerManager().IsTimerActive(CrimsonSlashHandle))
        Entries.Add({ 3.f, [this]() { ExecuteCrimsonSlash(); } });

    // Sanguine Burst (중거리)
    if (TargetDistance <= SanguineBurstRadius && !GetWorldTimerManager().IsTimerActive(BurstHandle))
        Entries.Add({ 2.f, [this]() { ExecuteSanguineBurst(); } });


    float Total = 0; for (auto& e : Entries) Total += e.W;
    float Pick = FMath::FRandRange(0.f, Total), Acc = 0;
    for (auto& e : Entries)
    {
        Acc += e.W;
        if (Pick <= Acc) { e.A(); return true; }
    }
    return false;
}


void ALCBossVampire::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossVampire, bIsMistForm);
    DOREPLIFETIME(ALCBossVampire, bIsBloodlust);
}