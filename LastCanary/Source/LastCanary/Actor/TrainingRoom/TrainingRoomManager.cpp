#include "Actor/TrainingRoom/TrainingRoomManager.h"
#include "Actor/TrainingRoom/SkeletalTrainingDummy.h"
#include "Actor/TrainingRoom/StaticTrainingDummy.h"
#include "Actor/TrainingRoom/TrainingTargetComponent.h"
#include "Item/Component/WeaponStatsComponent.h"
#include "Character/BaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "LastCanary.h"

ATrainingRoomManager::ATrainingRoomManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ATrainingRoomManager::BeginPlay()
{
    Super::BeginPlay();

    // 초기 세션 실행 시 값을 입력 받지 않은 상태에서 스폰포인트를 생성해버림
    //// 스폰 포인트가 없으면 자동 생성
    //if (DummySpawnPoints.Num() == 0)
    //{
    //    GenerateSpawnPoints();
    //}

    // 랜덤 스트림 초기화 (현재 시간 기반 시드)
    RandomStream.Initialize(FPlatformTime::Cycles());

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] 훈련장 매니저 초기화 완료"));
}

void ATrainingRoomManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority() || !bSessionActive)
    {
        return;
    }

#if WITH_EDITOR
    DrawDebugGrid(); // 조건 없이 호출
#endif

    // 세션 시간 체크
    CheckSessionTime();
}

void ATrainingRoomManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATrainingRoomManager, bSessionActive);
    DOREPLIFETIME(ATrainingRoomManager, SessionStartTime);
    DOREPLIFETIME(ATrainingRoomManager, CurrentTrainingMode);
    DOREPLIFETIME(ATrainingRoomManager, DefaultDummyType);
    DOREPLIFETIME(ATrainingRoomManager, DummyMovementSpeed);
    DOREPLIFETIME(ATrainingRoomManager, DummyRandomRadius);
    DOREPLIFETIME_CONDITION_NOTIFY(ATrainingRoomManager, CurrentStats, COND_None, REPNOTIFY_Always);
}

void ATrainingRoomManager::OnRep_SessionActive()
{
    OnSessionStateChanged.Broadcast(bSessionActive);

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] 세션 상태 변경: %s"), bSessionActive ? TEXT("시작") : TEXT("종료"));
}

void ATrainingRoomManager::OnRep_Stats()
{
    OnStatsUpdated.Broadcast(CurrentStats);
}

