#include "MonsterSpawnComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Components/BrushComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UMonsterSpawnComponent::UMonsterSpawnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsSpawning = false;
    ReSpawnCount = 0;
}

void UMonsterSpawnComponent::BeginPlay()
{
    Super::BeginPlay();
    if (bAutoStart)
    {
        StartSpawning();
    }
}

void UMonsterSpawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSpawning();
    Super::EndPlay(EndPlayReason);
}

void UMonsterSpawnComponent::StartSpawning()
{
    if (bIsSpawning || MonsterClasses.Num() == 0)
    {
        return;
    }

    bIsSpawning = true;

    if (bIsNighttime)
    {
        SpawnNightMonsters();
    }
    else
    {
        SpawnMonsters();
    }
}

void UMonsterSpawnComponent::StopSpawning()
{
    if (!bIsSpawning)
    {
        return;
    }

    bIsSpawning = false;
    GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
    DestroyAllMonsters();
}

bool UMonsterSpawnComponent::IsLocationInNavMeshBounds(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    //NavMeshVolume 체크
    for (TActorIterator<ANavMeshBoundsVolume> ActorItr(World); ActorItr; ++ActorItr)
    {
        ANavMeshBoundsVolume* NavMeshVolume = *ActorItr;
        if (NavMeshVolume && NavMeshVolume->GetBrushComponent())
        {
            // NavMeshBoundsVolume의 바운드 박스 내부에 있는지 확인
            FBoxSphereBounds Bounds = NavMeshVolume->GetBrushComponent()->Bounds;
            FBox VolumeBox = Bounds.GetBox();

            if (VolumeBox.IsInside(Location))
            {
                return true;
            }
        }
    }

    return false;
}

FVector UMonsterSpawnComponent::GetValidSpawnLocationInNavVolume(const FVector& OwnerLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }

    // NavMeshBoundsVolume들을 수집
    TArray<ANavMeshBoundsVolume*> NavMeshVolumes;
    for (TActorIterator<ANavMeshBoundsVolume> ActorItr(World); ActorItr; ++ActorItr)
    {
        ANavMeshBoundsVolume* NavMeshVolume = *ActorItr;
        if (NavMeshVolume && NavMeshVolume->GetBrushComponent())
        {
            NavMeshVolumes.Add(NavMeshVolume);
        }
    }

    if (NavMeshVolumes.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    // 최대 100번 시도
    for (int32 Attempt = 0; Attempt < 100; Attempt++)
    {
        // 랜덤 위치 계산 (기존 로직 유지)
        float Quadrant = FMath::RandRange(0, 3);
        float BaseAngle = Quadrant * 90.0f;
        float AngleVariation = FMath::FRandRange(-45.0f, 45.0f);
        float FinalAngle = BaseAngle + AngleVariation;

        float RandomRatio = FMath::Sqrt(FMath::FRand());
        float FinalRadius = MinSpawnRadius + (RandomRatio * (MaxSpawnRadius - MinSpawnRadius));

        float AngleRad = FMath::DegreesToRadians(FinalAngle);

        FVector SpawnOffset(
            FMath::Cos(AngleRad) * FinalRadius,
            FMath::Sin(AngleRad) * FinalRadius,
            0.0f
        );

        FVector TestLocation = OwnerLocation + SpawnOffset;

        // NavMeshBoundsVolume 내부에 있는지 확인
        bool bInNavVolume = false;
        for (ANavMeshBoundsVolume* NavVolume : NavMeshVolumes)
        {
            FBoxSphereBounds Bounds = NavVolume->GetBrushComponent()->Bounds;
            FBox VolumeBox = Bounds.GetBox();
            if (VolumeBox.IsInside(TestLocation))
            {
                bInNavVolume = true;
                break;
            }
        }

        if (!bInNavVolume)
        {
            continue;
        }

        FHitResult HitResult;
        FVector TraceStart = TestLocation;
        TraceStart.Z = OwnerLocation.Z + 500.0f;

        FVector TraceEnd = TestLocation;
        TraceEnd.Z = OwnerLocation.Z - 500.0f;

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());

        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

        if (bHit)
        {
            FVector ValidLocation = HitResult.Location;
            ValidLocation.Z += 90.0f; //스폰높이 조절할때
            return ValidLocation;
        }
        else
        {
            //아니면 플레이어 Z 높이로
            FVector ValidLocation = TestLocation;
            ValidLocation.Z = OwnerLocation.Z + 90.0f;
            return ValidLocation;
        }
    }

    return FVector::ZeroVector;
}

