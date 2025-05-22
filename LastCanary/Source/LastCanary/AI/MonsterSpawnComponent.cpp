#include "MonsterSpawnComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "NavigationData.h"
#include "AI/Navigation/NavigationTypes.h"

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

    FVector OwnerLocation = Owner->GetActorLocation();
    float OwnerZ = OwnerLocation.Z;

    FMath::RandInit(FMath::Rand() + FPlatformTime::Seconds());

    TArray<FVector> UsedLocations;

    for (int32 i = ReSpawnCount; i < MaxMonsterCount * 5 && ReSpawnCount < MaxMonsterCount; i++)
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

        FVector SpawnLocation = OwnerLocation + SpawnOffset;

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

        if (TooClose)
        {
            continue;
        }

        FHitResult HitResult;
        FVector TraceStart = SpawnLocation;
        TraceStart.Z = OwnerZ + 500.0f;

        FVector TraceEnd = SpawnLocation;
        TraceEnd.Z = OwnerZ - 500.0f;

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Owner);

        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

        if (bHit)
        {
            SpawnLocation = HitResult.Location;
            SpawnLocation.Z += 10.0f;
        }
        else
        {
            SpawnLocation.Z = OwnerZ;
        }

        int32 MonsterIndex = FMath::RandRange(0, MonsterClasses.Num() - 1);
        TSubclassOf<ACharacter> MonsterClass = MonsterClasses[MonsterIndex];

        if (!MonsterClass)
        {
            continue;
        }

        FVector DirectionToCenter = OwnerLocation - SpawnLocation;
        DirectionToCenter.Z = 0;
        FRotator Rotation = DirectionToCenter.Rotation();

        Rotation.Yaw += FMath::FRandRange(-45.0f, 45.0f);

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
            ReSpawnCount++;
        }
    }

    World->GetTimerManager().SetTimer(
        DestroyTimerHandle,
        this,
        &UMonsterSpawnComponent::DestroyAllMonsters,
        MonsterLifeTime,
        false);

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

    FVector OwnerLocation = Owner->GetActorLocation();
    float OwnerZ = OwnerLocation.Z;

    FMath::RandInit(FMath::Rand() + FPlatformTime::Seconds());

    TArray<FVector> UsedLocations;

    int32 NightMaxMonsterCount = FMath::RoundToInt(MaxMonsterCount * 1.5f);

    for (int32 i = ReSpawnCount; i < NightMaxMonsterCount * 5 && ReSpawnCount < NightMaxMonsterCount; i++)
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

        FVector SpawnLocation = OwnerLocation + SpawnOffset;

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

        if (TooClose)
        {
            continue;
        }

        FHitResult HitResult;
        FVector TraceStart = SpawnLocation;
        TraceStart.Z = OwnerZ + 500.0f;

        FVector TraceEnd = SpawnLocation;
        TraceEnd.Z = OwnerZ - 500.0f;

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Owner);

        bool bHit = World->LineTraceSingleByChannel(
            HitResult,
            TraceStart,
            TraceEnd,
            ECC_Visibility,
            QueryParams
        );

        if (bHit)
        {
            SpawnLocation = HitResult.Location;
            SpawnLocation.Z += 10.0f;
        }
        else
        {
            SpawnLocation.Z = OwnerZ;
        }

        int32 MonsterIndex = FMath::RandRange(0, MonsterClasses.Num() - 1);
        TSubclassOf<ACharacter> MonsterClass = MonsterClasses[MonsterIndex];

        if (!MonsterClass)
        {
            continue;
        }

        FVector DirectionToCenter = OwnerLocation - SpawnLocation;
        DirectionToCenter.Z = 0;
        FRotator Rotation = DirectionToCenter.Rotation();

        Rotation.Yaw += FMath::FRandRange(-45.0f, 45.0f);

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
            ReSpawnCount++;
        }
    }

    World->GetTimerManager().SetTimer(
        DestroyTimerHandle,
        this,
        &UMonsterSpawnComponent::DestroyAllMonsters,
        MonsterLifeTime,
        false);

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
    for (ACharacter* Monster : SpawnedMonsters)
    {
        if (IsValid(Monster))
        {
            ReSpawnCount = SpawnedMonsters.Num();
        }
    }
}