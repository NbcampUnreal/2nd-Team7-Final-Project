#include "RuinsMazeManager.h"
#include "RuinsMazeWall.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "LastCanary.h"

ARuinsMazeManager::ARuinsMazeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    MazeBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("MazeBounds"));
    RootComponent = MazeBounds;
}

void ARuinsMazeManager::BeginPlay()
{
    Super::BeginPlay();
    GenerateMaze();
}

void ARuinsMazeManager::GenerateMaze()
{
    LOG_Art(Log, TEXT("미로 생성 시작"));

    if (!WallClass)
    {
        LOG_Art_ERROR(TEXT("WallClass가 지정되지 않았습니다."));
        return;
    }

    const FVector BoundsExtent = MazeBounds->GetScaledBoxExtent();
    const FVector BoundsCenter = MazeBounds->GetComponentLocation();
    MazeOrigin = BoundsCenter - FVector(MazeSizeX * CellSize, MazeSizeY * CellSize, 0.f) * 0.5f;

    MazeCells.SetNum(MazeSizeX);
    for (int32 X = 0; X < MazeSizeX; ++X)
    {
        MazeCells[X].SetNum(MazeSizeY);
    }

    //  입구/출구 셀: Top, Bottom 경계에서 X 좌표 랜덤
    FIntPoint EntranceCell(0, FMath::RandRange(0, MazeSizeY - 1));
    FIntPoint ExitCell(MazeSizeX - 1, FMath::RandRange(0, MazeSizeY - 1));

    // DFS 메인 경로 생성
    GenerateMainPath(EntranceCell);

    //  입구/출구 벽 제거
    MazeCells[EntranceCell.X][EntranceCell.Y].bWallLeft = false;
    LOG_Art(Log, TEXT("입구 셀 (%d, %d)의 Left 벽 제거 완료"), EntranceCell.X, EntranceCell.Y);

    MazeCells[ExitCell.X][ExitCell.Y].bWallRight = false;
    LOG_Art(Log, TEXT("출구 셀 (%d, %d)의 Right 벽 제거 완료"), ExitCell.X, ExitCell.Y);

    //  각 셀 벽 생성
    for (int32 X = 0; X < MazeSizeX; ++X)
    {
        for (int32 Y = 0; Y < MazeSizeY; ++Y)
        {
            const FRuinsMazeCell& Cell = MazeCells[X][Y];
            FVector CellWorld = MazeOrigin + FVector(X * CellSize, Y * CellSize, 0.f);

            if (Cell.bWallTop)
                SpawnWall(CellWorld + FVector(0.f, CellSize / 2, 0.f), FRotator(0.f, 0.f, 0.f));
            if (Cell.bWallBottom)
                SpawnWall(CellWorld + FVector(0.f, -CellSize / 2, 0.f), FRotator(0.f, 0.f, 0.f));
            if (Cell.bWallLeft)
                SpawnWall(CellWorld + FVector(-CellSize / 2, 0.f, 0.f), FRotator(0.f, 90.f, 0.f));
            if (Cell.bWallRight)
                SpawnWall(CellWorld + FVector(CellSize / 2, 0.f, 0.f), FRotator(0.f, 90.f, 0.f));
        }
    }

    LOG_Art(Log, TEXT("미로 생성 완료"));

    //  경로 유효성 검사
    IsPathToExitValid(EntranceCell, ExitCell);
}

void ARuinsMazeManager::GenerateMainPath(const FIntPoint& Cell)
{
    MazeCells[Cell.X][Cell.Y].bVisited = true;

    for (const FIntPoint& Neighbor : GetShuffledUnvisitedNeighbors(Cell))
    {
        if (!MazeCells[Neighbor.X][Neighbor.Y].bVisited)
        {
            if (Neighbor.X > Cell.X)
            {
                MazeCells[Cell.X][Cell.Y].bWallRight = false;
                MazeCells[Neighbor.X][Neighbor.Y].bWallLeft = false;
            }
            else if (Neighbor.X < Cell.X)
            {
                MazeCells[Cell.X][Cell.Y].bWallLeft = false;
                MazeCells[Neighbor.X][Neighbor.Y].bWallRight = false;
            }
            else if (Neighbor.Y > Cell.Y)
            {
                MazeCells[Cell.X][Cell.Y].bWallTop = false;
                MazeCells[Neighbor.X][Neighbor.Y].bWallBottom = false;
            }
            else if (Neighbor.Y < Cell.Y)
            {
                MazeCells[Cell.X][Cell.Y].bWallBottom = false;
                MazeCells[Neighbor.X][Neighbor.Y].bWallTop = false;
            }

            GenerateMainPath(Neighbor);
        }
    }
}

TArray<FIntPoint> ARuinsMazeManager::GetShuffledUnvisitedNeighbors(const FIntPoint& Cell)
{
    TArray<FIntPoint> Neighbors;
    if (Cell.X > 0) Neighbors.Add(FIntPoint(Cell.X - 1, Cell.Y));
    if (Cell.X < MazeSizeX - 1) Neighbors.Add(FIntPoint(Cell.X + 1, Cell.Y));
    if (Cell.Y > 0) Neighbors.Add(FIntPoint(Cell.X, Cell.Y - 1));
    if (Cell.Y < MazeSizeY - 1) Neighbors.Add(FIntPoint(Cell.X, Cell.Y + 1));

    for (int32 i = 0; i < Neighbors.Num(); ++i)
    {
        int32 j = FMath::RandRange(i, Neighbors.Num() - 1);
        Neighbors.Swap(i, j);
    }

    return Neighbors;
}

void ARuinsMazeManager::SpawnWall(const FVector& Location, const FRotator& Rotation)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    LOG_Art(Log, TEXT("벽 생성: 위치 = (%.0f, %.0f, %.0f), 회전 = (%.0f, %.0f, %.0f)"),
        Location.X, Location.Y, Location.Z,
        Rotation.Pitch, Rotation.Yaw, Rotation.Roll);

    GetWorld()->SpawnActor<ARuinsMazeWall>(WallClass, Location, Rotation, Params);
}

bool ARuinsMazeManager::IsPathToExitValid(const FIntPoint& Start, const FIntPoint& End)
{
    TSet<FIntPoint> Visited;
    bool bValid = RecursiveCheckPath(Start, End, Visited);

    if (bValid)
    {
        LOG_Art(Log, TEXT("입구에서 출구까지 경로가 연결되어 있습니다."));
    }
    else
    {
        LOG_Art_WARNING(TEXT("입구에서 출구까지 경로가 없습니다!"));
    }

    return bValid;
}

bool ARuinsMazeManager::RecursiveCheckPath(const FIntPoint& Current, const FIntPoint& Target, TSet<FIntPoint>& Visited)
{
    if (Current == Target)
        return true;

    Visited.Add(Current);
    const FRuinsMazeCell& Cell = MazeCells[Current.X][Current.Y];

    const TArray<TPair<FIntPoint, bool>> Directions = {
        {FIntPoint(0, 1), !Cell.bWallTop},
        {FIntPoint(0, -1), !Cell.bWallBottom},
        {FIntPoint(-1, 0), !Cell.bWallLeft},
        {FIntPoint(1, 0), !Cell.bWallRight},
    };

    for (const auto& [Offset, bPassable] : Directions)
    {
        FIntPoint Next = Current + Offset;
        if (bPassable && !Visited.Contains(Next) &&
            Next.X >= 0 && Next.X < MazeSizeX &&
            Next.Y >= 0 && Next.Y < MazeSizeY)
        {
            if (RecursiveCheckPath(Next, Target, Visited))
                return true;
        }
    }

    return false;
}