void UMonsterSpawnComponent::SpawnMonsters()
{
    DestroyAllMonsters();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // 현재 살아있는 몬스터 수 확인 (DestroyAllMonsters에서 업데이트된 ReSpawnCount 사용)
    int32 MonstersToSpawn = MaxMonsterCount - ReSpawnCount;

    // 스폰할 몬스터가 없으면 리턴
    if (MonstersToSpawn <= 0)
    {
        // 다음 스폰 타이머 설정
        if (bIsSpawning)
        {
            World->GetTimerManager().SetTimer(
                SpawnTimerHandle,
                this,
                &UMonsterSpawnComponent::SpawnMonsters,
                SpawnCooldown,
                false);
        }
        return;
    }

    FVector OwnerLocation = Owner->GetActorLocation();
    FMath::RandInit(FMath::Rand() + FPlatformTime::Seconds());

    TArray<FVector> UsedLocations;

    // 기존 몬스터들의 위치도 UsedLocations에 추가
    for (ACharacter* ExistingMonster : SpawnedMonsters)
    {
        if (IsValid(ExistingMonster))
        {
            UsedLocations.Add(ExistingMonster->GetActorLocation());
        }
    }

    int32 SpawnedCount = 0;
    int32 MaxAttempts = MonstersToSpawn * 15;

    for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnedCount < MonstersToSpawn; Attempt++)
    {
        FVector SpawnLocation = GetValidSpawnLocationInNavVolume(OwnerLocation);

        if (SpawnLocation == FVector::ZeroVector)
        {
            continue; // 유효한 위치를 찾지 못함
        }

        // 기존 위치들과 너무 가까운지 확인
        bool TooClose = false;
        for (const FVector& UsedLocation : UsedLocations)
        {
            float DistSq = FVector::DistSquared2D(SpawnLocation, UsedLocation);
            if (DistSq < 22500.0f) // 150 units 거리
            {
                TooClose = true;
                break;
            }
        }

        if (TooClose)
        {
            continue;
        }

        // 몬스터 클래스 선택
        int32 MonsterIndex = FMath::RandRange(0, MonsterClasses.Num() - 1);
        TSubclassOf<ACharacter> MonsterClass = MonsterClasses[MonsterIndex];

        if (!MonsterClass)
        {
            continue;
        }

        // 회전 설정 (플레이어를 향하도록)
        FVector DirectionToCenter = OwnerLocation - SpawnLocation;
        DirectionToCenter.Z = 0;
        FRotator Rotation = DirectionToCenter.Rotation();
        Rotation.Yaw += FMath::FRandRange(-45.0f, 45.0f);

        // 몬스터 스폰
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACharacter* SpawnedMonster = World->SpawnActor<ACharacter>(
            MonsterClass,
            SpawnLocation,
            Rotation,
            SpawnParams
        );

        if (SpawnedMonster)
        {
            SpawnedMonsters.Add(SpawnedMonster);
            UsedLocations.Add(SpawnLocation);
            SpawnedCount++;
        }
    }

    // 다음 스폰 타이머 설정
    if (bIsSpawning)
    {
        World->GetTimerManager().SetTimer(
            SpawnTimerHandle,
            this,
            &UMonsterSpawnComponent::SpawnMonsters,
            SpawnCooldown,
            false);
    }
}

