#include "AI/LCBossEoduksini.h"
#include "AI/LCBaseBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALCBossEoduksini::ALCBossEoduksini()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // 초기값
    CurScale = MaxScale;
}

void ALCBossEoduksini::BeginPlay()
{
    Super::BeginPlay();
    // 시작 스케일 적용
    SetActorScale3D(FVector(CurScale));

    if (HasAuthority())
    {
        const float InitialDelay = FMath::RandRange(ClueSpawnIntervalMin, ClueSpawnIntervalMax);
        GetWorldTimerManager().SetTimer(
            ClueTimerHandle,
            this,
            &ALCBossEoduksini::SpawnRandomClue,
            InitialDelay,
            false
        );
    }
}

void ALCBossEoduksini::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;

    // 분노·스케일 자동 업데이트
    UpdateRageAndScale(DeltaSeconds);

    // Darkness 자동 트리거
    TryTriggerDarkness();

    // — 거리 체크로 어둠 ON/OFF —
    if (Rage >= DarknessRage)
    {
        UWorld* World = GetWorld();
        for (auto It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (!PC || !PC->IsLocalController()) continue;
            APawn* Pawn = PC->GetPawn();
            if (!Pawn) continue;

            const float Dist = FVector::Dist(
                Pawn->GetActorLocation(), GetActorLocation());
            const bool bInside = (Dist <= DarknessRadius);
            const bool bDark = DarkenedPlayers.Contains(PC);

            if (bInside && !bDark)
            {
                // 반경 안으로 들어옴 → Fade In
                PC->PlayerCameraManager->StartCameraFade(
                    0.f, DarknessFadeAlpha,
                    FadeDuration, FLinearColor::Black,
                    false, true);
                DarkenedPlayers.Add(PC);
            }
            else if (!bInside && bDark)
            {
                // 반경 밖으로 나감 → Fade Out
                PC->PlayerCameraManager->StartCameraFade(
                    DarknessFadeAlpha, 0.f,
                    FadeDuration, FLinearColor::Black,
                    false, false);
                DarkenedPlayers.Remove(PC);
            }
        }
    }

    // Blackboard 갱신
    if (auto* AICon = Cast<ALCBaseBossAIController>(GetController()))
    {
        if (auto* BB = AICon->GetBlackboardComponent())
        {
            BB->SetValueAsFloat(TEXT("RagePercent"), Rage / MaxRage);
            BB->SetValueAsBool(TEXT("IsDarknessActive"), bDarknessActive);
        }
    }
}

void ALCBossEoduksini::SpawnRandomClue()
{
    // 1) ScratchMarkClass / ShadowStainClass 가 제대로 지정되었는지 확인
    if (!ScratchMarkClass || !ShadowStainClass)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[SpawnRandomClue] 클래스가 지정되지 않음: Scratch=%s, Shadow=%s"),
            *GetNameSafe(ScratchMarkClass), *GetNameSafe(ShadowStainClass));

        // 다음 타이머 예약만 재설정 후 반환
        float NextDelay = FMath::RandRange(ClueSpawnIntervalMin, ClueSpawnIntervalMax);
        GetWorldTimerManager().SetTimer(
            ClueTimerHandle,
            this,
            &ALCBossEoduksini::SpawnRandomClue,
            NextDelay,
            false
        );
        return;
    }

    // 2) 절반 확률로 ScratchMark 또는 ShadowStain 중 하나 선택
    bool bSpawnScratch = FMath::RandBool();
    TSubclassOf<AActor> ChosenClass = bSpawnScratch
        ? ScratchMarkClass
        : ShadowStainClass;

    // 3) 스폰 위치를 보스 위치 주변에 약간 오프셋
    FVector BossLoc = GetActorLocation();
    FVector Forward = GetActorForwardVector();
    FVector Right = GetActorRightVector();
    FVector Up = FVector::UpVector;

    float OffsetForward = FMath::RandRange(-50.f, 50.f);
    float OffsetRight = FMath::RandRange(-100.f, 100.f);
    float OffsetUp = FMath::RandRange(-20.f, 20.f);

    FVector SpawnLoc = BossLoc
        + Forward * OffsetForward
        + Right * OffsetRight
        + Up * OffsetUp;

    FRotator SpawnRot = GetActorRotation();

    // 4) SpawnParameters 설정 (충돌이 있더라도 무조건 스폰하도록 설정)
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 5) 실제로 액터 스폰 시도
    AActor* NewClue = GetWorld()->SpawnActor<AActor>(
        ChosenClass, SpawnLoc, SpawnRot, SpawnParams);

    if (NewClue)
    {
        UE_LOG(LogTemp, Log,
            TEXT("[SpawnRandomClue] %s 을(를) %s 에 스폰 성공"),
            *NewClue->GetName(), *SpawnLoc.ToCompactString());

        SpawnedClues.Add(NewClue);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[SpawnRandomClue] 스폰 실패: 클래스=%s, 위치=%s"),
            *GetNameSafe(ChosenClass), *SpawnLoc.ToCompactString());
    }

    // 6) 다음 단서를 스폰하기 위해 타이머를 다시 예약
    float NextDelay = FMath::RandRange(ClueSpawnIntervalMin, ClueSpawnIntervalMax);
    GetWorldTimerManager().SetTimer(
        ClueTimerHandle,
        this,
        &ALCBossEoduksini::SpawnRandomClue,
        NextDelay,
        false
    );
}

