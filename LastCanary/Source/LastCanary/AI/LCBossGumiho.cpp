#include "AI/LCBossGumiho.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/DamageType.h"

ALCBossGumiho::ALCBossGumiho()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ALCBossGumiho::BeginPlay()
{
    Super::BeginPlay();

    // 타이머 세팅
    GetWorldTimerManager().SetTimer(IllusionTimerHandle, this, &ALCBossGumiho::SpawnIllusions,
        IllusionInterval, true);
    GetWorldTimerManager().SetTimer(FoxfireTimerHandle, this, &ALCBossGumiho::ExecuteFoxfireVolley,
        FoxfireInterval, true);
    GetWorldTimerManager().SetTimer(SwapTimerHandle, this, &ALCBossGumiho::PerformIllusionSwap,
        IllusionSwapInterval, true);
    GetWorldTimerManager().SetTimer(CharmTimerHandle, this, &ALCBossGumiho::ExecuteCharmGaze,
        CharmInterval, true);
}

void ALCBossGumiho::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void ALCBossGumiho::SpawnIllusions()
{
    if (!HasAuthority() || !IllusionClass) return;

    // 이전 환영 제거
    for (AActor* I : IllusionActors) if (I) I->Destroy();
    IllusionActors.Empty();

    // 새 환영 생성
    for (int32 i = 0; i < NumIllusions; ++i)
    {
        FVector Offset = FMath::RandPointInBox(FBox(FVector(-600, -600, 0), FVector(600, 600, 0)));
        FVector Loc = GetActorLocation() + Offset;
        FRotator Rot = GetActorRotation();
        AActor* Ill = GetWorld()->SpawnActor<AActor>(IllusionClass, Loc, Rot);
        if (Ill) IllusionActors.Add(Ill);
    }
}

void ALCBossGumiho::ExecuteFoxfireVolley()
{
    if (!HasAuthority() || !FoxfireProjectileClass) return;

    for (int32 i = 0; i < FoxfireCount; ++i)
    {
        FVector Dir = FMath::VRand();
        Dir.Z = FMath::Abs(Dir.Z);
        FVector SpawnLoc = GetActorLocation() + Dir * 200.f + FVector(0, 0, 100);
        FRotator SpawnRot = Dir.Rotation();
        GetWorld()->SpawnActor<AActor>(FoxfireProjectileClass, SpawnLoc, SpawnRot);
    }
}

void ALCBossGumiho::PerformIllusionSwap()
{
    if (!HasAuthority() || IllusionActors.Num() == 0) return;

    int32 Idx = FMath::RandRange(0, IllusionActors.Num() - 1);
    AActor* Ill = IllusionActors[Idx];
    if (!Ill) return;

    const FVector BossLoc = GetActorLocation();
    const FVector IllLoc = Ill->GetActorLocation();

    SetActorLocation(IllLoc);
    Ill->SetActorLocation(BossLoc);
}

void ALCBossGumiho::ExecuteCharmGaze()
{
    if (!HasAuthority()) return;

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(CharmRadius);
    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits,
        GetActorLocation(),
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        Sphere
	);

    if (!bHit) return;

    for (auto& Hit : Hits)
    {
        if (APawn* P = Cast<APawn>(Hit.GetActor()))
        {
            if (P->IsPlayerControlled())
            {
                P->Tags.AddUnique(FName("Charmed"));
                UE_LOG(LogTemp, Log, TEXT("[Gumiho] %s is Charmed"), *P->GetName());
            }
        }
    }
}

void ALCBossGumiho::ExecuteNineTailBurst()
{
    // 광역 대미지 (범위는 AttackRange 사용)
    DealDamageInRange(NormalAttackDamage * 2.f);
    UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Nine-Tail Burst executed"));
}


void ALCBossGumiho::OnRep_DivineGrace()
{
    if (bIsDivineGrace)
    {
        // 클라이언트 로컬: 면역·버프 적용
        GetCharacterMovement()->MaxWalkSpeed *= 1.5f;
        UE_LOG(LogTemp, Warning, TEXT("[Gumiho] Divine Grace activated"));
    }
}

void ALCBossGumiho::Multicast_StartDivineGrace_Implementation()
{
    OnRep_DivineGrace();
    // 추가: 화이트 윈드 이펙트, 현악기 사운드 재생 가능
}

void ALCBossGumiho::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALCBossGumiho, bIsDivineGrace);
}