void ATrainingRoomManager::StartTrainingSession(ABaseCharacter* TrainingPlayer)
{
    if (!HasAuthority())
    {
        return;
    }

    if (bSessionActive)
    {
        LOG_Item_WARNING(TEXT("[TrainingRoomManager] 이미 훈련 세션이 진행 중"));
        return;
    }

    if (!TrainingPlayer)
    {
        LOG_Item_WARNING(TEXT("[TrainingRoomManager] 유효하지 않은 플레이어"));
        return;
    }

    CurrentTrainingPlayer = TrainingPlayer;
    PlayerWeaponStats = TrainingPlayer->FindComponentByClass<UWeaponStatsComponent>();

    if (!PlayerWeaponStats)
    {
        LOG_Item_WARNING(TEXT("[TrainingRoomManager] 플레이어에게 WeaponStatsComponent가 없음"));
        return;
    }

    SubscribeToWeaponStats(PlayerWeaponStats);

    // 통계 초기화
    CurrentStats.Reset();
    PlayerWeaponStats->ResetStats();

    // 세션 시작
    bSessionActive = true;
    SessionStartTime = GetWorld()->GetTimeSeconds();

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] 훈련 시작 - 플레이어: %s"),
        *TrainingPlayer->GetName());

    // 더미 리셋
    ResetAllDummies();

    // 그냥 매번 새로운 스폰 포인트 생성
    GenerateSpawnPoints();

    // 더미가 있을 시 재활용
    if (ActiveTargets.Num() > 0)
    {
        // 스폰 패턴이 그리드 기반인 경우 매번 새로운 순서로 생성
        if (SpawnPattern == ESpawnPattern::GridRandom ||
            SpawnPattern == ESpawnPattern::GridByDistance)
        {
            // 스폰 포인트를 매번 새로 생성하여 랜덤성 보장
            GenerateSpawnPoints();
            int32 ActualSpawnCount = FMath::Min(CurrentDummyCount, DummySpawnPoints.Num());
            SpawnDummies(ActualSpawnCount);
        }

        // 기존 더미 재활용
        if (ActiveTargets.Num() < CurrentDummyCount)
        {
            int32 AdditionalCount = CurrentDummyCount - ActiveTargets.Num();
            SpawnDummies(AdditionalCount);
        }

        for (int32 i = 0; i < ActiveTargets.Num(); i++)
        {
            if (IsValid(ActiveTargets[i]))
            {
                UTrainingTargetComponent* TargetComp = GetTargetComponent(ActiveTargets[i]);

                if (i < CurrentDummyCount)
                {
                    if (i < DummySpawnPoints.Num() && TargetComp)
                    {
                        // 이 부분 수정: UpdateInitialLocation 사용
                        TargetComp->UpdateInitialLocation(DummySpawnPoints[i].GetLocation());
                    }
                    if (TargetComp)
                    {
                        TargetComp->ResetTarget();
                    }
                }
                else
                {
                    if (TargetComp)
                    {
                        TargetComp->SetTargetEnabled(false);
                    }
                }
            }
        }
    }
    else
    {
        GenerateSpawnPoints(); // 최초 실행 시 생성
        SpawnDummies(CurrentDummyCount);
    }

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] 훈련 세션 시작 - 모드: %d, 시간: %.1f초, 타겟 타입: %s"), static_cast<int32>(CurrentTrainingMode), SessionDuration, TargetMeshType == ETargetMeshType::Skeletal ? TEXT("스켈레탈") : TEXT("스태틱"));

    OnSessionStateChanged.Broadcast(true);
}

void ATrainingRoomManager::ServerStartTraining_Implementation(ABaseCharacter* TrainingPlayer)
{
    StartTrainingSession(TrainingPlayer);
}

void ATrainingRoomManager::EndTrainingSession()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!bSessionActive)
    {
        return;
    }

    bSessionActive = false;

    if (PlayerWeaponStats)
    {
        FWeaponUsageStats WeaponStats = PlayerWeaponStats->TotalStats;

        CurrentStats.TotalShots = WeaponStats.TotalShots;
        CurrentStats.TotalHits = WeaponStats.ShotsHit;
        CurrentStats.TotalDamageDealt = WeaponStats.TotalDamageDealt;

        UnsubscribeFromWeaponStats(PlayerWeaponStats);
    }
    else
    {
        LOG_Item_WARNING(TEXT("[EndTrainingSession] ✗ PlayerWeaponStats가 NULL - 통계 수집 실패"));
    }

    // 최종 통계 업데이트
    CurrentStats.SessionTime = GetWorld()->GetTimeSeconds() - SessionStartTime;
    CurrentStats.CalculateAccuracy();
    CurrentStats.CalculateDPS();

    // 모든 더미 숨기기
    for (AActor* Target : ActiveTargets)
    {
        if (IsValid(Target))
        {
            UTrainingTargetComponent* TargetComp = GetTargetComponent(Target);
            if (TargetComp)
            {
                TargetComp->SetTargetEnabled(false);
            }
        }
    }

    // 최종 결과 출력
    LOG_Item_WARNING(TEXT("[TrainingRoomManager] ========== 훈련 세션 결과 =========="));
    LOG_Item_WARNING(TEXT("  - 총 발사: %d발"), CurrentStats.TotalShots);
    LOG_Item_WARNING(TEXT("  - 총 명중: %d발"), CurrentStats.TotalHits);
    LOG_Item_WARNING(TEXT("  - 명중률: %.1f%%"), CurrentStats.AccuracyPercentage);
    LOG_Item_WARNING(TEXT("  - 헤드샷: %d회"), CurrentStats.HeadshotCount);
    LOG_Item_WARNING(TEXT("  - 파괴한 더미: %d개"), CurrentStats.DummiesDestroyed);
    LOG_Item_WARNING(TEXT("  - 총 데미지: %.1f"), CurrentStats.TotalDamageDealt);
    LOG_Item_WARNING(TEXT("  - 평균 DPS: %.1f"), CurrentStats.AverageDPS);
    LOG_Item_WARNING(TEXT("  - 훈련 시간: %.1f초"), CurrentStats.SessionTime);
    LOG_Item_WARNING(TEXT("================================================"));

    OnSessionStateChanged.Broadcast(false);
    OnSessionComplete.Broadcast();
    UpdateStats();

    CurrentTrainingPlayer = nullptr;
    PlayerWeaponStats = nullptr;
}

