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

    // ── DarknessSphere 생성 ──
    DarknessSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DarknessSphere"));
    DarknessSphere->SetupAttachment(GetRootComponent());
    DarknessSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    DarknessSphere->SetGenerateOverlapEvents(false); // 처음에는 비활성화
    DarknessSphere->SetSphereRadius(DarknessRadius);

    // Overlap 콜백 바인딩 (BeginPlay에서 활성화 타이밍 설정)
    DarknessSphere->OnComponentBeginOverlap.AddDynamic(this, &ALCBossEoduksini::OnDarknessSphereBeginOverlap);
    DarknessSphere->OnComponentEndOverlap.AddDynamic(this, &ALCBossEoduksini::OnDarknessSphereEndOverlap);


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

    // 1) ClueClasses 내용을 RemainingClueClasses로 복사
    RemainingClueClasses = ClueClasses;

    // 2) 남은 단서가 있을 때만 타이머 시작
    if (HasAuthority() && RemainingClueClasses.Num() > 0)
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

    // BeginPlay 시점에 Sphere 반경 설정을 한 번 더 보장
    DarknessSphere->SetSphereRadius(DarknessRadius);
    DarknessSphere->SetGenerateOverlapEvents(true);
}

void ALCBossEoduksini::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;

    // 분노·스케일 자동 업데이트
    UpdateRageAndScale(DeltaSeconds);

	if (bIsBerserk && Rage >= MaxRage && !bDarknessActive)
	{
		// 광폭화 상태에서 Darkness가 활성화되지 않았다면 자동으로 트리거
		TryTriggerDarkness();
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
    if (bIsBerserk)
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
    // 1) Darkness 상태 활성화
    bDarknessActive = true;
    Multicast_StartDarkness();

    // 2) 일정 시간 후 Darkness 종료 예약
    GetWorldTimerManager().SetTimer(
        DarknessTimer,
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

    float CurrentStrongCooldown = bIsBerserk ? StrongAttackCooldown_Berserk : StrongAttackCooldown;
    float CurrentStrongChance = bIsBerserk ? StrongAttackChance_Berserk : StrongAttackChance;

    // (1) 강공격 우선
    if (StrongAttackMontage &&
        FMath::FRand() < CurrentStrongChance &&
        (Now - LastStrongTime) >= CurrentStrongCooldown &&
        (bIsBerserk || RagePct >= (DarknessRage / MaxRage)))
    {
        LastStrongTime = Now;
        PlayStrongWithRage();
        return true;
    }

    // (2) 일반 공격
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
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->OnMontageEnded.RemoveDynamic(this, &ALCBossEoduksini::OnAttackMontageEnded);
        float PlayRate = bIsBerserk ? BerserkPlayRateMultiplier : 1.0f;
        Anim->Montage_Play(NormalAttackMontage, PlayRate);
        Anim->OnMontageEnded.AddDynamic(this, &ALCBossEoduksini::OnAttackMontageEnded);
    }

    // Rage 증가
    Rage = FMath::Clamp(Rage + RageGain_Normal, 0.f, MaxRage);
}

void ALCBossEoduksini::PlayStrongWithRage()
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        Anim->OnMontageEnded.RemoveDynamic(this, &ALCBossEoduksini::OnAttackMontageEnded);
        float PlayRate = bIsBerserk ? BerserkPlayRateMultiplier : 1.0f;
        Anim->Montage_Play(StrongAttackMontage, PlayRate);
        Anim->OnMontageEnded.AddDynamic(this, &ALCBossEoduksini::OnAttackMontageEnded);
    }

    // Rage 소모
    float Consumed = bIsBerserk ? RageLoss_Strong * 0.8f : RageLoss_Strong;
    Rage = FMath::Clamp(Rage - Consumed, 0.f, MaxRage);
}

void ALCBossEoduksini::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossEoduksini, CurScale);
    DOREPLIFETIME(ALCBossEoduksini, bDarknessActive);
}