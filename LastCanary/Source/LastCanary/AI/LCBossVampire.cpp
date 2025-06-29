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
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"


ALCBossVampire::ALCBossVampire()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // MistSphere 생성·설정 (기본으론 비활성)
    MistSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MistSphere"));
    MistSphere->SetupAttachment(RootComponent);
    MistSphere->SetSphereRadius(MistRadius);
    MistSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MistSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    MistSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    MistSphere->OnComponentBeginOverlap.AddDynamic(this, &ALCBossVampire::OnMistOverlapBegin);
    MistSphere->OnComponentEndOverlap.AddDynamic(this, &ALCBossVampire::OnMistOverlapEnd);
}

void ALCBossVampire::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 1) DeltaSeconds 유효성 검사
    if (DeltaSeconds <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Vampire::Tick] Invalid DeltaSeconds: %f"), DeltaSeconds);
        return;
    }

    // 2) Rage 갱신
    UpdateRage(DeltaSeconds);
}

void ALCBossVampire::UpdateBlackboardValues() 
{
    Super::UpdateBlackboardValues();
}

void ALCBossVampire::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(BatSwarmTimerHandle, this, &ALCBossVampire::ExecuteBatSwarm, BatSwarmInterval, true);
    GetWorldTimerManager().SetTimer(GazeTimerHandle, this, &ALCBossVampire::ExecuteNightmareGaze, GazeInterval, true);
    GetWorldTimerManager().SetTimer(CrimsonChainsTimerHandle, this, &ALCBossVampire::ExecuteCrimsonChains, CrimsonChainsCooldown, true);
    GetWorldTimerManager().SetTimer(RainTimerHandle, this, &ALCBossVampire::ExecuteSanguineRain, SanguineBurstCooldown, true);

    GetWorldTimerManager().SetTimer(
        MistFormTimerHandle,
        this,
        &ALCBossVampire::EnterMistForm,
        MistFormInterval,
        true
    );
}

void ALCBossVampire::UpdateRage(float DeltaSeconds)
{
    // (0) 서버 전용 & DeltaSeconds 유효성 검사
    if (!HasAuthority() || DeltaSeconds <= 0.f)
        return;

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[Vampire::UpdateRage] World is null!"));
        return;
    }

    // (1) 부모 로직 호출 (예: DeathNova 등)
    Super::UpdateRage(DeltaSeconds);

    // ── MistSphere 활성 중일 때만 기존 회복/보너스 적용 ──
    if (MistSphere->GetCollisionEnabled() == ECollisionEnabled::QueryOnly)
    {
        // 1. 범위 내 플레이어 보너스
        TArray<FHitResult> Hits;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(MistRadius);
        if (World->SweepMultiByChannel(
            Hits,
            GetActorLocation(), GetActorLocation(),
            FQuat::Identity,
            ECC_Pawn,
            Sphere))
        {
            for (auto& H : Hits)
            {
                if (APawn* P = Cast<APawn>(H.GetActor()))
                {
                    if (P->IsPlayerControlled())
                    {
                        AddRage(MistPlayerBonusRagePerSecond * DeltaSeconds);
                    }
                }
            }
        }
    }

    // ── Bat Swarm에 따라 Rage 증가 ──
    // 1. 이미 죽은(Invalid) 박쥐 정리
    SpawnedBatSwarm.RemoveAll([](AActor* Bat) {
        return !IsValid(Bat);
        });
    // 2. 살아있는 박쥐 수 집계
    const int32 AliveBats = SpawnedBatSwarm.Num();
    if (AliveBats > 0 && BatRagePerSecond > 0.f)
    {
        AddRage(BatRagePerSecond * AliveBats * DeltaSeconds);
    }
}

