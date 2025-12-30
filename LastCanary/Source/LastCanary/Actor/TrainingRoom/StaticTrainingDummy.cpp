#include "Actor/TrainingRoom/StaticTrainingDummy.h"
#include "Item/Component/DamageReceiverComponent.h"
#include "Actor/TrainingRoom/TrainingTargetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "LastCanary.h"


AStaticTrainingDummy::AStaticTrainingDummy()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
    TargetMesh->SetupAttachment(RootComponent);
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TargetMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    DamageReceiverComponent = CreateDefaultSubobject<UDamageReceiverComponent>(TEXT("DamageReceiverComponent"));
    DamageReceiverComponent->MaxHealth = 50.0f;
    DamageReceiverComponent->bDestroyOnHealthDepleted = false;
    DamageReceiverComponent->bAutoAddEnemyTag = false;

    TrainingTargetComponent = CreateDefaultSubobject<UTrainingTargetComponent>(TEXT("TrainingTargetComponent"));

    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Training.Dummy.Static")));

    // 기본 마커 설정 (중앙 단일 마커)
    FTargetScoreMarker CenterMarker;
    CenterMarker.MarkerLabel = TEXT("Bullseye");
    CenterMarker.LocalPosition = FVector::ZeroVector;
    CenterMarker.MarkerColor = FLinearColor::Red;

    // 점수 구간 설정
    FScoreZoneMultiplier Zone1;
    Zone1.Radius = 5.0f;
    Zone1.DamageMultiplier = 5.0f;
    Zone1.ZoneColor = FLinearColor::Red;
    CenterMarker.ScoreZones.Add(Zone1);

    FScoreZoneMultiplier Zone2;
    Zone2.Radius = 15.0f;
    Zone2.DamageMultiplier = 2.0f;
    Zone2.ZoneColor = FLinearColor::Yellow;
    CenterMarker.ScoreZones.Add(Zone2);

    FScoreZoneMultiplier Zone3;
    Zone3.Radius = 30.0f;
    Zone3.DamageMultiplier = 1.0f;
    Zone3.ZoneColor = FLinearColor::Green;
    CenterMarker.ScoreZones.Add(Zone3);

    ScoreMarkers.Add(CenterMarker);
}


void AStaticTrainingDummy::BeginPlay()
{
	Super::BeginPlay();
	
    if (DamageReceiverComponent)
    {
        DamageReceiverComponent->OnDamageReceived.AddDynamic(this, &AStaticTrainingDummy::OnDamageReceived);
    }

    if (!GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Training.Dummy"))))
    {
        GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Training.Dummy")));
    }

    if (!GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy"))))
    {
        GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy")));
    }

    InitializeMarkers();
}


void AStaticTrainingDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (bShowDebugZones)
    {
        DrawDebugZones();
    }
}


void AStaticTrainingDummy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AStaticTrainingDummy::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer = GameplayTags;
}

void AStaticTrainingDummy::InitializeMarkers()
{
    // 각 마커의 점수 구간을 반지름 오름차순으로 정렬
    for (FTargetScoreMarker& Marker : ScoreMarkers)
    {
        Marker.ScoreZones.Sort([](const FScoreZoneMultiplier& A, const FScoreZoneMultiplier& B) {
            return A.Radius < B.Radius;
            });
    }

    LOG_Item_WARNING(TEXT("[StaticDummy] 마커 초기화 완료: %d개"), ScoreMarkers.Num());
}

FVector AStaticTrainingDummy::GetMarkerWorldPosition(const FTargetScoreMarker& Marker) const
{
    if (TargetMesh)
    {
        return TargetMesh->GetComponentTransform().TransformPosition(Marker.LocalPosition);
    }
    return GetActorLocation() + Marker.LocalPosition;
}

bool AStaticTrainingDummy::CalculateMarkerMultiplier(const FTargetScoreMarker& Marker, const FVector& HitLocation, float& OutMultiplier, FLinearColor& OutZoneColor) const
{
    if (!Marker.bEnabled || Marker.ScoreZones.Num() == 0)
    {
        return false;
    }

    FVector MarkerWorld = GetMarkerWorldPosition(Marker);
    float Distance = FVector::Dist(HitLocation, MarkerWorld);

    // 가장 작은 구간부터 체크
    for (const FScoreZoneMultiplier& Zone : Marker.ScoreZones)
    {
        if (Distance <= Zone.Radius)
        {
            OutMultiplier = Zone.DamageMultiplier;
            OutZoneColor = Zone.ZoneColor;
            return true;
        }
    }

    return false;
}

