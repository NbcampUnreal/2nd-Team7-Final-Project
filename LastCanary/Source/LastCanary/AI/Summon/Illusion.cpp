#include "AI/Summon/Illusion.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"

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

    // 주기적으로 랜덤 플레이어에게 Illusion 효과 걸기 시작
    if (IllusionPostProcessMaterial && IllusionInterval > 0.f)
    {
        GetWorldTimerManager().SetTimer(
            IllusionTimerHandle,
            this, &AIllusion::ExecuteRandomPlayerIllusion,
            IllusionInterval,
            true);
    }
}

void AIllusion::SetBossOwner(AActor* Boss)
{
    BossOwner = Boss;
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

void AIllusion::ExecuteRandomPlayerIllusion()
{
    // 1) 반경 내 Pawn Sweep (시작과 끝 위치 동일 → 단순 범위 검사)
    TArray<FHitResult> Hits;
    FVector Loc = GetActorLocation();
    FCollisionShape Sphere = FCollisionShape::MakeSphere(IllusionRadius);

    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits,
        Loc, Loc,
        FQuat::Identity,
        ECC_Pawn,
        Sphere
    );
    if (!bHit) return;

    // 2) 유효한 로컬 플레이어 컨트롤러만 필터
    TArray<APlayerController*> ValidPCs;
    for (const FHitResult& Hit : Hits)
    {
        if (APawn* P = Cast<APawn>(Hit.GetActor()))
        {
            if (AController* C = P->GetController())
            {
                if (C->IsLocalController())
                {
                    ValidPCs.Add(Cast<APlayerController>(C));
                }
            }
        }
    }
    if (ValidPCs.Num() == 0) return;

    // 3) 랜덤으로 한 명 선택
    int32 Idx = FMath::RandRange(0, ValidPCs.Num() - 1);
    APlayerController* ChosenPC = ValidPCs[Idx];
    APawn* ChosenPawn = ChosenPC->GetPawn();
    if (!ChosenPawn) return;

    // 4) 선택된 플레이어 카메라에 포스트프로세스 적용
    if (UCameraComponent* Cam = ChosenPawn->FindComponentByClass<UCameraComponent>())
    {
        // 중복 적용 방지: 이미 같은 머티리얼이 있으면 스킵
        auto& BlendArray = Cam->PostProcessSettings.WeightedBlendables.Array;
        bool bAlready = BlendArray.ContainsByPredicate(
            [this](const FWeightedBlendable& Elem)
            {
                return Elem.Object == IllusionPostProcessMaterial;
            });

        if (!bAlready)
        {
            // a) 추가
            Cam->PostProcessSettings.AddBlendable(
                IllusionPostProcessMaterial,
                IllusionBlendWeight
            );

            // b) 제거 예약
            FTimerHandle TmpHandle;
            GetWorldTimerManager().SetTimer(
                TmpHandle,
                FTimerDelegate::CreateLambda([Cam, this]()
                    {
                        Cam->PostProcessSettings.RemoveBlendable(IllusionPostProcessMaterial);
                    }),
                IllusionDuration,
                false
            );
        }
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