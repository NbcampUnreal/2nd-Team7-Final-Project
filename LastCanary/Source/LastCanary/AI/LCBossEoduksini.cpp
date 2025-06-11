#include "AI/LCBossEoduksini.h"
#include "AI/LCBaseBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraShakeBase.h"

ALCBossEoduksini::ALCBossEoduksini()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // Darkness sphere
    DarknessSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DarknessSphere"));
    DarknessSphere->SetupAttachment(GetRootComponent());
    DarknessSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DarknessSphere->SetSphereRadius(DarknessRadius);
    DarknessSphere->SetGenerateOverlapEvents(false);

    DarknessSphere->OnComponentBeginOverlap.AddDynamic(this, &ALCBossEoduksini::OnDarknessSphereBeginOverlap);
    DarknessSphere->OnComponentEndOverlap.AddDynamic(this, &ALCBossEoduksini::OnDarknessSphereEndOverlap);

}

void ALCBossEoduksini::BeginPlay()
{
    Super::BeginPlay();

    DarknessSphere->SetGenerateOverlapEvents(true);

}

void ALCBossEoduksini::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;

    UpdateRageAndScale(DeltaSeconds);


    if (bIsBerserk && Rage >= DarknessRageThreshold && !bDarknessActive)
    {
        TryTriggerDarkness();
    }

    // update blackboard
    if (auto* AICon = Cast<ALCBaseBossAIController>(GetController()))
    {
        if (auto* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsFloat(TEXT("RagePercent"), Rage / MaxRage);
            BB->SetValueAsBool(TEXT("IsDarknessActive"), bDarknessActive);
            BB->SetValueAsBool(TEXT("IsBerserkMode"), bIsBerserk);
        }
    }
}

// --- Darkness overlap handlers ---

void ALCBossEoduksini::OnDarknessSphereBeginOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (bDarknessActive)
    {
        return;  // 어둠 상태가 활성화된 뒤에만 처리
    }

    // ① OtherActor를 Pawn으로 캐스트
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn)
    {
        return;
    }

    // ② Pawn에서 PlayerController를 얻음
    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC)
    {
        return;
    }
    if (!PC->IsLocalController())
    {
        return;
    }

    // ③ 이미 어둠 효과가 적용된 플레이어는 무시
    if (DarkenedPlayers.Contains(PC))
    {
        UE_LOG(LogTemp, Warning, TEXT("[OverlapBegin] 이미 DarkenedPlayers에 있음 → 리턴"));
        return;
    }

    // Fade In 적용
    PC->PlayerCameraManager->StartCameraFade(
        0.f, DarknessFadeAlpha,
        FadeDuration, FLinearColor::Black,
        false, true
    );

    DarkenedPlayers.Add(PC);
    UE_LOG(LogTemp, Log, TEXT("[OverlapBegin] DarkenedPlayers에 추가: %s"), *PC->GetName());
}

void ALCBossEoduksini::OnDarknessSphereEndOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    // 전역 어둠 중이라면 로컬 어둠 해제 무시
    if (bDarknessActive)
    {
        return;
    }

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn) return;

    APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (!PC || !PC->IsLocalController()) return;

    if (!DarkenedPlayers.Contains(PC)) return;

    // Fade Out 처리
    UE_LOG(LogTemp, Log, TEXT("[OverlapEnd] 로컬 Fade Out: %s"), *PC->GetName());
    PC->PlayerCameraManager->StartCameraFade(
        DarknessFadeAlpha, 0.f,
        FadeDuration, FLinearColor::Black,
        false, false
    );

    DarkenedPlayers.Remove(PC);
}

// --- gaze checking ---

bool ALCBossEoduksini::IsLookedAtByAnyPlayer() const
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (IsPlayerLooking(It->Get())) return true;
    }
    return false;
}

bool ALCBossEoduksini::IsPlayerLooking(APlayerController* PC) const
{
    if (!PC) return false;
    FVector ViewLoc; FRotator ViewRot;
    PC->GetPlayerViewPoint(ViewLoc, ViewRot);
    FVector ToBoss = (GetActorLocation() - ViewLoc).GetSafeNormal();
    float Angle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(ViewRot.Vector(), ToBoss)));
    return Angle <= LookAngleDeg && PC->LineOfSightTo(this);
}

// --- Rage & scale update ---