void ATrainingRoomManager::SpawnDummies(int32 Count)
{
    if (!HasAuthority())
    {
        return;
    }

    TSubclassOf<AActor> SelectedClass = nullptr;

    if (TargetMeshType == ETargetMeshType::Skeletal)
    {
        if (!SkeletalDummyClass)
        {
            LOG_Item_WARNING(TEXT("[TrainingRoomManager] SkeletalDummyClass가 설정되지 않음"));
            return;
        }
        SelectedClass = SkeletalDummyClass;
    }
    else
    {
        if (!StaticDummyClass)
        {
            LOG_Item_WARNING(TEXT("[TrainingRoomManager] StaticDummyClass가 설정되지 않음"));
            return;
        }
        SelectedClass = StaticDummyClass;
    }

    LOG_Item_WARNING(TEXT("[SpawnDummies] 시작 - 요청 개수: %d, 현재 ActiveTargets: %d, DummySpawnPoints: %d"), Count, ActiveTargets.Num(), DummySpawnPoints.Num());

    if (DummySpawnPoints.Num() == 0)
    {
        LOG_Item_WARNING(TEXT("[TrainingRoomManager] 스폰 포인트가 없음"));
        return;
    }

    int32 StartIndex = ActiveTargets.Num();
    int32 SpawnCount = FMath::Min(Count, MaxDummies - ActiveTargets.Num());
    SpawnCount = FMath::Min(SpawnCount, DummySpawnPoints.Num());

    LOG_Item_WARNING(TEXT("[SpawnDummies] 실제 스폰 개수: %d"), SpawnCount);

    for (int32 i = 0; i < SpawnCount; i++)
    {
        int32 SpawnIndex = (StartIndex + i) % DummySpawnPoints.Num();
        FTransform SpawnTransform = DummySpawnPoints[SpawnIndex];
        FVector SpawnLocation = SpawnTransform.GetLocation();

        // 중복 체크를 위한 좌표 출력
        LOG_Item_WARNING(TEXT("[SpawnDummies] [%d] SpawnIndex=%d, Location=(%.1f, %.1f, %.1f)"),
            i, SpawnIndex, SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* NewTarget = GetWorld()->SpawnActor<AActor>(SelectedClass, SpawnTransform, SpawnParams);

        if (NewTarget)
        {
            UTrainingTargetComponent* TargetComp = GetTargetComponent(NewTarget);
            if (TargetComp)
            {
                TargetComp->SetTrainingManager(this);
                TargetComp->DummyType = DefaultDummyType;
            }

            ActiveTargets.Add(NewTarget);

            LOG_Item_WARNING(TEXT("[SpawnDummies] 타겟 스폰 성공: %s at (%.1f, %.1f, %.1f)"), *NewTarget->GetName(), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
        }
    }

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] %d개 타겟 스폰 완료 (총 %d개)"), SpawnCount, ActiveTargets.Num());
}

void ATrainingRoomManager::ClearAllDummies()
{
    if (!HasAuthority())
    {
        return;
    }

    for (AActor* Target : ActiveTargets)
    {
        if (IsValid(Target))
        {
            Target->Destroy();
        }
    }

    ActiveTargets.Empty();
    LOG_Item_WARNING(TEXT("[TrainingRoomManager] 모든 더미 제거 완료"));
}

void ATrainingRoomManager::ResetAllDummies()
{
    if (!HasAuthority())
    {
        return;
    }

    for (AActor* Target : ActiveTargets)
    {
        if (IsValid(Target))
        {
            UTrainingTargetComponent* TargetComp = GetTargetComponent(Target);
            if (TargetComp)
            {
                TargetComp->ResetTarget();
            }
        }
    }

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] 모든 더미 리셋 완료"));
}

