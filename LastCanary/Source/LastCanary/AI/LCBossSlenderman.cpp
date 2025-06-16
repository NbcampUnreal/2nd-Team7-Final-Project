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

    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), Players);
    for (AActor* A : Players)
    {
        if (APawn* P = Cast<APawn>(A))
        {
            float Delta = IsPlayerLookingAtMe(P) ? +10.f : -10.f;
            AddRage(Delta * DeltaSeconds);
        }
    }
}

void ALCBossSlenderman::AddRage(float Amount)
{
    Rage = FMath::Clamp(Rage + Amount, 0.f, MaxRage);
    if (Rage >= MaxRage && !bIsBerserk)
        EnterBerserkState();
}

// 클라이언트에서도 RepNotify 처리
void ALCBossSlenderman::OnRep_IsBerserk()
{
    Super::OnRep_IsBerserk();

    if (bIsBerserk)
        StartBerserk();  // 클라이언트 연출
    else
        EndBerserk();    // 클라이언트 연출 해제
}

// 실제 광폭화 진입 직전(서버) + 로그
void ALCBossSlenderman::EnterBerserkState()
{
    Super::EnterBerserkState();
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Enter Berserk State"));

    // (1) 이동 속도 대폭 상승
    GetCharacterMovement()->MaxWalkSpeed *= BerserkSpeedMultiplier;

    // (2) 피해 면역 시간 처리: 예시로 물리 충돌 비활성화
    // GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

// 광폭화 시작 시 연출(이펙트/사운드) + 분신 소환
void ALCBossSlenderman::StartBerserk()
{
    Super::StartBerserk(); // bIsBerserk = true & 멀티캐스트

    // (3) 이펙트
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

    // (4) 사운드
    if (BerserkSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            BerserkSound,
            GetActorLocation()
        );
    }

    // (5) 그림자 분신 소환
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

// Duration 초간 지속되는 버전
void ALCBossSlenderman::StartBerserk(float Duration)
{
    // 연출 포함한 무한 버전 실행
    StartBerserk();

    // 일정 시간 뒤 EndBerserk 자동 호출
    GetWorldTimerManager().ClearTimer(BerserkTimerHandle);
    GetWorldTimerManager().SetTimer(
        BerserkTimerHandle,
        this, &ALCBossSlenderman::EndBerserk,
        Duration, false
    );
}

// 광폭화 종료: 버프 해제, 면역 해제, 분신 정리
void ALCBossSlenderman::EndBerserk()
{
    Super::EndBerserk();
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] End Berserk State"));

    // (1) 이동 속도 원복
    GetCharacterMovement()->MaxWalkSpeed /= BerserkSpeedMultiplier;

    // (2) 분신 제거
    for (AActor* Clone : ShadowClones)
    {
        if (IsValid(Clone))
        {
            Clone->Destroy();
        }
    }
    ShadowClones.Empty();
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