void UMonsterSpawnComponent::SpawnNightMonsters()
{
    DestroyAllMonsters();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // 현재 살아있는 몬스터 수 확인 (DestroyAllMonsters에서 업데이트된 ReSpawnCount 사용)
    int32 NightMaxMonsterCount = FMath::RoundToInt(MaxMonsterCount * 1.5f);
    int32 MonstersToSpawn = NightMaxMonsterCount - ReSpawnCount;

    // 스폰할 몬스터가 없으면 리턴
    if (MonstersToSpawn <= 0)
    {
        // 다음 스폰 타이머 설정
        if (bIsSpawning)
        {
            World->GetTimerManager().SetTimer(
                SpawnTimerHandle,
                this,
                &UMonsterSpawnComponent::SpawnNightMonsters,
                SpawnCooldown * 0.5f,
                false);
        }
        return;
    }

    FVector OwnerLocation = Owner->GetActorLocation();
    FMath::RandInit(FMath::Rand() + FPlatformTime::Seconds());

    TArray<FVector> UsedLocations;

    // 기존 몬스터들의 위치도 UsedLocations에 추가
    for (ACharacter* ExistingMonster : SpawnedMonsters)
    {
        if (IsValid(ExistingMonster))
        {
            UsedLocations.Add(ExistingMonster->GetActorLocation());
        }
    }

    int32 SpawnedCount = 0;
    int32 MaxAttempts = MonstersToSpawn * 15;

    for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnedCount < MonstersToSpawn; Attempt++)
    {
        FVector SpawnLocation = GetValidSpawnLocationInNavVolume(OwnerLocation);

        if (SpawnLocation == FVector::ZeroVector)
        {
            continue; // 유효한 위치를 찾지 못함
        }

        // 기존 위치들과 너무 가까운지 확인
        bool TooClose = false;
        for (const FVector& UsedLocation : UsedLocations)
        {
            float DistSq = FVector::DistSquared2D(SpawnLocation, UsedLocation);
            if (DistSq < 22500.0f) // 150 units 거리
            {
                TooClose = true;
                break;
            }
        }

        if (TooClose)
        {
            continue;
        }

        //몬스터선택
        int32 MonsterIndex = FMath::RandRange(0, MonsterClasses.Num() - 1);
        TSubclassOf<ACharacter> MonsterClass = MonsterClasses[MonsterIndex];

        if (!MonsterClass)
        {
            continue;
        }

        // 회전 설정 (플레이어를 향하도록)
        FVector DirectionToCenter = OwnerLocation - SpawnLocation;
        DirectionToCenter.Z = 0;
        FRotator Rotation = DirectionToCenter.Rotation();
        Rotation.Yaw += FMath::FRandRange(-45.0f, 45.0f);

        // 몬스터 스폰
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACharacter* SpawnedMonster = World->SpawnActor<ACharacter>(
            MonsterClass,
            SpawnLocation,
            Rotation,
            SpawnParams
        );

        if (SpawnedMonster)
        {
            SpawnedMonsters.Add(SpawnedMonster);
            UsedLocations.Add(SpawnLocation);
            SpawnedCount++;
        }
    }

    // 다음 스폰 타이머 설정
    if (bIsSpawning)
    {
        World->GetTimerManager().SetTimer(
            SpawnTimerHandle,
            this,
            &UMonsterSpawnComponent::SpawnNightMonsters,
            SpawnCooldown * 0.5f,
            false);
    }
}

void UMonsterSpawnComponent::DestroyAllMonsters()
{
    // 죽거나 삭제된 몬스터들을 배열에서 제거하고 살아있는 몬스터 수 계산
    SpawnedMonsters.RemoveAll([](ACharacter* Monster) 
    {
        return !IsValid(Monster);
    });

    ReSpawnCount = SpawnedMonsters.Num();

    //위는 기존 몬스터 유지, 아래는 기존 몬스터 삭제 후 리스폰
    /*for (ACharacter* Monster : SpawnedMonsters)
    {
        if (IsValid(Monster))
        {
            Monster->Destroy();
        }
    }
    SpawnedMonsters.Empty();
    ReSpawnCount = 0;*/
}