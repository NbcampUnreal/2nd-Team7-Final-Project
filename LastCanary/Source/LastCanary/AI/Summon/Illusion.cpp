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
    

    // → 추가: 데미지용 캡슐 콜리전
    DamageCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("DamageCapsule"));
    // 캡슐 크기 설정 (예시)
    RootComponent = DamageCapsule;
    DamageCapsule->InitCapsuleSize(50.f, 100.f);
    DamageCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DamageCapsule->SetCollisionObjectType(ECC_Pawn);     // Pawn 타입으로
    DamageCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
    // 프로젝타일 라인 트레이스가 기본적으로 Visibility 채널을 씁니다.
    DamageCapsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    // 혹은 발사체가 WorldDynamic 채널이라면
    DamageCapsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy")));


    SetCanBeDamaged(true);

}

void AIllusion::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer = GameplayTags;
}

void AIllusion::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;

    if (IllusionPostProcessMaterial && IllusionInterval > 0.f)
    {
        // 타이머 핸들에 바로 바인딩
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

    if (!IsValid(BossOwner))
        return;

    const FVector Current = GetActorLocation();
    const FVector BossLoc = BossOwner->GetActorLocation();

    if (FVector::DistSquared(MoveTarget, BossLoc) > FMath::Square(MoveRadius))
    {
        PickNewMoveTarget();
    }

    FVector Next = FMath::VInterpTo(Current, MoveTarget, DeltaTime, MoveInterpSpeed);
    FHitResult Hit;
    SetActorLocation(Next, true, &Hit);

    if (FVector::DistSquared(Next, MoveTarget) < FMath::Square(20.f))
    {
        PickNewMoveTarget();
    }
}

void AIllusion::PickNewMoveTarget()
{
    if (!IsValid(BossOwner)) return;

    UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!Nav) return;

    FNavLocation RandLoc;
    if (Nav->GetRandomPointInNavigableRadius(BossOwner->GetActorLocation(), MoveRadius, RandLoc))
    {
        RandLoc.Location.Z = BossOwner->GetActorLocation().Z;
        MoveTarget = RandLoc.Location;
    }
}

void AIllusion::ExecuteRandomPlayerIllusion()
{
    if (!IsValid(this))
        return;

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[Illusion] World == nullptr"));
        return;
    }

    // 1) SweepMultiByChannel
    TArray<FHitResult> Hits;
    FVector Loc = GetActorLocation();
    FCollisionShape Sphere = FCollisionShape::MakeSphere(IllusionRadius);

    World->SweepMultiByChannel(Hits, Loc, Loc, FQuat::Identity, ECC_Pawn, Sphere);
    if (Hits.Num() == 0)
        return;

    // 2) ValidPCs 필터
    TArray<APlayerController*> ValidPCs;
    for (const FHitResult& Hit : Hits)
    {
        AActor* Actor = Hit.GetActor();
        if (!Actor) continue;

        if (APawn* P = Cast<APawn>(Actor))
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
    if (ValidPCs.IsEmpty())
        return;

    // 3) 랜덤 선택
    int32 Idx = FMath::RandRange(0, ValidPCs.Num() - 1);
    APlayerController* ChosenPC = ValidPCs.IsValidIndex(Idx) ? ValidPCs[Idx] : nullptr;
    if (!ChosenPC) return;

    APawn* ChosenPawn = ChosenPC->GetPawn();
    if (!ChosenPawn) return;

    // 4) 카메라 컴포넌트
    UCameraComponent* Cam = ChosenPawn->FindComponentByClass<UCameraComponent>();
    if (!Cam)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Illusion] CameraComponent not found on %s"), *ChosenPawn->GetName());
        return;
    }

    // 중복 적용 방지
    auto& BlendArray = Cam->PostProcessSettings.WeightedBlendables.Array;
    bool bAlready = BlendArray.ContainsByPredicate(
        [this](const FWeightedBlendable& Elem)
        {
            return Elem.Object == IllusionPostProcessMaterial;
        });

    if (!bAlready)
    {
        Cam->PostProcessSettings.AddBlendable(IllusionPostProcessMaterial, IllusionBlendWeight);

        // 제거용 델리게이트로 안전하게 바인딩
        FTimerDelegate RemoveDel;
        RemoveDel.BindUFunction(this, FName("RemovePostProcess"), Cam);
        GetWorldTimerManager().SetTimer(
            RemovePPHandle,
            RemoveDel,
            IllusionDuration,
            false);
    }
}

void AIllusion::RemovePostProcess(UCameraComponent* Cam)
{
    if (IsValid(Cam) && IllusionPostProcessMaterial)
    {
        Cam->PostProcessSettings.RemoveBlendable(IllusionPostProcessMaterial);
    }
}

float AIllusion::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser
)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (ActualDamage <= 0.f)
        return 0.f;

    // HP 차감
    Health -= ActualDamage;
    UE_LOG(LogTemp, Warning, TEXT("[Illusion] Took %f damage, HP now %f"), ActualDamage, Health);

    if (Health <= 0.f)
    {
        DestroyIllusion();
    }

    return ActualDamage;
}

void AIllusion::DestroyIllusion()
{
    // 타이머 정리
    GetWorldTimerManager().ClearTimer(IllusionTimerHandle);
    GetWorldTimerManager().ClearTimer(RemovePPHandle);
    Destroy();
}