void ALCBossVampire::AddRage(float Amount)
{
    // (0) 서버 전용 검사
    if (!HasAuthority())
        return;

    // (1) MaxRage 유효성 검사
    if (MaxRage <= 0.f)
    {
        UE_LOG(LogTemp, Error, TEXT("[Slenderman::AddRage] Invalid MaxRage: %f"), MaxRage);
        return;
    }

    // (2) Rage 갱신 및 클램프
    Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);

    // (3) 공통된 블랙보드 갱신
    UpdateBlackboardValues();

    // (4) MaxRage 도달 시 Berserk 진입 (한 번만)
    if (Rage >= MaxRage && !bIsBerserk)
    {
        if (BerserkDuration > 0.f)
        {
            StartBerserk(BerserkDuration);
            UpdateBlackboardValues();
        }
    }

    if (Rage >= SanguineRainRageThreshold)
    {
        ExecuteSanguineRain();
    }
}

void ALCBossVampire::ExecuteBatSwarm()
{
    // (0) 서버 전용 & 클래스 유효성 검사
    if (!HasAuthority() || !BatSwarmClass)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    // (1) 이미 죽은(Invalid) Bat 정리
    SpawnedBatSwarm.RemoveAll([](AActor* Bat) {
        return !IsValid(Bat);
        });

    // (2) 부족한 수 계산
    const int32 CurrentCount = SpawnedBatSwarm.Num();
    const int32 ToSpawn = FMath::Max(0, BatCount - CurrentCount);
    if (ToSpawn == 0)
        return;

    // (3) 부족한 만큼 소환
    for (int32 i = 0; i < ToSpawn; ++i)
    {
        FVector Dir = FMath::VRand();
        Dir.Z = 0.5f;
        FVector Loc = GetActorLocation() + Dir * 300.f + FVector(0, 0, 100);

        if (AActor* Bat = World->SpawnActor<AActor>(BatSwarmClass, Loc, Dir.Rotation()))
        {
            // (a) 리스트에 추가
            SpawnedBatSwarm.Add(Bat);
            // (b) 파괴 시 콜백 바인딩
            Bat->OnDestroyed.AddDynamic(this, &ALCBossVampire::OnBatDestroyed);
        }
    }
}

void ALCBossVampire::OnBatDestroyed(AActor* DestroyedActor)
{
    // (0) 서버 권한 검사
    if (!HasAuthority() || !DestroyedActor)
        return;

    // (1) 배열에서 제거
    SpawnedBatSwarm.Remove(DestroyedActor);

    // (2) Rage 페널티 적용
    //    BatDeathRagePenalty 만큼 Rage를 깎되, AddRage 내부에서 0..MaxRage로 클램프
    AddRage(-BatDeathRagePenalty);

    UE_LOG(LogTemp, Warning,
        TEXT("[Vampire] Bat killed → Rage -%.1f (now %.1f/%.1f)"),
        BatDeathRagePenalty, Rage, MaxRage);
}

void ALCBossVampire::ExecuteNightmareGaze()
{
    // (1) 서버 전용 검사 + World 널 체크
    if (!HasAuthority()) return;
    UWorld* World = GetWorld();
    if (!World) return;

    // (2) 모든 클라이언트에 FX/SFX 요청
    Multicast_PlayNightmareGazeEffects();

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] NightmareGaze"));

    // (3) 게임플레이 로직: 범위 내 플레이어 감속
    TArray<FHitResult> Hits;
    if (World->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(GazeRadius)))
    {
        for (auto& H : Hits)
        {
            if (ABaseCharacter* Ch = Cast<ABaseCharacter>(H.GetActor()))
            {
                if (Ch->IsPlayerControlled() && Ch->GetCharacterMovement())
                {
                    float Orig = Ch->GetCharacterMovement()->MaxWalkSpeed;
                    Ch->GetCharacterMovement()->MaxWalkSpeed = Orig * 0.5f;

                    // (4) 일정 시간 후 원상복구
                    FTimerHandle Tmp;
                    FTimerDelegate Del = FTimerDelegate::CreateWeakLambda(Ch, [=]() {
                        if (Ch->GetCharacterMovement())
                            Ch->GetCharacterMovement()->MaxWalkSpeed = Orig;
                        });
                    World->GetTimerManager().SetTimer(Tmp, Del, GazeDebuffDuration, false);
                }
            }
        }
    }
}

