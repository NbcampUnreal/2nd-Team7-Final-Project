#include "Art/Ruins/Maze/RuinsMazeManager.h"
#include "Art/Ruins/Maze/RuinsMazeWall.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"

ARuinsMazeManager::ARuinsMazeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // MazeBounds는 디버깅/생성 영역 시각화를 위한 BoxComponent
    MazeBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("MazeBounds"));
    RootComponent = MazeBounds;
    MazeBounds->SetBoxExtent(FVector(1000.f, 1000.f, 100.f));
}

void ARuinsMazeManager::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        GenerateMaze();
    }
}

void ARuinsMazeManager::GenerateMaze()
{
    const FVector Extent = MazeBounds->GetUnscaledBoxExtent();
    const FVector Center = MazeBounds->GetComponentLocation();

    // MazeBounds 내부 크기를 기반으로 셀 크기 계산
    MazeSizeX = FMath::FloorToInt((Extent.X * 2.f) / CellSize);
    MazeSizeY = FMath::FloorToInt((Extent.Y * 2.f) / CellSize);

    // 중심에서 좌하단으로 이동 → 정렬된 MazeOrigin 계산
    MazeOrigin = Center - FVector(Extent.X, Extent.Y, 0.f);

    // 셀 배열 초기화
    VisitedCells.SetNum(MazeSizeY);
    for (int32 Y = 0; Y < MazeSizeY; ++Y)
    {
        VisitedCells[Y].Init(false, MazeSizeX);
    }

    LOG_Art(Log, TEXT("미로 셀 크기: %d x %d"), MazeSizeX, MazeSizeY);

    FindEntranceAndExit();
}

void ARuinsMazeManager::FindEntranceAndExit()
{
    TArray<AActor*> Entrances, Exits;
    UGameplayStatics::GetAllActorsWithTag(this, FName("MazeEntrance"), Entrances);
    UGameplayStatics::GetAllActorsWithTag(this, FName("MazeExit"), Exits);

    CHECK_Art(Entrances.Num() > 0 && Exits.Num() > 0, );

    const FIntPoint EntranceCell = WorldToCell(Entrances[0]->GetActorLocation());
    const FIntPoint ExitCell = WorldToCell(Exits[0]->GetActorLocation());

    if (!IsValidCell(EntranceCell) || !IsValidCell(ExitCell))
    {
        LOG_Art_ERROR(TEXT("입출구가 MazeBounds 범위 밖입니다: 입구 (%d,%d), 출구 (%d,%d)"),
            EntranceCell.X, EntranceCell.Y, ExitCell.X, ExitCell.Y);
        return;
    }

    LOG_Art(Log, TEXT("입구 셀: (%d, %d) | 출구 셀: (%d, %d)"), EntranceCell.X, EntranceCell.Y, ExitCell.X, ExitCell.Y);

    GenerateRecursive(EntranceCell);
}

void ARuinsMazeManager::GenerateRecursive(const FIntPoint& Cell)
{
    VisitedCells[Cell.Y][Cell.X] = true;

    for (const FIntPoint& Dir : GetShuffledDirections())
    {
        FIntPoint Next = Cell + Dir;

        if (IsValidCell(Next) && !VisitedCells[Next.Y][Next.X])
        {
            RemoveWallBetween(Cell, Next);
            GenerateRecursive(Next);
        }
    }
}

TArray<FIntPoint> ARuinsMazeManager::GetShuffledDirections()
{
    TArray<FIntPoint> Directions = {
        FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1)
    };

    for (int32 i = Directions.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Directions.Swap(i, j);
    }

    return Directions;
}

bool ARuinsMazeManager::IsValidCell(const FIntPoint& Cell) const
{
    return Cell.X >= 0 && Cell.X < MazeSizeX && Cell.Y >= 0 && Cell.Y < MazeSizeY;
}

FIntPoint ARuinsMazeManager::WorldToCell(const FVector& WorldLocation) const
{
    const FVector Local = MazeBounds->GetComponentTransform().InverseTransformPosition(WorldLocation);
    const FVector Extent = MazeBounds->GetUnscaledBoxExtent();

    const int32 X = FMath::FloorToInt((Local.X + Extent.X) / CellSize);
    const int32 Y = FMath::FloorToInt((Local.Y + Extent.Y) / CellSize);

    return FIntPoint(X, Y);
}

FVector ARuinsMazeManager::CellToWorld(const FIntPoint& Cell) const
{
    return MazeOrigin + FVector(Cell.X * CellSize, Cell.Y * CellSize, 0.f);
}

void ARuinsMazeManager::RemoveWallBetween(const FIntPoint& From, const FIntPoint& To)
{
    SpawnWall(From, To);
}

void ARuinsMazeManager::SpawnWall(const FIntPoint& From, const FIntPoint& To)
{
    CHECK_Art(WallClass != nullptr, );

    const FVector Mid = (CellToWorld(From) + CellToWorld(To)) / 2.f;
    const FRotator Rot = (From.X != To.X) ? FRotator(0.f, 0.f, 0.f) : FRotator(0.f, 90.f, 0.f);

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.Owner = this;

    ARuinsMazeWall* Wall = GetWorld()->SpawnActor<ARuinsMazeWall>(WallClass, Mid, Rot, Params);

    if (Wall)
    {
        SpawnedWalls.Add((From + To) / 2, Wall);
    }
}