void ATrainingRoomManager::OnDummyDestroyed(AActor* DestroyedDummy, float DamageDealt)
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentStats.DummiesDestroyed++;

    UpdateStats();

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] 더미 파괴: %s (총 %d개 파괴)"),
        *DestroyedDummy->GetName(), CurrentStats.DummiesDestroyed);

    // 서바이벌 모드면 새 더미 스폰
    if (bSessionActive && CurrentTrainingMode == ETrainingMode::Survival)
    {
        SpawnDummies(1);
    }
}

void ATrainingRoomManager::OnDummyHit(AActor* HitDummy, float DamageAmount, const FHitResult& HitInfo)
{
    if (!HasAuthority())
    {
        return;
    }

    // 헤드샷 판정 (히트 본 이름으로 확인)
    if (HitInfo.BoneName.ToString().Contains(TEXT("Head")))
    {
        CurrentStats.HeadshotCount++;
        LOG_Item_WARNING(TEXT("[TrainingRoomManager] 헤드샷! (총 %d회)"), CurrentStats.HeadshotCount);
    }

    UpdateStats();
}

void ATrainingRoomManager::OnWeaponFired(bool bHit)
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentStats.TotalShots++;

    if (bHit)
    {
        CurrentStats.TotalHits++;
    }

    UpdateStats();
}

void ATrainingRoomManager::UpdateStats()
{
    // 명중률 계산
    CurrentStats.CalculateAccuracy();

    // 세션 시간 업데이트
    if (bSessionActive)
    {
        CurrentStats.SessionTime = GetWorld()->GetTimeSeconds() - SessionStartTime;
        CurrentStats.CalculateDPS();
    }

    // 델리게이트 브로드캐스트
    OnStatsUpdated.Broadcast(CurrentStats);
}

void ATrainingRoomManager::CheckSessionTime()
{
    if (SessionDuration <= 0.0f)
    {
        return; // 무제한 모드
    }

    float ElapsedTime = GetWorld()->GetTimeSeconds() - SessionStartTime;

    if (ElapsedTime >= SessionDuration)
    {
        EndTrainingSession();
    }
}

void ATrainingRoomManager::DrawDebugGrid()
{
    FVector BaseLocation = GetActorLocation() + SpawnAreaCenter;

    // 전체 스폰 영역 상자 그리기
    FVector BoxExtent(SpawnAreaSize.X * 0.5f, SpawnAreaSize.Y * 0.5f, 10.0f);
    DrawDebugBox(GetWorld(), BaseLocation, BoxExtent, FColor::Yellow, false, -1.0f, 0, 5.0f);

    if (SpawnPattern == ESpawnPattern::GridRandom || SpawnPattern == ESpawnPattern::GridByDistance)
    {
        // 그리드 행, 열 간격 계산
        float CellWidth = SpawnAreaSize.X / GridColumns;
        float CellHeight = SpawnAreaSize.Y / GridRows;

        for (int32 Row = 0; Row < GridRows; Row++)
        {
            for (int32 Col = 0; Col < GridColumns; Col++)
            {
                FVector CellCenter = BaseLocation
                    - FVector(SpawnAreaSize.X * 0.5f, SpawnAreaSize.Y * 0.5f, 0.0f)
                    + FVector(CellWidth * (Col + 0.5f), CellHeight * (Row + 0.5f), 0.0f);

                // 각 셀 위치에 작은 파란색 박스 그리기
                DrawDebugBox(GetWorld(), CellCenter, FVector(CellWidth * 0.4f, CellHeight * 0.4f, 10.0f), FColor::Blue, false, -1.0f, 0, 2.5f);
            }
        }
    }
}