void ALCBossVampire::Multicast_PlayNightmareGazeEffects_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // SFX (감쇠 포함)
    if (GazeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, GazeSound, GetActorLocation());
    }
}

void ALCBossVampire::ExecuteCrimsonChains()
{
    if (!HasAuthority()) return;
    UWorld* World = GetWorld();
    if (!World) return;

    Multicast_PlayCrimsonChainsEffects();

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] CrimsonChains"));

    // 끌어당기기 로직
    TArray<FHitResult> Hits;
    if (World->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(),
        FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(CrimsonChainsRadius)))
    {
        for (auto& H : Hits)
        {
            if (auto* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
                {
                    FVector Dir = (GetActorLocation() - C->GetActorLocation()).GetSafeNormal();
                    Move->AddImpulse(Dir * CrimsonChainsPullStrength, true);
                }
            }
        }
    }

    // 폭발 콜백
    FTimerHandle Tmp;
    World->GetTimerManager().SetTimer(Tmp, this, &ALCBossVampire::ExecuteSanguineBurst, CrimsonChainsDuration, false);
}

void ALCBossVampire::Multicast_PlayCrimsonChainsEffects_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (CrimsonChainsEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World, CrimsonChainsEffectFX,
            GetActorLocation(), FRotator::ZeroRotator,
            FVector(1.f), true);
    }
    if (CrimsonChainsSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, CrimsonChainsSound, GetActorLocation(),
            1.f, 1.f, 0.f,
            AttackAttenuation);
    }
}

void ALCBossVampire::ExecuteCrimsonSlash()
{
    // (1) 서버 전용 & World 널체크
    if (!HasAuthority()) return;
    UWorld* World = GetWorld();
    if (!World) return;

    // (2) 쿨다운 타이머 설정
    World->GetTimerManager().SetTimer(
        CrimsonSlashHandle,
        CrimsonSlashCooldown,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] CrimsonSlash"));

    // (3) FX/SFX 멀티캐스트
    Multicast_PlayCrimsonSlashEffects();

    // (4) 범위 내 플레이어 데미지 적용
    TArray<FHitResult> Hits;
    if (World->SweepMultiByChannel(
        Hits,
        GetActorLocation(), GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(CrimsonSlashRadius)))
    {
        for (auto& H : Hits)
        {
            if (auto* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                UGameplayStatics::ApplyDamage(
                    C,
                    CrimsonSlashDamage,
                    GetController(),
                    this,
                    nullptr
                );
            }
        }
    }
}

void ALCBossVampire::Multicast_PlayCrimsonSlashEffects_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // (A) VFX
    if (CrimsonSlashEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            CrimsonSlashEffectFX,
            GetActorLocation(),
            FRotator::ZeroRotator,
            FVector(1.f),
            true
        );
    }

    // (B) SFX (감쇠 포함)
    if (CrimsonSlashSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            CrimsonSlashSound,
            GetActorLocation(),
            1.f,    // Volume
            1.f,    // Pitch
            0.f,    // StartTime
            AttackAttenuation
        );
    }
}

void ALCBossVampire::ExecuteSanguineRain()
{
    // (1) 서버 전용 및 필수 에셋 체크
    if (!HasAuthority() || !SanguineRainEffectFX)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    UE_LOG(LogTemp, Warning,
        TEXT("[Vampire] ExecuteSanguineRain 실행: Radius=%.1f, Duration=%.1f"),
        RainRadius, RainDuration);

    // (2) 멀티캐스트로 모든 클라이언트에 FX/SFX 요청
    Multicast_PlaySanguineRainEffects();

    // (3) 1초마다 TickRainDamage() 호출
    GetWorldTimerManager().SetTimer(
        RainTickHandle,
        this, &ALCBossVampire::TickRainDamage,
        1.f,
        true
    );

    // (4) 지정된 지속시간 후 효과 종료
    GetWorldTimerManager().SetTimer(
        RainTimerHandle,
        this, &ALCBossVampire::EndSanguineRain,
        RainDuration,
        false
    );
}