bool ALCBossEoduksini::IsLookedAtByAnyPlayer() const
{
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
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
        
    FVector Dir = (GetActorLocation() - ViewLoc).GetSafeNormal();
    float AngleDeg = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(ViewRot.Vector(), Dir)));

    return AngleDeg <= LookAngleDeg
        && PC->LineOfSightTo(this, ViewLoc, true);
}

void ALCBossEoduksini::UpdateRageAndScale(float DeltaSeconds)
{
    const bool bLooked = IsLookedAtByAnyPlayer();

    float DeltaRage = (bLooked ? -RageLossPerSec : RageGainPerSec) * DeltaSeconds;
    Rage = FMath::Clamp(Rage + DeltaRage, 0.f, MaxRage);

    //float Target = bLooked ? MinScale : MaxScale;
    //CurScale = FMath::FInterpTo(CurScale, Target, DeltaSeconds, ScaleInterpSpeed);
    //SetActorScale3D(FVector(CurScale));
}

void ALCBossEoduksini::TryTriggerDarkness()
{
    if (bDarknessActive && Rage < MaxRage) return;
    bDarknessActive = true;
    Multicast_StartDarkness();
    GetWorldTimerManager().SetTimer(
        DarknessTimer, this,
        &ALCBossEoduksini::EndDarkness,
        DarknessDuration, false
    );
}

void ALCBossEoduksini::EndDarkness()
{
    if (!bDarknessActive) return;
    bDarknessActive = false;
    Multicast_EndDarkness();
}

void ALCBossEoduksini::OnRep_CurScale()
{
    SetActorScale3D(FVector(CurScale));
}

void ALCBossEoduksini::OnRep_DarknessActive()
{
    if (bDarknessActive) BP_StartDarknessEffect();
    else                BP_EndDarknessEffect();
}

void ALCBossEoduksini::BP_StartDarknessEffect_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[Darkness] 시작!"));

    UWorld* World = GetWorld();
    if (!World) return;

    for (auto It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC || !PC->IsLocalController())
            continue;
        APawn* Pawn = PC->GetPawn();
        if (!Pawn)
            continue;


        //PC->PlayerCameraManager->StartCameraFade(
        //    0.f,
        //    DarknessFadeAlpha,
        //    FadeDuration,
        //    FLinearColor::Black,
        //    false,
        //    true
        //    );
    }
}

void ALCBossEoduksini::BP_EndDarknessEffect_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[Darkness] 종료!"));

    UWorld* World = GetWorld();
    if (!World) return;

    for (auto It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC || !PC->IsLocalController())
            continue;
        APawn* Pawn = PC->GetPawn();
        if (!Pawn)
            continue;

        //PC->PlayerCameraManager->StartCameraFade(
        //    DarknessFadeAlpha,
        //    0.f,
        //    FadeDuration,
        //    FLinearColor::Black,
        //    false,
        //    false
        //    );
    }
}


void ALCBossEoduksini::Multicast_StartDarkness_Implementation()
{

    UE_LOG(LogTemp, Warning, TEXT("→ Multicast_StartDarkness 호출"));
    BP_StartDarknessEffect();
}

void ALCBossEoduksini::Multicast_EndDarkness_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("→ Multicast_EndDarkness 호출"));
    BP_EndDarknessEffect();
}

bool ALCBossEoduksini::RequestAttack()
{
    if (!HasAuthority()) return false;

    const float Now = GetWorld()->GetTimeSeconds();
    const float RagePct = Rage / MaxRage;

    // 강공격 우선
    bool bStrongOK =
        FMath::FRand() < StrongAttackChance &&
        (Now - LastStrongTime) >= StrongAttackCooldown &&
        RagePct >= (DarknessRage / MaxRage);
    if (bStrongOK)
    {
        LastStrongTime = Now;
        PlayStrongWithRage();
        return true;
    }

    // 일반 공격
    if ((Now - LastNormalTime) >= NormalAttackCooldown)
    {
        LastNormalTime = Now;
        PlayNormalWithRage();
        return true;
    }

    return false;
}

void ALCBossEoduksini::PlayNormalWithRage()
{
    PlayNormalAttack();  // 부모에서 몽타주 실행
    Rage = FMath::Clamp(Rage + RageGain_Normal, 0.f, MaxRage);
}

void ALCBossEoduksini::PlayStrongWithRage()
{
    PlayStrongAttack();  // 부모에서 몽타주 실행
    // 이 보스만의 데미지 로직이 필요하면 여기에 추가
    Rage = FMath::Clamp(Rage - RageLoss_Strong, 0.f, MaxRage);
}

void ALCBossEoduksini::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossEoduksini, CurScale);
    DOREPLIFETIME(ALCBossEoduksini, bDarknessActive);
}