UTrainingTargetComponent* ATrainingRoomManager::GetTargetComponent(AActor* Target) const
{
    if (!Target)
    {
        return nullptr;
    }

    return Target->FindComponentByClass<UTrainingTargetComponent>();
}

void ATrainingRoomManager::GenerateSpawnPoints()
{
    DummySpawnPoints.Empty();

    int32 NewSeed = FPlatformTime::Cycles();
    RandomStream.Initialize(NewSeed);

    LOG_Item_WARNING(TEXT("[GenerateSpawnPoints] RandomStream 재초기화 - Seed: %d, SpawnPattern: %d"),
        NewSeed, static_cast<int32>(SpawnPattern));

    switch (SpawnPattern)
    {
    case ESpawnPattern::FreeRandom:
        GenerateFreeRandomPattern();
        break;

    case ESpawnPattern::GridRandom:
        GenerateGridRandomPattern();
        break;

    case ESpawnPattern::GridByDistance:
        GenerateGridByDistancePattern();
        break;
    }

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] %d개의 스폰 포인트 생성 완료 (패턴: %d)"),
        DummySpawnPoints.Num(), static_cast<int32>(SpawnPattern));
}

void ATrainingRoomManager::GenerateFreeRandomPattern()
{
    FVector BaseLocation = GetActorLocation() + SpawnAreaCenter;

    for (int32 i = 0; i < MaxDummies; i++)
    {
        // 직사각형 영역 내 랜덤 위치
        float RandomX = RandomStream.FRandRange(-SpawnAreaSize.X / 2.0f, SpawnAreaSize.X / 2.0f);
        float RandomY = RandomStream.RandRange(-SpawnAreaSize.Y / 2.0f, SpawnAreaSize.Y / 2.0f);

        FVector SpawnLocation = BaseLocation + FVector(RandomX, RandomY, 0.0f);

        FTransform SpawnTransform;
        SpawnTransform.SetLocation(SpawnLocation);
        SpawnTransform.SetRotation(FQuat::Identity);

        DummySpawnPoints.Add(SpawnTransform);
    }
}

void ATrainingRoomManager::GenerateGridRandomPattern()
{
    // 그리드 기반 위치들을 생성한 뒤 랜덤하게 선택
    FVector BaseLocation = GetActorLocation() + SpawnAreaCenter;

    // 그리드 간격 계산
    float CellWidth = SpawnAreaSize.X / GridColumns;
    float CellHeight = SpawnAreaSize.Y / GridRows;

    // 모든 그리드 셀 생성
    TArray<FVector> AllGridPositions;

    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Col = 0; Col < GridColumns; Col++)
        {
            // 그리드 셀의 중심 위치 계산
            float X = -SpawnAreaSize.X / 2.0f + CellWidth * (Col + 0.5f);
            float Y = -SpawnAreaSize.Y / 2.0f + CellHeight * (Row + 0.5f);

            FVector GridPosition = BaseLocation + FVector(X, Y, 0.0f);
            AllGridPositions.Add(GridPosition);
        }
    }

    // 셔플하여 랜덤 순서로 만들기
    for (int32 i = AllGridPositions.Num() - 1; i > 0; i--)
    {
        int32 j = RandomStream.RandRange(0, i);
        AllGridPositions.Swap(i, j);
    }

    // MaxDummies 개수만큼만 사용
    int32 PointCount = FMath::Min(AllGridPositions.Num(), MaxDummies);
    for (int32 i = 0; i < PointCount; i++)
    {
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(AllGridPositions[i]);
        SpawnTransform.SetRotation(FQuat::Identity);

        DummySpawnPoints.Add(SpawnTransform);
    }
}