void ALCBossVampire::Multicast_PlaySanguineRainEffects_Implementation()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    const FVector Loc = GetActorLocation();

    // VFX
    if (SanguineRainEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            SanguineRainEffectFX,
            Loc,
            FRotator::ZeroRotator,
            FVector(1.f),
            true
        );
    }

    // SFX with attenuation
    if (SanguineRainSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            SanguineRainSound,
            Loc,
            1.f,               // Volume
            1.f,               // Pitch
            0.f,               // StartTime
            AttackAttenuation  // Attenuation Asset
        );
    }
}

void ALCBossVampire::TickRainDamage()
{
    // (1) 서버 전용 및 널 체크
    if (!HasAuthority())
        return;
    UWorld* World = GetWorld();
    if (!World)
        return;

    // (2) 범위 내 플레이어에 출혈 데미지·감속
    TArray<FHitResult> Hits;
    if (World->SweepMultiByChannel(
        Hits,
        GetActorLocation(), GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(RainRadius)))
    {
        for (auto& H : Hits)
        {
            if (auto* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                if (C->IsPlayerControlled())
                {
                    // 출혈 데미지
                    UGameplayStatics::ApplyDamage(
                        C, RainDotDamage,
                        GetController(),
                        this,
                        nullptr
                    );

                    // 속도 감소
                    if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
                    {
                        float Orig = Move->MaxWalkSpeed;
                        Move->MaxWalkSpeed = Orig * (1.f - RainSlowPercent);

                        // 1초 뒤 복구
                        FTimerHandle Tmp;
                        GetWorldTimerManager().SetTimer(
                            Tmp,
                            FTimerDelegate::CreateLambda([Move, Orig]() {
                                Move->MaxWalkSpeed = Orig;
                                }),
                            1.f,
                            false
                        );
                    }
                }
            }
        }
    }
}

void ALCBossVampire::EndSanguineRain()
{
    // (1) 서버 전용 및 널 체크
    if (!HasAuthority())
        return;
    UWorld* World = GetWorld();
    if (!World)
        return;

    // (2) 주기 타이머 해제
    GetWorldTimerManager().ClearTimer(RainTickHandle);

    // (3) 피 웅덩이 소환
    if (BloodPuddleClass)
    {
        World->SpawnActor<AActor>(
            BloodPuddleClass,
            GetActorLocation(),
            FRotator::ZeroRotator
        );
    }
}

void ALCBossVampire::ExecuteSanguineBurst()
{
    // (1) 서버 전용 검사
    if (!HasAuthority())
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    // (2) 쿨다운 타이머 재설정
    World->GetTimerManager().SetTimer(
        BurstHandle,
        this, &ALCBossVampire::ExecuteSanguineBurst,
        SanguineBurstCooldown,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("[Vampire] ExecuteSanguineBurst"));

    // (3) 모든 클라이언트에 FX/SFX 재생 요청
    Multicast_PlaySanguineBurstEffects();

    // (4) 범위 내 플레이어에게 데미지 적용
    TArray<FHitResult> Hits;
    if (World->SweepMultiByChannel(
        Hits,
        GetActorLocation(), GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(SanguineBurstRadius)))
    {
        AController* InstigatorCtrl = GetController();
        for (const auto& H : Hits)
        {
            if (auto* C = Cast<ABaseCharacter>(H.GetActor()))
            {
                UGameplayStatics::ApplyDamage(
                    C,
                    SanguineBurstDamage,
                    InstigatorCtrl,
                    this,
                    nullptr
                );
            }
        }
    }
}

void ALCBossVampire::Multicast_PlaySanguineBurstEffects_Implementation()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    const FVector Loc = GetActorLocation();


    // SFX with attenuation
    if (SanguineBurstSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            SanguineBurstSound,
            Loc,
            1.f,               // VolumeMultiplier
            1.f,               // PitchMultiplier
            0.f,               // StartTime
            AttackAttenuation  // Attenuation Asset
        );
    }
}

