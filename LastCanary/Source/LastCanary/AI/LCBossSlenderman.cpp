#include "AI/LCBossSlenderman.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"  // FHitResult 정의

ALCBossSlenderman::ALCBossSlenderman()
{
    // PawnSensingComponent 세팅
    PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
    PawnSensingComp->bHearNoises = true;
}

void ALCBossSlenderman::BeginPlay()
{
    Super::BeginPlay();

    // 1) Echo Ping
    GetWorldTimerManager().SetTimer(
        PingTimerHandle, this, &ALCBossSlenderman::EchoPing,
        PingInterval, true);

    // 2) Noise Heard 바인딩
    if (PawnSensingComp)
        PawnSensingComp->OnHearNoise.AddDynamic(this, &ALCBossSlenderman::OnNoiseHeard);

    // 3) Teleport
    GetWorldTimerManager().SetTimer(
        TeleportTimerHandle, this, &ALCBossSlenderman::TeleportToRandomLocation,
        TeleportInterval, true);

    // 4) Distortion
    GetWorldTimerManager().SetTimer(
        DistortionTimerHandle, this, &ALCBossSlenderman::ExecuteDistortion,
        DistortionInterval, true);

    // 5) Endless Stalk 발동 예약 (60초)
    GetWorldTimerManager().SetTimer(
        EndlessStalkTimerHandle, this, &ALCBossSlenderman::EnterEndlessStalk,
        60.f, false);
}

void ALCBossSlenderman::EchoPing()
{
    if (!HasAuthority()) return;

    // SweepMultiByChannel 을 사용해 반경 내 Pawn 탐지
    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(PingRadius);

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        GetActorLocation(),        // 시작
        GetActorLocation(),        // 끝 (같은 지점)
        FQuat::Identity,
        ECC_Pawn,                  // Pawn 채널
        Sphere
    );

    if (bHit)
    {
        for (auto& Hit : HitResults)
        {
            if (APawn* P = Cast<APawn>(Hit.GetActor()))
            {
                if (P->IsPlayerControlled())
                {
                    if (UMeshComponent* PawnMesh = P->FindComponentByClass<UMeshComponent>())
                    {
                        PawnMesh->SetRenderCustomDepth(true);
                    }
                }
            }
        }
    }
}

void ALCBossSlenderman::OnNoiseHeard(APawn* InstigatorPawn, const FVector& Location, float Volume)
{
    // 소리 난 방향 바라보기
    FVector Dir = (Location - GetActorLocation()).GetSafeNormal();
    FRotator NewRot = Dir.Rotation();
    SetActorRotation(NewRot);

    // 울부짖기 몽타주 재생
    if (ScreamMontage && GetMesh()->GetAnimInstance())
        GetMesh()->GetAnimInstance()->Montage_Play(ScreamMontage);
}

void ALCBossSlenderman::TeleportToRandomLocation()
{
    if (!HasAuthority()) return;

    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!Nav) return;

    FNavLocation RandomPoint;
    if (Nav->GetRandomPointInNavigableRadius(GetActorLocation(), 1000.f, RandomPoint))
    {
        SetActorLocation(RandomPoint.Location);
    }
}

void ALCBossSlenderman::ExecuteDistortion()
{
    if (!HasAuthority()) return;
    Multicast_DistortionEffect();
}

void ALCBossSlenderman::Multicast_DistortionEffect_Implementation()
{
    // Blueprint나 머티리얼로 맵 왜곡 효과 처리
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Distortion Effect Triggered"));
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
    {
        // 클라이언트 측 로컬 효과
        GetCharacterMovement()->MaxWalkSpeed *= 2.f;
        // 면역 시간 증가 등 추가 처리
    }
}

void ALCBossSlenderman::Multicast_StartEndlessStalk_Implementation()
{
    OnRep_EndlessStalk();
    UE_LOG(LogTemp, Warning, TEXT("[Slenderman] Endless Stalk Activated"));
}

void ALCBossSlenderman::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALCBossSlenderman, bIsEndlessStalk);
}