void ATrainingRoomManager::GenerateGridByDistancePattern()
{
    // 특정 거리(행)에서 랜덤 열 위치에 생성(행, 열 헷갈림...)
    FVector BaseLocation = GetActorLocation() + SpawnAreaCenter;

    // 열 간격 계산
    float ColumnWidth = SpawnAreaSize.X / GridColumns;

    // 사용 가능한 모든 열 인덱스 생성
    TArray<int32> AvailableColumns;
    for (int32 Col = 0; Col < GridColumns; Col++)
    {
        AvailableColumns.Add(Col);
    }

    // 셔플하여 랜덤 순서로 만들기
    for (int32 i = AvailableColumns.Num() - 1; i > 0; i--)
    {
        int32 j = RandomStream.RandRange(0, i);
        AvailableColumns.Swap(i, j);
    }

    // TargetDistance를 Y 좌표로 사용 (특정 행 거리)
    float Y = -SpawnAreaSize.Y / 2.0f + TargetDistance;

    // GridColumns와 MaxDummies 중 작은 값만큼만 생성 (중복 방지)
    int32 SpawnCount = FMath::Min(GridColumns, MaxDummies);

    LOG_Item_WARNING(TEXT("[GenerateGridByDistance] 시작 - GridColumns: %d, MaxDummies: %d, SpawnCount: %d"),
        GridColumns, MaxDummies, SpawnCount);

    for (int32 i = 0; i < SpawnCount; i++)
    {
        // 셔플된 순서대로 열 선택
        int32 Col = AvailableColumns[i];
        float X = -SpawnAreaSize.X / 2.0f + ColumnWidth * (Col + 0.5f);

        FVector SpawnLocation = BaseLocation + FVector(X, Y, 0.0f);

        LOG_Item_WARNING(TEXT("[GenerateGridByDistance] [%d] Col=%d, Location=(%.1f, %.1f, %.1f)"),
            i, Col, SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);

        FTransform SpawnTransform;
        SpawnTransform.SetLocation(SpawnLocation);
        SpawnTransform.SetRotation(FQuat::Identity);

        DummySpawnPoints.Add(SpawnTransform);
    }

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] GridByDistance - 최대 %d개 중 %d개 생성"),
        GridColumns, SpawnCount);
}

void ATrainingRoomManager::SubscribeToWeaponStats(UWeaponStatsComponent* WeaponStatsComp)
{
    if (!WeaponStatsComp)
    {
        return;
    }

    // 델리게이트 바인딩
    WeaponStatsComp->OnStatsUpdated.AddDynamic(this, &ATrainingRoomManager::OnWeaponStatsUpdated);

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] WeaponStats 이벤트 구독 완료"));
}

void ATrainingRoomManager::UnsubscribeFromWeaponStats(UWeaponStatsComponent* WeaponStatsComp)
{
    if (!WeaponStatsComp)
    {
        return;
    }

    // 델리게이트 언바인딩
    WeaponStatsComp->OnStatsUpdated.RemoveDynamic(this, &ATrainingRoomManager::OnWeaponStatsUpdated);

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] WeaponStats 이벤트 구독 해제"));
}

void ATrainingRoomManager::OnWeaponStatsUpdated()
{
    if (!HasAuthority() || !bSessionActive || !PlayerWeaponStats)
    {
        return;
    }

    // WeaponStatsComponent에서 직접 데이터 읽기
    FWeaponUsageStats WeaponStats = PlayerWeaponStats->TotalStats;

    CurrentStats.TotalShots = WeaponStats.TotalShots;
    CurrentStats.TotalHits = WeaponStats.ShotsHit;
    CurrentStats.TotalDamageDealt = WeaponStats.TotalDamageDealt;

    // 명중률 계산
    CurrentStats.CalculateAccuracy();

    // 세션 시간 및 DPS 계산
    CurrentStats.SessionTime = GetWorld()->GetTimeSeconds() - SessionStartTime;
    CurrentStats.CalculateDPS();

    LOG_Item_WARNING(TEXT("[TrainingRoomManager] 통계 동기화 - 발사: %d, 명중: %d, 명중률: %.1f%%"),
        CurrentStats.TotalShots, CurrentStats.TotalHits, CurrentStats.AccuracyPercentage);

    // 델리게이트 브로드캐스트 (UI 업데이트용)
    OnStatsUpdated.Broadcast(CurrentStats);
}



