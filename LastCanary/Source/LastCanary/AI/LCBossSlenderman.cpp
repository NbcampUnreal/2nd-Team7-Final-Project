#include "AI/LCBossSlenderman.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineTypes.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "AI/LCBaseBossAIController.h"
#include "Camera/CameraComponent.h"


ALCBossSlenderman::ALCBossSlenderman()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ALCBossSlenderman::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(FearTimerHandle, this, &ALCBossSlenderman::ExecuteFearWave, FearInterval, true);
    GetWorldTimerManager().SetTimer(WhisperTimerHandle, this, &ALCBossSlenderman::ExecuteAbyssalWhisper, WhisperInterval, true);
    GetWorldTimerManager().SetTimer(TeleportTimerHandle, this, &ALCBossSlenderman::TeleportToRandomLocation, TeleportInterval, true);
    GetWorldTimerManager().SetTimer(DistortionTimerHandle, this, &ALCBossSlenderman::ExecuteDistortion, DistortionInterval, true);
    GetWorldTimerManager().SetTimer(EndlessStalkTimerHandle, this, &ALCBossSlenderman::EnterEndlessStalk, 60.f, false);
}

void ALCBossSlenderman::Tick(float DeltaTime)
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

bool ALCBossSlenderman::IsPlayerLookingAtMe(APawn* PlayerPawn) const
{
    FVector ToBoss = GetActorLocation() - PlayerPawn->GetActorLocation();
    FVector Forward = PlayerPawn->GetViewRotation().Vector();
    return FVector::DotProduct(Forward, ToBoss.GetSafeNormal()) > 0.8f;
}

void ALCBossSlenderman::UpdateRage(float DeltaSeconds)
{
    Super::UpdateRage(DeltaSeconds);

    // (1) 실제 플레이어만 찾아서
    float TotalDelta = 0.f;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = Cast<APlayerController>(*It))
        {
            if (APawn* P = PC->GetPawn())
            {
                // (2) 시야 체크
                FVector ToBoss = (GetActorLocation() - P->GetActorLocation()).GetSafeNormal();
                FVector Forward = P->GetViewRotation().Vector();
                bool bLooking = FVector::DotProduct(Forward, ToBoss) > LookDotThreshold;

                // (3) 누적
                float Rate = bLooking ? LookRagePerSecond : -LoseRagePerSecond;
                TotalDelta += Rate * DeltaSeconds;
            }
        }
    }

    // (4) 한 번만 Rage 갱신
    AddRage(TotalDelta);
}

void ALCBossSlenderman::AddRage(float Amount)
{
    Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);
    if (Rage >= MaxRage && !bIsBerserk)
        EnterBerserkState();
}

void ALCBossSlenderman::EnterBerserkState()
{
    Super::EnterBerserkState();
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] 영구 Berserk 진입"));
    // (서버) Speed 증가 등, 기존 로직만 남겨둡니다
    GetCharacterMovement()->MaxWalkSpeed *= BerserkSpeedMultiplier;
}

void ALCBossSlenderman::StartBerserk()
{
    Super::StartBerserk();  // bIsBerserk = true 및 Multicast 호출

    // (서버) 분신 소환 로직 등 원래 구현 유지
    if (ShadowCloneClass)
    {
        for (int32 i = 0; i < BerserkCloneCount; ++i)
        {
            FVector Offset = FMath::VRand() * 300.f;
            FActorSpawnParameters Params;
            Params.Owner = this;
            if (AActor* Clone = GetWorld()->SpawnActor<AActor>(
                ShadowCloneClass,
                GetActorLocation() + Offset,
                GetActorRotation(),
                Params))
            {
                ShadowClones.Add(Clone);
            }
        }
    }
}

void ALCBossSlenderman::StartBerserk(float Duration)
{
    // 서버: 먼저 무한 버전 실행
    StartBerserk();

    // 일정 시간 후 End 호출 예약
    GetWorldTimerManager().ClearTimer(BerserkTimerHandle);
    GetWorldTimerManager().SetTimer(
        BerserkTimerHandle,
        this, &ALCBossSlenderman::EndBerserk,
        Duration, false
    );
}

void ALCBossSlenderman::EndBerserk()
{
    Super::EndBerserk();
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Berserk 종료"));

    // (서버) Speed 원복 및 분신 정리
    GetCharacterMovement()->MaxWalkSpeed /= BerserkSpeedMultiplier;
    for (AActor* Clone : ShadowClones)
    {
        if (IsValid(Clone))
            Clone->Destroy();
    }
    ShadowClones.Empty();
}

void ALCBossSlenderman::OnRep_IsBerserk()
{
    Super::OnRep_IsBerserk();

    // **절대 StartBerserk() 호출 금지!**
    if (bIsBerserk)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Slenderman] OnRep → Berserk 이펙트 클라에서 재생"));

        // 클라이언트 연출: 이펙트 / 사운드
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

