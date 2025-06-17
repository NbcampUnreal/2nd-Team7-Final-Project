#include "AI/Summon/Illusion.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

AIllusion::AIllusion()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    // 기본 충돌 프로필
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    // 월드 정적 지형은 블록
    MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    // Pawn(플레이어)하고는 겹침(혹은 원하는 대로)
    MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    // **동적 액터끼리(WorldDynamic) 서로 무시**
    MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);

    MeshComp->SetCastShadow(false);

    OnTakeAnyDamage.AddDynamic(this, &AIllusion::OnTakeAnyDamage_Handler);
}

void AIllusion::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
    // BeginPlay 에서는 목표를 뽑지 않습니다!
}

void AIllusion::SetBossOwner(AActor* Boss)
{
    BossOwner = Boss;
    // 보스 설정 직후에야 비로소 유효한 목표를 뽑을 수 있습니다.
    PickNewMoveTarget();
}

void AIllusion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!BossOwner)
        return;

    const FVector Current = GetActorLocation();
    const FVector BossLoc = BossOwner->GetActorLocation();

    // 1) 목표가 보스 범위를 벗어났으면 즉시 재선정
    if (FVector::DistSquared(MoveTarget, BossLoc) > FMath::Square(MoveRadius))
    {
        PickNewMoveTarget();
    }

    // 3) 목표로 부드럽게 보간
    FVector Next = FMath::VInterpTo(Current, MoveTarget, DeltaTime, MoveInterpSpeed);

    // 4) Sweep 활성화 (지형·벽만 차단)
    FHitResult Hit;
    SetActorLocation(Next, true, &Hit);

    // 5) 목표 도달 여부 재확인 후 재선정
    if (FVector::DistSquared(Next, MoveTarget) < FMath::Square(20.f))
        PickNewMoveTarget();
}

void AIllusion::PickNewMoveTarget()
{
    if (!BossOwner) return;
    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!Nav) return;

    FNavLocation RandLoc;
    if (Nav->GetRandomPointInNavigableRadius(
        BossOwner->GetActorLocation(),
        MoveRadius,
        RandLoc))
    {
        RandLoc.Location.Z = BossOwner->GetActorLocation().Z; // 동일 높이 유지
        MoveTarget = RandLoc.Location;

        // 디버그 시각화 (2초)
        DrawDebugSphere(GetWorld(), MoveTarget, 25.f, 8, FColor::Green, false, 2.f);
    }
}

void AIllusion::OnTakeAnyDamage_Handler(
    AActor*,
    float Damage,
    const UDamageType*,
    AController*,
    AActor*)
{
    Health -= Damage;
    if (Health <= 0.f)
        DestroyIllusion();
}

void AIllusion::DestroyIllusion()
{
    Destroy();
}