float AStaticTrainingDummy::CalculateFinalScore(const FVector& HitLocation, float BaseDamage, FString& OutMarkerLabel, FLinearColor& OutZoneColor)
{
    if (ScoreMarkers.Num() == 0)
    {
        OutMarkerLabel = TEXT("None");
        OutZoneColor = FLinearColor::White;
        return BaseDamage * DefaultDamageMultiplier;
    }

    // 히트 위치와 가장 가까운 마커 찾기
    const FTargetScoreMarker* BestMarker = nullptr;
    float BestMultiplier = 0.0f;
    FLinearColor BestColor = FLinearColor::White;
    float ClosestDistance = FLT_MAX;

    for (const FTargetScoreMarker& Marker : ScoreMarkers)
    {
        if (!Marker.bEnabled)
        {
            continue;
        }

        float TempMultiplier = 0.0f;
        FLinearColor TempColor;

        if (CalculateMarkerMultiplier(Marker, HitLocation, TempMultiplier, TempColor))
        {
            FVector MarkerWorld = GetMarkerWorldPosition(Marker);
            float Distance = FVector::Dist(HitLocation, MarkerWorld);

            // 더 가까운 마커 우선
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                BestMarker = &Marker;
                BestMultiplier = TempMultiplier;
                BestColor = TempColor;
            }
        }
    }

    // 유효한 마커를 찾았으면 해당 배율 적용
    if (BestMarker)
    {
        OutMarkerLabel = BestMarker->MarkerLabel;
        OutZoneColor = BestColor;
        return BaseDamage * BestMultiplier;
    }

    // 모든 마커 범위 밖
    if (bIgnoreOutOfRangeHits)
    {
        OutMarkerLabel = TEXT("Miss");
        OutZoneColor = FLinearColor::Gray;
        return 0.0f;
    }

    OutMarkerLabel = TEXT("Body");
    OutZoneColor = FLinearColor::White;
    return BaseDamage * DefaultDamageMultiplier;
}

float AStaticTrainingDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (DamageReceiverComponent)
    {
        ActualDamage = DamageReceiverComponent->HandleDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    }

    return ActualDamage;
}

void AStaticTrainingDummy::OnDamageReceived(AActor* DamagedActor, float DamageAmount, const FHitResult& HitInfo, AActor* DamageCauser)
{
    if (!HasAuthority())
    {
        return;
    }

    FString MarkerLabel;
    FLinearColor ZoneColor;
    float FinalScore = CalculateFinalScore(HitInfo.ImpactPoint, DamageAmount, MarkerLabel, ZoneColor);

    LOG_Item_WARNING(TEXT("[StaticDummy] 피격 | 마커: %s | 기본 데미지: %.1f | 최종 점수: %.1f | 위치: %s"),
        *MarkerLabel, DamageAmount, FinalScore, *HitInfo.ImpactPoint.ToString());

    // TrainingManager에 점수 전달
    if (TrainingTargetComponent)
    {
        // 점수 기록 로직
    }
}

void AStaticTrainingDummy::DrawDebugZones()
{
    if (!GetWorld())
    {
        return;
    }

    for (const FTargetScoreMarker& Marker : ScoreMarkers)
    {
        if (!Marker.bEnabled)
        {
            continue;
        }

        FVector MarkerWorld = GetMarkerWorldPosition(Marker);

        FVector Normal = TargetMesh ? TargetMesh->GetForwardVector() : GetActorForwardVector();
        FVector UpVector = FVector::UpVector;
        FVector RightVector = FVector::CrossProduct(UpVector, Normal).GetSafeNormal();
        UpVector = FVector::CrossProduct(Normal, RightVector).GetSafeNormal();

        for (const FScoreZoneMultiplier& Zone : Marker.ScoreZones)
        {
            DrawDebugCircle(
                GetWorld(),
                MarkerWorld,
                Zone.Radius,
                32,
                Zone.ZoneColor.ToFColor(true),
                false,
                -1.0f,
                0,
                2.0f,
                Normal,
                UpVector,
                false
            );

            FString MultiplierText = FString::Printf(TEXT("x%.1f"), Zone.DamageMultiplier);
            DrawDebugString(
                GetWorld(),
                MarkerWorld + (UpVector * (Zone.Radius + 5.0f)),
                MultiplierText,
                nullptr,
                Zone.ZoneColor.ToFColor(true),
                0.0f,
                true,
                1.0f
            );
        }

        float MaxRadius = Marker.ScoreZones.Num() > 0 ? Marker.ScoreZones.Last().Radius : 0.0f;
        DrawDebugString(
            GetWorld(),
            MarkerWorld + (UpVector * (MaxRadius + 15.0f)),
            Marker.MarkerLabel,
            nullptr,
            Marker.MarkerColor.ToFColor(true),
            0.0f,
            true,
            1.5f
        );

        DrawDebugSphere(
            GetWorld(),
            MarkerWorld,
            3.0f,
            8,
            Marker.MarkerColor.ToFColor(true),
            false,
            -1.0f,
            0,
            2.0f
        );
    }
}