void ALCBossEoduksini::UpdateRageAndScale(float DeltaSeconds)
{
    bool bLooked = IsLookedAtByAnyPlayer();
    float Delta = (bLooked ? -RageLossPerSec : RageGainPerSec) * DeltaSeconds;
    if (bIsBerserk) Delta *= BerserkRageGainMultiplier;
    Rage = FMath::Clamp(Rage + Delta, 0.f, MaxRage);
}

// --- Darkness state ---

void ALCBossEoduksini::TryTriggerDarkness()
{
    // 1) Darkness 상태 활성화
    bDarknessActive = true;
    Multicast_StartDarkness();

    // 2) 일정 시간 후 Darkness 종료 예약
    GetWorldTimerManager().SetTimer(
        DarknessTimerHandle,
        this,
        &ALCBossEoduksini::EndDarkness,
        DarknessDuration,
        false
    );
}

void ALCBossEoduksini::EndDarkness()
{
    if (!bDarknessActive) return;
    bDarknessActive = false;
    Multicast_EndDarkness();
}

void ALCBossEoduksini::OnRep_DarknessActive()
{
    if (bDarknessActive) BP_StartDarknessEffect();
    else                BP_EndDarknessEffect();
}

void ALCBossEoduksini::Multicast_StartDarkness_Implementation()
{
    BP_StartDarknessEffect();
}

void ALCBossEoduksini::Multicast_EndDarkness_Implementation()
{
    BP_EndDarknessEffect();
}

void ALCBossEoduksini::BP_StartDarknessEffect_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[Darkness] 클라이언트: 화면 어둡게 처리 시작"));

    // 1) 이 클라이언트에서 로컬 컨트롤러를 찾아 페이드 처리
    if (UWorld* World = GetWorld())
    {
        for (auto It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (!PC || !PC->IsLocalController())
                continue;

            // 각 로컬 플레이어 화면에 Fade In 적용
            PC->PlayerCameraManager->StartCameraFade(
                0.f,                  // 시작 Alpha
                DarknessFadeAlpha,    // 목표 Alpha
                FadeDuration,         // 페이드 시간
                FLinearColor::Black,  // Black으로 페이드
                false,                // bHoldWhenFinished = false
                true                  // bFadeAudio = true (필요 시 사운드도 페이드)
            );
        }
    }
}

void ALCBossEoduksini::BP_EndDarknessEffect_Implementation()
{
    if (UWorld* World = GetWorld())
    {
        for (auto It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (!PC || !PC->IsLocalController())
                continue;

            APawn* Pawn = PC->GetPawn();
            if (!Pawn)
                continue;

            // 보스와 플레이어 간의 거리 계산
            float DistToBoss = FVector::Dist(
                Pawn->GetActorLocation(),
                GetActorLocation());

            // 보스의 DarknessRadius보다 작으면 페이드 아웃을 하지 않고 건너뜀
            if (DistToBoss < DarknessRadius)
            {
                UE_LOG(LogTemp, Log, TEXT("[Darkness] 페이드 아웃 스킵 (거리 %.1f < Radius %.1f)"),
                    DistToBoss, DarknessRadius);
                continue;
            }

            // 해당 플레이어에 대해서만 Fade Out 적용
            PC->PlayerCameraManager->StartCameraFade(
                DarknessFadeAlpha, // 시작 Alpha
                0.f,               // 목표 Alpha
                FadeDuration,      // 페이드 시간
                FLinearColor::Black,
                false,
                false
            );
        }
    }
}

// --- Abilities ---

void ALCBossEoduksini::PhaseShift()
{
    if (!HasAuthority()) return;

    FVector Dest;
    if (bIsBerserk)
    {
        // cluster to most players
        TArray<APlayerController*> PCs;
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            PCs.Add(It->Get());
        }
        // simple: teleport to first player's location
        if (PCs.Num() > 0 && PCs[0]->GetPawn())
            Dest = PCs[0]->GetPawn()->GetActorLocation();
        else
            Dest = GetActorLocation();
    }
    else
    {
        UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        FNavLocation Loc;
        Nav->GetRandomPointInNavigableRadius(GetActorLocation(), DarknessRadius, Loc);
        Dest = Loc.Location;
    }
    SetActorLocation(Dest);
}

void ALCBossEoduksini::ShadowEcho()
{
    if (!HasAuthority()) return;

    FVector EchoLoc = GetActorLocation() + GetActorForwardVector() * 500.f;
    // spawn VFX here if desired

    FTimerDelegate Delegate = FTimerDelegate::CreateUObject(
        this, &ALCBossEoduksini::ExecuteShadowEchoDamage, EchoLoc
    );

    GetWorldTimerManager().SetTimer(
        ShadowEchoDamageHandle,  // ← lvalue 핸들
        Delegate,
        ShadowEchoDelay,
        false
    );
}

