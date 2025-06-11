#include "MonsterSpawnComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Components/BrushComponent.h"
#include "AI/BaseMonsterCharacter.h"
#include "Character/BaseCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DataTable/MonsterDataTable.h"
//스폰 중 startspawning함수 불러오게 하기
UMonsterSpawnComponent::UMonsterSpawnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsSpawning = false;
    ReSpawnCount = 0;
}

void UMonsterSpawnComponent::BeginPlay()
{
    Super::BeginPlay();

    MonsterClasses.Empty();

    CurrentMap();

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

void UMonsterSpawnComponent::SwitchDayNight()//타임 매니저가 불러야 할 것 //기존 몬스터를 모두 없애야 하는가?
{
    bIsNighttime = !bIsNighttime;

    if (bIsSpawning)
    {
        StopSpawning();
        StartSpawning();
    }
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
    DestroyAllMonsters();
}

FVector UMonsterSpawnComponent::GetValidSpawnLocationInNavVolume(const FVector& OwnerLocation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ZeroVector;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (!NavSys)
    {
        return FVector::ZeroVector;
    }

    ANavigationData* NavData = NavSys->GetDefaultNavDataInstance();
    if (!NavData)
    {
        return FVector::ZeroVector;
    }

    for (int32 Attempt = 0; Attempt < 100; Attempt++)
    {
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

        FNavLocation NavLocation;
        bool bFoundNavMesh = NavSys->ProjectPointToNavigation(//navmesh 찾아주는 함수
            TestLocation,
            NavLocation,
            FVector(100.0f, 100.0f, 500.0f)//검색 범위
        );

        if (!bFoundNavMesh)
        {
            continue;
        }

        FVector ValidNavLocation = NavLocation.Location;

        FHitResult HitResult;
        FVector TraceStart = ValidNavLocation;
        TraceStart.Z += 500.0f;

        FVector TraceEnd = ValidNavLocation;
        TraceEnd.Z -= 100.0f;

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
            FVector FinalLocation = HitResult.Location;
            FinalLocation.Z += 90.0f;
            return FinalLocation;
        }
        else
        {
            ValidNavLocation.Z += 90.0f;
            return ValidNavLocation;
        }
    }

    return FVector::ZeroVector;
}

void UMonsterSpawnComponent::CurrentMap()
{
    if (!MonsterDataTable)
    {
        return;
    }

    FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);

    for (const auto& Row : MonsterDataTable->GetRowMap())
    {
        const FMonsterDataTable* MonsterData = (FMonsterDataTable*)Row.Value;

        if (!MonsterData->Level.IsNull())
        {
            FString RowLevelName = FPackageName::GetShortName(MonsterData->Level.GetAssetName());

            if (RowLevelName == CurrentLevelName)
            {
                if (MonsterData->MonsterActor)
                {
                    MonsterClasses.Add(MonsterData->MonsterActor);
                }
            }
        }
    }
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

    int32 MonstersToSpawn = MaxMonsterCount - ReSpawnCount;

    if (MonstersToSpawn <= 0)
    {
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

    TArray<FVector> UsedLocations;

    int32 SpawnedCount = 0;
    int32 MaxAttempts = MonstersToSpawn * 10;

    for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnedCount < MonstersToSpawn; Attempt++)
    {
        FVector SpawnLocation = GetValidSpawnLocationInNavVolume(OwnerLocation);

        if (SpawnLocation == FVector::ZeroVector)
        {
            continue;
        }

        bool TooClose = false;//뭉쳐서 스폰되지 않도록

        for (const FVector& UsedLocation : UsedLocations)
        {
            float DistSq = FVector::DistSquared2D(SpawnLocation, UsedLocation);
            if (DistSq < 22500.0f)
            {
                TooClose = true;
                break;
            }
        }

        if (!TooClose)
        {
            for (TActorIterator<ABaseCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
            {
                ABaseCharacter* Player = *ActorItr;
                if (IsValid(Player) && Player->IsPlayerControlled())
                {
                    float DistToPlayer = FVector::Dist2D(SpawnLocation, Player->GetActorLocation());
                    if (DistToPlayer < AvoidSpawnRadius)
                    {
                        TooClose = true;
                        break;
                    }
                }
            }
        }

        if (TooClose)
        {
            continue;
        }

        int32 MonsterIndex = FMath::RandRange(0, MonsterClasses.Num() - 1);
        TSubclassOf<ABaseMonsterCharacter> MonsterClass = MonsterClasses[MonsterIndex];

        if (!MonsterClass)
        {
            continue;
        }

        FRotator RandomRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;//몬스터 안끼게, 안겹치게 해주는 안전장치

        ACharacter* SpawnedMonster = World->SpawnActor<ACharacter>(
            MonsterClass,
            SpawnLocation,
            RandomRotation,
            SpawnParams
        );

        if (SpawnedMonster)
        {
            SpawnedMonsters.Add(SpawnedMonster);
            UsedLocations.Add(SpawnLocation);
            SpawnedCount++;
        }
    }

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

    int32 NightMaxMonsterCount = FMath::RoundToInt(MaxMonsterCount * 1.5f);
    int32 MonstersToSpawn = NightMaxMonsterCount - ReSpawnCount;

    if (MonstersToSpawn <= 0)
    {
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

    TArray<FVector> UsedLocations;

    int32 SpawnedCount = 0;
    int32 MaxAttempts = MonstersToSpawn * 10;

    for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnedCount < MonstersToSpawn; Attempt++)
    {
        FVector SpawnLocation = GetValidSpawnLocationInNavVolume(OwnerLocation);

        if (SpawnLocation == FVector::ZeroVector)
        {
            continue;
        }

        bool TooClose = false;
        for (const FVector& UsedLocation : UsedLocations)
        {
            float DistSq = FVector::DistSquared2D(SpawnLocation, UsedLocation);
            if (DistSq < 22500.0f)
            {
                TooClose = true;
                break;
            }
        }

        if (!TooClose)
        {
            for (TActorIterator<ABaseCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
            {
                ABaseCharacter* Player = *ActorItr;
                if (IsValid(Player) && Player->IsPlayerControlled())
                {
                    float DistToPlayer = FVector::Dist2D(SpawnLocation, Player->GetActorLocation());
                    if (DistToPlayer < AvoidSpawnRadius)
                    {
                        TooClose = true;
                        break;
                    }
                }
            }
        }

        if (TooClose)
        {
            continue;
        }

        int32 MonsterIndex = FMath::RandRange(0, MonsterClasses.Num() - 1);
        TSubclassOf<ABaseMonsterCharacter> MonsterClass = MonsterClasses[MonsterIndex];

        if (!MonsterClass)
        {
            continue;
        }

        FRotator RandomRotation(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;//몬스터 안끼게, 안겹치게 해주는 안전장치

        ACharacter* SpawnedMonster = World->SpawnActor<ACharacter>(
            MonsterClass,
            SpawnLocation,
            RandomRotation,
            SpawnParams
        ); 

        if (SpawnedMonster)
        {
            SpawnedMonsters.Add(SpawnedMonster);
            UsedLocations.Add(SpawnLocation);
            SpawnedCount++;
        }
    }

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