#include "Actor/TrainingRoom/TrainingDummy.h"
#include "Actor/TrainingRoom/TrainingRoomManager.h"
#include "Item/Component/DamageReceiverComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "LastCanary.h"

ATrainingDummy::ATrainingDummy()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    // 씬 루트
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // 메시 설정
    DummyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DummyMesh"));
    DummyMesh->SetupAttachment(RootComponent);
    DummyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DummyMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // 데미지 컴포넌트
    DamageReceiverComponent = CreateDefaultSubobject<UDamageReceiverComponent>(TEXT("DamageReceiverComponent"));
    DamageReceiverComponent->MaxHealth = 100.0f;
    DamageReceiverComponent->bDestroyOnHealthDepleted = false;
    DamageReceiverComponent->bAutoAddEnemyTag = false;
}

void ATrainingDummy::BeginPlay()
{
	Super::BeginPlay();
	
    // 초기 위치 저장
    InitialLocation = GetActorLocation();

    // 게임플레이 태그 초기화
    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Training.Dummy")));

    // 데미지 이벤트 바인딩
    if (DamageReceiverComponent)
    {
        DamageReceiverComponent->OnDamageReceived.AddDynamic(this, &ATrainingDummy::OnDamageReceived);
        DamageReceiverComponent->OnHealthDepleted.AddDynamic(this, &ATrainingDummy::OnHealthDepleted);
    }

    // 동적 머티리얼 생성
    if (DummyMesh && DummyMesh->GetNumMaterials() > 0)
    {
        DynamicMaterial = DummyMesh->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DummyColor);
        }
    }

    // 랜덤 이동 타입이면 타이머 시작
    if (DummyType == EDummyType::Random && HasAuthority())
    {
        SetNewRandomTarget();
    }
}

// Called every frame
void ATrainingDummy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsActive || !HasAuthority())
    {
        return;
    }

    // 이동 처리
    HandleMovement(DeltaTime);
}

void ATrainingDummy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATrainingDummy, bIsActive);
}

void ATrainingDummy::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer = GameplayTags;
}

void ATrainingDummy::SetActive(bool bNewActive)
{
    bIsActive = bNewActive;

    if (DummyMesh)
    {
        DummyMesh->SetVisibility(bIsActive);
        DummyMesh->SetCollisionEnabled(bIsActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    }
}

void ATrainingDummy::ResetDummy()
{
    if (!HasAuthority())
    {
        return;
    }

    // 체력 복구
    if (DamageReceiverComponent)
    {
        DamageReceiverComponent->SetHealth(DamageReceiverComponent->MaxHealth);
    }

    // 위치 초기화
    SetActorLocation(InitialLocation);

    // 활성화
    SetActive(true);

    // 색상 초기화
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DummyColor);
    }

    LOG_Item_WARNING(TEXT("[TrainingDummy] 더미 리셋 완료: %s"), *GetName());
}

void ATrainingDummy::SetTrainingManager(ATrainingRoomManager* Manager)
{
    TrainingManager = Manager;
}

void ATrainingDummy::UpdateInitialLocation(const FVector& NewLocation)
{
    InitialLocation = NewLocation;
    SetActorLocation(NewLocation);
}

void ATrainingDummy::OnDamageReceived(AActor* DamagedActor, float DamageAmount, const FHitResult& HitInfo, AActor* DamageCauser)
{
    if (!HasAuthority())
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[TrainingDummy] 데미지 받음: %.1f (남은 체력: %.1f/%.1f)"),
        DamageAmount,
        DamageReceiverComponent->GetCurrentHealth(),
        DamageReceiverComponent->GetMaxHealth());

    // 훈련 매니저에 히트 알림
    if (TrainingManager)
    {
        TrainingManager->OnDummyHit(this, DamageAmount, HitInfo);
    }

    // 피격 효과
    if (bShowHitReaction)
    {
        Multicast_ShowHitReaction(HitInfo.ImpactPoint);
    }
}

void ATrainingDummy::OnHealthDepleted(AActor* DamagedActor)
{
    if (!HasAuthority())
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[TrainingDummy] 체력 소진: %s"), *GetName());

    // 훈련 매니저에 파괴 알림
    if (TrainingManager)
    {
        TrainingManager->OnDummyDestroyed(this, DamageReceiverComponent->MaxHealth);
    }

    // 비활성화 (파괴하지 않고 재사용)
    SetActive(false);
}

void ATrainingDummy::HandleMovement(float DeltaTime)
{
    switch (DummyType)
    {
    case EDummyType::Patrol:
        HandlePatrolMovement(DeltaTime);
        break;

    case EDummyType::Random:
        HandleRandomMovement(DeltaTime);
        break;

    case EDummyType::Static:
    case EDummyType::Reactive:
    default:
        // 이동 없음
        break;
    }
}

void ATrainingDummy::HandlePatrolMovement(float DeltaTime)
{
    if (PatrolPoints.Num() == 0)
    {
        return;
    }

    FVector CurrentLocation = GetActorLocation();
    FVector TargetLocation = PatrolPoints[CurrentPatrolIndex];

    // 목표 지점으로 이동
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    FVector NewLocation = CurrentLocation + Direction * MovementSpeed * DeltaTime;

    SetActorLocation(NewLocation);

    // 목표 지점 도달 확인
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);
    if (DistanceToTarget < 50.0f)
    {
        // 다음 포인트로
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }
}

void ATrainingDummy::HandleRandomMovement(float DeltaTime)
{
    FVector CurrentLocation = GetActorLocation();

    // 목표 지점으로 이동
    FVector Direction = (RandomTargetLocation - CurrentLocation).GetSafeNormal();
    FVector NewLocation = CurrentLocation + Direction * MovementSpeed * DeltaTime;

    SetActorLocation(NewLocation);

    // 목표 지점 도달 확인
    float DistanceToTarget = FVector::Dist(CurrentLocation, RandomTargetLocation);
    if (DistanceToTarget < 100.0f)
    {
        // 새로운 목표 설정
        SetNewRandomTarget();
    }
}

void ATrainingDummy::SetNewRandomTarget()
{
    // 초기 위치 기준 랜덤 반경 내 목표 설정
    FVector RandomOffset = FVector(
        FMath::RandRange(-RandomMovementRadius, RandomMovementRadius),
        FMath::RandRange(-RandomMovementRadius, RandomMovementRadius),
        0.0f
    );

    RandomTargetLocation = InitialLocation + RandomOffset;
}

void ATrainingDummy::Multicast_ShowHitReaction_Implementation(FVector HitLocation)
{
    // 피격 색상 변화 효과
    if (DynamicMaterial)
    {
        // 빨간색으로 깜빡임
        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Red);

        // 0.2초 후 원래 색상으로
        FTimerHandle ColorResetTimer;
        GetWorld()->GetTimerManager().SetTimer(ColorResetTimer, [this]()
            {
                if (DynamicMaterial)
                {
                    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DummyColor);
                }
            }, 0.2f, false);
    }
}
