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
    int32 TotalPlayers = 0;
    int32 LookCount = 0;

    // 모든 플레이어 컨트롤러를 순회하면서
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        APawn* Pawn = PC->GetPawn();
        if (!Pawn || !Pawn->IsPlayerControlled())
            continue;

        ++TotalPlayers;

        // 플레이어가 보스 바라보고 있으면 카운트
        if (IsPlayerLooking(PC))
            ++LookCount;
    }

    // 바라보는 플레이어 수에 비례해서 Rage 감소,
    // 바라보지 않는 플레이어 수에 비례해서 Rage 증가
    int32 NotLookCount = TotalPlayers - LookCount;

    float DeltaRage = 0.f;
    DeltaRage += NotLookCount * RageGainPerSec * DeltaSeconds;
    DeltaRage -= LookCount * RageLossPerSec * DeltaSeconds;

    // Berserk 중엔 증가/감소율 배수 적용
    if (bIsBerserk)
    {
        DeltaRage *= BerserkRageGainMultiplier;
    }

    Rage = FMath::Clamp(Rage + DeltaRage, 0.f, MaxRage);
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

void ALCBossEoduksini::ShadowEcho()
{
    UE_LOG(LogTemp, Log, TEXT("[Eoduksini] ShadowEcho 시작 (딜레이: %.1f초)"), ShadowEchoDelay);

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
    UE_LOG(LogTemp, Log, TEXT("[Eoduksini] ShadowEcho 폭발 데미지 수행 (위치: %s)"),
        *Location.ToCompactString());

    float Radius = 400.f;

    // 1) Debug: 폭발 위치에 보라색 구 그리기 (2초간, 두께 5)
    DrawDebugSphere(
        GetWorld(),
        Location,
        Radius,
        16,
        FColor::Purple,
        false,
        2.0f,
        0,
        5.0f
    );

    // 2) 실제 SweepMultiByChannel
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits,
        Location,
        Location,
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );

    // 3) 히트된 각 Pawn에 대해 디버그 라인과 대미지 적용
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
    UE_LOG(LogTemp, Log, TEXT("[Eoduksini] NightmareGrasp 실행"));

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
    UE_LOG(LogTemp, Log, TEXT("[Eoduksini] NightTerror 실행"));

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
    UE_LOG(LogTemp, Log, TEXT("[Eoduksini] ShadowSwipe 실행: 대미지 %.1f"), NormalAttackDamage);

    DealDamageInRange(NormalAttackDamage);



}

void ALCBossEoduksini::VoidGrasp()
{
    UE_LOG(LogTemp, Log, TEXT("[Eoduksini] VoidGrasp 실행: 대미지 %.1f"), StrongAttackDamage);

    FVector BossLoc = GetActorLocation();
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(VoidGraspRange);

    // 반경 내 Pawn 검색
    if (GetWorld()->SweepMultiByChannel(Hits, BossLoc, BossLoc, FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& Hit : Hits)
        {
            if (APawn* P = Cast<APawn>(Hit.GetActor()))
            {
                // 당기는 방향 계산
                FVector PullDir = (BossLoc - P->GetActorLocation()).GetSafeNormal();

                // 캐릭터라면 LaunchCharacter 로 물리 이동
                if (ACharacter* Ch = Cast<ACharacter>(P))
                {
                    const float PullStrength = 1500.f; // 필요에 따라 조정
                    // XY/Z 모두 덮어쓰도록 true,true
                    Ch->LaunchCharacter(PullDir * PullStrength, true, true);
                }
                // 만약 물리 시뮬레이션 컴포넌트라면 AddImpulse 사용
                else if (UPrimitiveComponent* Prim = Hit.GetComponent())
                {
                    if (Prim->IsSimulatingPhysics())
                    {
                        const float ImpulseStrength = 800.f;
                        Prim->AddImpulse(PullDir * ImpulseStrength, NAME_None, true);
                    }
                }

                // 대미지 적용
                UGameplayStatics::ApplyDamage(
                    P,
                    StrongAttackDamage,
                    GetController(),
                    this,
                    nullptr
                );
            }
        }
    }
}

// --- override base attack to include rage logic ---

bool ALCBossEoduksini::RequestAttack(float TargetDistance)
{
    if (!HasAuthority()) return false;

    const float Now = GetWorld()->GetTimeSeconds();
    struct FEntry { float Weight; TFunction<void()> Action; };
    TArray<FEntry> Entries;

    // (1) 특수 액션—예: Night Terror
    if (!bHasUsedNightTerror && Rage >= NightTerrorRageThreshold)
    {
        bHasUsedNightTerror = true;
        UE_LOG(LogTemp, Log, TEXT("[Eoduksini] NightTerror 발동"));
        NightTerror();
        return true;
    }

    // 2) ShadowEcho
    if (TargetDistance <= ShadowEchoRange && Now - LastShadowEchoTime >= ShadowEchoInterval)
    {
        Entries.Add({ ShadowEchoWeight, [this, Now]()
        {
            LastShadowEchoTime = Now;
            UE_LOG(LogTemp, Log, TEXT("[Eodu] ShadowEcho 실행"));
            ShadowEcho();
        } });
    }

    // 3) NightmareGrasp
    if (TargetDistance <= NightmareGraspRange && Now - LastNightmareGraspTime >= NightmareGraspInterval)
    {
        Entries.Add({ NightmareGraspWeight, [this, Now]()
        {
            LastNightmareGraspTime = Now;
            UE_LOG(LogTemp, Log, TEXT("[Eodu] NightmareGrasp 실행"));
            NightmareGrasp();
        } });
    }

    // 4) 근접계열 (ShadowSwipe)
    if (TargetDistance <= ShadowSwipeRange && Now - LastNormalTime >= NormalAttackCooldown)
    {
        Entries.Add({ ShadowSwipeWeight, [this, Now]()
        {
            LastNormalTime = Now;
            UE_LOG(LogTemp, Log, TEXT("[Eodu] ShadowSwipe 실행"));
            ShadowSwipe();
        } });
    }

    // 5) 견인계열 (VoidGrasp)
    if (TargetDistance <= VoidGraspRange && Now - LastStrongTime >= StrongAttackCooldown)
    {
        Entries.Add({ VoidGraspWeight, [this, Now]()
        {
            LastStrongTime = Now;
            UE_LOG(LogTemp, Log, TEXT("[Eodu] VoidGrasp 실행"));
            VoidGrasp();
        } });
    }

    // 가중치 랜덤 선택
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

void ALCBossEoduksini::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossEoduksini, bDarknessActive);
}