void ALCBossEoduksini::ExecuteShadowEchoDamage(FVector Location)
{
    float Radius = 400.f;
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    bool bHit = GetWorld()->SweepMultiByChannel(Hits, Location, Location, FQuat::Identity, ECC_Pawn, Sphere);
    if (bHit)
    {
        for (auto& Hit : Hits)
        {
            if (APawn* P = Cast<APawn>(Hit.GetActor()))
            {
                if (P->IsPlayerControlled())
                {
                    UGameplayStatics::ApplyDamage(P, NormalAttackDamage * 0.5f, GetController(), this, nullptr);
                    // apply slow: could interface with character movement
                    if (auto* Ch = Cast<ACharacter>(P))
                    {
                        Ch->GetCharacterMovement()->MaxWalkSpeed *= 0.5f;
                    }
                }
            }
        }
    }
}

void ALCBossEoduksini::NightmareGrasp()
{
    if (!HasAuthority()) return;

    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 800.f;
    FHitResult Hit;
    FCollisionQueryParams Params(NAME_None, false, this);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
    {
        if (APawn* P = Cast<APawn>(Hit.GetActor()))
        {
            UGameplayStatics::ApplyDamage(P, StrongAttackDamage, GetController(), this, nullptr);
            // stun: disable movement briefly
            if (auto* Ch = Cast<ACharacter>(P))
            {
                Ch->GetCharacterMovement()->DisableMovement();
                FTimerHandle Unused;
                GetWorldTimerManager().SetTimer(Unused, [Ch]() { Ch->GetCharacterMovement()->SetMovementMode(MOVE_Walking); }, 1.5f, false);
            }
        }
    }
}

void ALCBossEoduksini::NightTerror()
{
    // full-screen terror: 모든 로컬 플레이어에게 셰이크 적용
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get(); PC->IsLocalController())
        {
            if (TerrorCameraShakeClass)
            {
                // 에디터에서 할당한 셰이크 클래스로 안전하게 호출
                PC->PlayerCameraManager->StartCameraShake(TerrorCameraShakeClass, 1.f);
            }
        }
    }

    // radial damage
    UGameplayStatics::ApplyRadialDamage(
        this,
        StrongAttackDamage * 2.f,
        GetActorLocation(),
        600.f,
        nullptr, {}, this, GetController(), true
    );
}

// --- basic attacks ---

void ALCBossEoduksini::ShadowSwipe()
{
    DealDamageInRange(NormalAttackDamage);
}

void ALCBossEoduksini::VoidGrasp()
{
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(600.f);
    if (GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& Hit : Hits)
        {
            if (APawn* P = Cast<APawn>(Hit.GetActor()))
            {
                FVector PullLoc = GetActorLocation() + GetActorForwardVector() * 200.f;
                P->SetActorLocation(PullLoc);
                UGameplayStatics::ApplyDamage(P, StrongAttackDamage, GetController(), this, nullptr);
            }
        }
    }
}

// --- override base attack to include rage logic ---

bool ALCBossEoduksini::RequestAttack()
{
    if (!HasAuthority()) return false;

    const float Now = GetWorld()->GetTimeSeconds();
    const float Rand = FMath::FRand();

    // 1) Night Terror
    if (!bHasUsedNightTerror)
    {
        bHasUsedNightTerror = true;
        NightTerror();
        return true;
    }

    // 2) Phase Shift
    if (Rand < PhaseShiftChance && Now - LastPhaseShiftTime >= PhaseShiftInterval)
    {
        LastPhaseShiftTime = Now;
        PhaseShift();
        return true;
    }

    // 3) Shadow Echo
    if (Rand < ShadowEchoChance && Now - LastShadowEchoTime >= ShadowEchoInterval)
    {
        LastShadowEchoTime = Now;
        ShadowEcho();
        return true;
    }

    // 4) Nightmare Grasp
    if (Rand < NightmareGraspChance && Now - LastNightmareGraspTime >= NightmareGraspInterval)
    {
        LastNightmareGraspTime = Now;
        NightmareGrasp();
        return true;
    }

    return false;
}

void ALCBossEoduksini::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossEoduksini, bDarknessActive);
}