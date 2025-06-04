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

    // 기본 Berserk 차등값 설정 (필요에 따라 블루프린트에서 조정 가능)
    BerserkRageGainMultiplier = 2.0f;
    StrongAttackChance_Berserk = 0.6f;
    NormalAttackCooldown_Berserk = 0.6f;
    StrongAttackCooldown_Berserk = 3.0f;
    BerserkPlayRateMultiplier = 1.3f;
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
			BB->SetValueAsBool(TEXT("IsBerserkMode"), bIsBerserk);
        }
    }
}

void ALCBossEoduksini::SpawnRandomClue()
{
    // 1) ClueClasses가 하나라도 등록되어 있는지 확인
    if (ClueClasses.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnRandomClue] ClueClasses 배열이 비어있음"));
        // 다음 스폰 예약만 설정 후 리턴
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

    // 2) 배열 인덱스를 랜덤하게 뽑아서 스폰할 클래스 선택
    int32 Index = FMath::RandRange(0, ClueClasses.Num() - 1);
    TSubclassOf<AActor> ChosenClass = ClueClasses[Index];
    if (!ChosenClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SpawnRandomClue] 선택된 Clue 클래스가 유효하지 않음: Index=%d"), Index);
        // 다음 스폰 예약만 설정 후 리턴
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

    // 3) 스폰 위치를 보스 주변에 랜덤 오프셋
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

    // 4) 스폰 파라미터 설정
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 5) 실제 액터 스폰 시도
    AActor* NewClue = GetWorld()->SpawnActor<AActor>(
        ChosenClass, SpawnLoc, SpawnRot, SpawnParams);

    if (NewClue)
    {
        UE_LOG(LogTemp, Log,
            TEXT("[SpawnRandomClue] %s 을(를) 인덱스 %d 로 스폰 성공 (위치=%s)"),
            *NewClue->GetName(),
            Index,
            *SpawnLoc.ToCompactString());
        SpawnedClues.Add(NewClue);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("[SpawnRandomClue] 스폰 실패: 클래스=%s, 인덱스=%d, 위치=%s"),
            *GetNameSafe(ChosenClass),
            Index,
            *SpawnLoc.ToCompactString());
    }

    // 6) 다음 단서 스폰 예약
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

    // 광폭화 중이면 배수 적용
    if (bIsBerserk && !bLooked)
    {
        DeltaRage *= BerserkRageGainMultiplier;
    }

    Rage = FMath::Clamp(Rage + DeltaRage, 0.f, MaxRage);

    //float Target = bLooked ? MinScale : MaxScale;
    //CurScale = FMath::FInterpTo(CurScale, Target, DeltaSeconds, ScaleInterpSpeed);
    //SetActorScale3D(FVector(CurScale));
}

void ALCBossEoduksini::TryTriggerDarkness()
{
    if (bDarknessActive || Rage < MaxRage) return;
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

    bool bStrongOK = false;
    float CurrentStrongCooldown = StrongAttackCooldown;
    float CurrentStrongChance = StrongAttackChance;

    // 광폭화라면 쿨다운 짧게, 확률 높게, RagePct 제한을 해제
    if (bIsBerserk)
    {
        CurrentStrongCooldown = StrongAttackCooldown_Berserk;
        CurrentStrongChance = StrongAttackChance_Berserk;
    }

    // (1) 강공격 우선
    if (StrongAttackMontage &&
        FMath::FRand() < CurrentStrongChance &&
        (Now - LastStrongTime) >= CurrentStrongCooldown &&
        // 광폭화가 아니라면 기존 RagePct 조건 적용
        (bIsBerserk || RagePct >= (DarknessRage / MaxRage)))
    {
        LastStrongTime = Now;
        PlayStrongWithRage();
        return true;
    }

    // (2) 일반 공격 (광폭화 시 쿨다운 단축)
    float CurrentNormalCooldown = bIsBerserk ? NormalAttackCooldown_Berserk : NormalAttackCooldown;
    if (NormalAttackMontage &&
        (Now - LastNormalTime) >= CurrentNormalCooldown)
    {
        LastNormalTime = Now;
        PlayNormalWithRage();
        return true;
    }

    return false;
}

void ALCBossEoduksini::PlayNormalWithRage()
{
    // 몽타주 재생 속도를 광폭화 시 빠르게
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        float PlayRate = bIsBerserk ? BerserkPlayRateMultiplier : 1.0f;
        Anim->Montage_Play(NormalAttackMontage, PlayRate);
        Anim->OnMontageEnded.AddDynamic(this, &ALCBossEoduksini::OnAttackMontageEnded);
    }
    // 기본 Rage 증가
    Rage = FMath::Clamp(Rage + RageGain_Normal, 0.f, MaxRage);
}

void ALCBossEoduksini::PlayStrongWithRage()
{
    // 몽타주 재생 속도를 광폭화 시 빠르게
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        float PlayRate = bIsBerserk ? BerserkPlayRateMultiplier : 1.0f;
        Anim->Montage_Play(StrongAttackMontage, PlayRate);
        Anim->OnMontageEnded.AddDynamic(this, &ALCBossEoduksini::OnAttackMontageEnded);
    }
    // Rage 소비량도 광폭화 시 약간 감소(더 자주 강공격을 시도할 수 있게)
    float ConsumedRage = bIsBerserk ? RageLoss_Strong * 0.8f : RageLoss_Strong;
    Rage = FMath::Clamp(Rage - ConsumedRage, 0.f, MaxRage);
}

void ALCBossEoduksini::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossEoduksini, CurScale);
    DOREPLIFETIME(ALCBossEoduksini, bDarknessActive);
}