void ALCBossVampire::EnterMistForm()
{
    if (!HasAuthority()) return;
    UWorld* World = GetWorld();
    if (!World) return;

    DrawDebugSphere(World, GetActorLocation(), MistRadius, 32, FColor::Purple, false, MistDuration, 0, 3.f);
    UE_LOG(LogTemp, Warning, TEXT("[Vampire] EnterMistForm: Radius=%.1f, Duration=%.1f"), MistRadius, MistDuration);

    MistSphere->SetSphereRadius(MistRadius);
    MistSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    World->GetTimerManager().SetTimer(
        MistDurationHandle,
        this, &ALCBossVampire::EndMistForm,
        MistDuration,
        false
    );
}

void ALCBossVampire::OnMistOverlapBegin(
    UPrimitiveComponent*,
    AActor* OtherActor,
    UPrimitiveComponent*,
    int32,
    bool,
    const FHitResult& )
{
    if (!HasAuthority() || MistSphere->GetCollisionEnabled() != ECollisionEnabled::QueryOnly)
        return;

    if (APawn* P = Cast<APawn>(OtherActor))
    {
        if (P->IsPlayerControlled())
        {
            // 모든 클라이언트에서 해당 플레이어에 사운드 재생
            Multicast_PlayMistEnterEffects(P);
        }
    }
}



void ALCBossVampire::OnMistOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority() || MistSphere->GetCollisionEnabled() != ECollisionEnabled::QueryOnly)
        return;

    // Mist 끝날 때 이펙트만 재생 (사운드는 Enter 시 한 번만)
    Multicast_PlayMistExitEffects(GetActorLocation(), GetActorRotation());
}

void ALCBossVampire::EndMistForm()
{
    if (!HasAuthority()) return;

    MistSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 모든 클라이언트에 Exit FX 재생
    Multicast_PlayMistExitEffects(GetActorLocation(), GetActorRotation());
}

void ALCBossVampire::Multicast_PlayMistEnterEffects_Implementation(APawn* Pawn)
{
    if (!IsValid(Pawn) || !MistEnterSound) return;

    // 로컬 플레이어든 서버든, 각 클라이언트 컨텍스트에서 재생
    UGameplayStatics::SpawnSoundAttached(
        MistEnterSound,
        Pawn->GetRootComponent(),
        NAME_None,
        FVector::ZeroVector,
        EAttachLocation::KeepRelativeOffset,
        true,                 // bStopWhenAttachedToDestroyed
        1.f,                  // VolumeMultiplier
        1.f,                  // PitchMultiplier
        0.f,                  // StartTime
        MistSoundAttenuation  // AttenuationSettings
    );
}

void ALCBossVampire::Multicast_PlayMistExitEffects_Implementation(const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (MistExitEffectFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            MistExitEffectFX,
            Location,
            Rotation
        );
    }
}

void ALCBossVampire::OnRep_Bloodlust()
{
    // (0) MovementComponent 유효성 검사
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[Vampire] OnRep_Bloodlust: CharacterMovementComponent is null"));
        return;
    }

    if (bIsBloodlust)
    {
        // 활성화 시
        BloodDrainEfficiency = 2.0f;

        // 캐시
        OriginalMoveSpeed = MoveComp->MaxWalkSpeed;
        OriginalCrimsonCooldown = CrimsonSlashCooldown;
        OriginalBurstCooldown = SanguineBurstCooldown;

        // 버프 적용
        MoveComp->MaxWalkSpeed *= 1.3f;
        CrimsonSlashCooldown *= 0.7f;
        SanguineBurstCooldown *= 0.7f;

        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Eternal Bloodlust Activated"));
    }
    else
    {
        // 종료 시 복구
        BloodDrainEfficiency = 1.0f;
        MoveComp->MaxWalkSpeed = OriginalMoveSpeed;
        CrimsonSlashCooldown = OriginalCrimsonCooldown;
        SanguineBurstCooldown = OriginalBurstCooldown;

        UE_LOG(LogTemp, Warning, TEXT("[Vampire] Eternal Bloodlust Ended"));
    }
}