void ALCBossSlenderman::ExecuteFearWave()
{
    if (!HasAuthority() || !FearPostProcessMaterial) return;

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(FearRadius);
    if (GetWorld()->SweepMultiByChannel(
        Hits,
        GetActorLocation(),
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere))
    {
        for (auto& Hit : Hits)
        {
            if (APawn* P = Cast<APawn>(Hit.GetActor()))
            {
                const float Dist = FVector::Dist(P->GetActorLocation(), GetActorLocation());
                if (!IsPlayerLookingAtMe(P) && Dist < FearRadius)
                {
                    // 폰에 붙은 카메라 컴포넌트를 찾는다
                    if (UCameraComponent* CamComp = P->FindComponentByClass<UCameraComponent>())
                    {
                        // 포스트프로세스 머티리얼을 블렌더블로 추가
                        CamComp->PostProcessSettings.AddBlendable(FearPostProcessMaterial, FearPPBlendWeight);

                        // 일정 시간 후 제거
                        FTimerHandle TmpHandle;
                        GetWorldTimerManager().SetTimer(TmpHandle, [CamComp, this]() {
                            CamComp->PostProcessSettings.RemoveBlendable(FearPostProcessMaterial);
                            }, FearPPDuration, false);
                    }
                }
            }
        }
    }
}

void ALCBossSlenderman::ExecuteAbyssalWhisper()
{
    if (!HasAuthority()) return;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (auto* PC = Cast<APlayerController>(*It))
        {
            FVector TrueLoc = PC->GetPawn()->GetActorLocation();
            FVector FakeLoc = TrueLoc + FMath::VRand() * 500.f;
            UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Whisper distorted for %s: %s"),
                *PC->GetName(), *FakeLoc.ToString());
        }
    }
}

void ALCBossSlenderman::TeleportToRandomLocation()
{
    if (!HasAuthority()) return;
    if (auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation Loc;
        if (Nav->GetRandomPointInNavigableRadius(GetActorLocation(), 1000.f, Loc))
            SetActorLocation(Loc.Location);
    }
}

void ALCBossSlenderman::ExecuteDistortion()
{
    if (!HasAuthority()) return;
    Multicast_DistortionEffect();
}

void ALCBossSlenderman::Multicast_DistortionEffect_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Distortion Effect Triggered"));
    // BP 또는 머티리얼로 맵 왜곡 처리
}

void ALCBossSlenderman::EnterEndlessStalk()
{
    if (!HasAuthority() || bIsEndlessStalk) return;
    bIsEndlessStalk = true;
    Multicast_StartEndlessStalk();
}

void ALCBossSlenderman::OnRep_EndlessStalk()
{
    if (bIsEndlessStalk)
        GetCharacterMovement()->MaxWalkSpeed *= 2.f;
}

void ALCBossSlenderman::Multicast_StartEndlessStalk_Implementation()
{
    OnRep_EndlessStalk();
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Endless Stalk Activated"));
}

void ALCBossSlenderman::ExecuteReachSlash()
{
    GetWorldTimerManager().SetTimer(ReachSlashTimerHandle, ReachSlashCooldown, false);
    FVector O = GetActorLocation();
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(ReachSlashRadius);
    if (GetWorld()->SweepMultiByChannel(Hits, O, O, FQuat::Identity, ECC_Pawn, Sphere))
        for (auto& H : Hits)
            if (ACharacter* C = Cast<ACharacter>(H.GetActor()))
                UGameplayStatics::ApplyDamage(C, ReachSlashDamage, GetController(), this, nullptr);
}

void ALCBossSlenderman::ExecuteShadowGrasp()
{
    GetWorldTimerManager().SetTimer(ShadowGraspTimerHandle, ShadowGraspCooldown, false);
    if (auto* AC = Cast<AAIController>(GetController()))
    {
        if (auto* BB = AC->GetBlackboardComponent())
        {
            if (auto* T = Cast<ACharacter>(BB->GetValueAsObject(TEXT("TargetActor"))))
            {
                FVector Dir = (T->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                T->LaunchCharacter(-Dir * 1000.f, true, true);
            }
        }
    }
}

void ALCBossSlenderman::ExecuteAttackDistortion()
{
    GetWorldTimerManager().SetTimer(AttackDistortionTimerHandle, DistortionCooldown, false);
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Attack Distortion Triggered"));
}

bool ALCBossSlenderman::RequestAttack(float TargetDistance)
{
    if (!HasAuthority()) return false;
    const float Now = GetWorld()->GetTimeSeconds();
    AActor* Target = nullptr;
    if (auto* AC = Cast<AAIController>(GetController()))
        Target = Cast<AActor>(AC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));

    struct FEntry { float W; TFunction<void()> A; };
    TArray<FEntry> Entries;

    // Reach Slash
    if (Target && TargetDistance <= ReachSlashRadius && !GetWorldTimerManager().IsTimerActive(ReachSlashTimerHandle))
        Entries.Add({ 3.f, [this]() { ExecuteReachSlash(); } });

    // Shadow Grasp
    if (Target && TargetDistance <= ShadowGraspDistance && !GetWorldTimerManager().IsTimerActive(ShadowGraspTimerHandle))
        Entries.Add({ 2.f, [this]() { ExecuteShadowGrasp(); } });

    // Distortion Attack
    if (!GetWorldTimerManager().IsTimerActive(AttackDistortionTimerHandle))
        Entries.Add({ 1.f, [this]() { ExecuteAttackDistortion(); } });

    float Total = 0; for (auto& e : Entries) Total += e.W;
    float Pick = FMath::FRandRange(0.f, Total), Acc = 0;
    for (auto& e : Entries)
    {
        Acc += e.W;
        if (Pick <= Acc) { e.A(); return true; }
    }
    return false;
}

void ALCBossSlenderman::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALCBossSlenderman, bIsEndlessStalk);
}