void ALCBossVampire::Multicast_StartBloodlust_Implementation()
{
    // (0) World 유효성 검사
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[Vampire] Multicast_StartBloodlust: World is null"));
        return;
    }

    // (1) 플래그 세팅 및 즉시 효과 적용
    bIsBloodlust = true;
    OnRep_Bloodlust();

    // (2) 기존 타이머 해제
    World->GetTimerManager().ClearTimer(BloodlustDurationHandle);

    // (3) 일정 시간 후 bIsBloodlust=false 처리
    World->GetTimerManager().SetTimer(
        BloodlustDurationHandle,
        FTimerDelegate::CreateLambda([this]()
            {
                bIsBloodlust = false;
                OnRep_Bloodlust();
            }),
        BloodlustDuration,
        false
    );
}
// --- 서버에서 Rage 가 Max 에 도달했을 때 최초 진입 (입장직후, Enter→Start 자동 실행) ---
void ALCBossVampire::EnterBerserkState()
{
    Super::EnterBerserkState();
    UE_LOG(LogTemp, Warning, TEXT("[Vampire] Enter Eternal Bloodlust"));

}

// --- bIsBerserk=true 직후(클라이언트/서버 공통) 이펙트 & 사운드 ---
void ALCBossVampire::StartBerserk()
{
    Super::StartBerserk();

	MistRadius = BerserkMistRadius; // 광폭화 시 MistForm 거리 증가
    Multicast_StartBloodlust();

}

void ALCBossVampire::StartBerserk(float Duration)
{
    Super::StartBerserk(BerserkDuration);
    Multicast_StartBloodlust();
}

// --- bIsBerserk=false 로 전환(클라이언트/서버) 시 실행 ---
void ALCBossVampire::EndBerserk()
{
    Super::EndBerserk();
    UE_LOG(LogTemp, Warning, TEXT("[Vampire] Exit Eternal Bloodlust"));

    // 블러드러스트 상태 해제
    bIsBloodlust = false;
    OnRep_Bloodlust();

    // 기존 Bloodlust 타이머 취소
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BloodlustDurationHandle);
    }

}

bool ALCBossVampire::RequestAttack(float TargetDistance)
{
    // 1) 서버 전용 & 월드 유효성 검사
    if (!HasAuthority())
        return false;
    UWorld* World = GetWorld();
    if (!World)
        return false;
    FTimerManager& TimerManager = World->GetTimerManager();

    // 2) 가능한 공격 목록 수집
    struct FAttackEntry { float Weight; TFunction<void()> Action; };
    TArray<FAttackEntry> Entries;

    // Crimson Slash (근접)
    if (TargetDistance <= CrimsonSlashRadius
        && !TimerManager.IsTimerActive(CrimsonSlashHandle))
    {
        Entries.Add({ 3.f, [this]() {
            ExecuteCrimsonSlash();
        } });
    }

    // Sanguine Burst (중거리)
    if (TargetDistance <= SanguineBurstRadius
        && !TimerManager.IsTimerActive(BurstHandle))
    {
        Entries.Add({ 2.f, [this]() {
            ExecuteSanguineBurst();
        } });
    }

    // 3) 선택지 없으면 false 반환
    if (Entries.Num() == 0)
        return false;

    // 4) 가중치 합산
    float TotalWeight = 0.f;
    for (const auto& E : Entries)
        TotalWeight += E.Weight;
    if (TotalWeight <= KINDA_SMALL_NUMBER)
        return false;

    // 5) 랜덤 선택
    float Pick = FMath::FRandRange(0.f, TotalWeight);
    float Accum = 0.f;
    for (const auto& E : Entries)
    {
        Accum += E.Weight;
        if (Pick <= Accum)
        {
            E.Action();
            return true;
        }
    }

    return false;
}


void ALCBossVampire::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossVampire, bIsBloodlust);
}