#include "RuinsMazeManager.h"
#include "RuinsMazeWall.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
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
        LOG_Art(Error, TEXT("WallClass가 지정되지 않았습니다."));
        return;
    }

    const FVector BoundsExtent = MazeBounds->GetScaledBoxExtent();
    const FVector BoundsOrigin = MazeBounds->GetComponentLocation();
    MazeOrigin = BoundsOrigin - FVector(BoundsExtent.X, BoundsExtent.Y, 0.f);

    MazeCells.SetNum(MazeSizeX);
    for (int32 X = 0; X < MazeSizeX; ++X)
    {
        MazeCells[X].SetNum(MazeSizeY);
    }

    GenerateMainPath(FIntPoint(0, 0));

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
    IsPathToExitValid(FIntPoint(0, 0), FIntPoint(MazeSizeX - 1, MazeSizeY - 1));
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
    if (Cell.X > 0)
        Neighbors.Add(FIntPoint(Cell.X - 1, Cell.Y));
    if (Cell.X < MazeSizeX - 1)
        Neighbors.Add(FIntPoint(Cell.X + 1, Cell.Y));
    if (Cell.Y > 0)
        Neighbors.Add(FIntPoint(Cell.X, Cell.Y - 1));
    if (Cell.Y < MazeSizeY - 1)
        Neighbors.Add(FIntPoint(Cell.X, Cell.Y + 1));

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
    GetWorld()->SpawnActor<ARuinsMazeWall>(WallClass, Location, Rotation, Params);
}

bool ARuinsMazeManager::IsPathToExitValid(const FIntPoint& Start, const FIntPoint& End)
{
    TSet<FIntPoint> VisitedPathCheck;
    bool bValid = RecursiveCheckPath(Start, End, VisitedPathCheck);

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

bool ARuinsMazeManager::RecursiveCheckPath(const FIntPoint& Current, const FIntPoint& Target, TSet<FIntPoint>& VisitedPathCheck)
{
    if (Current == Target)
        return true;

    VisitedPathCheck.Add(Current);
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
        if (bPassable && !VisitedPathCheck.Contains(Next) &&
            Next.X >= 0 && Next.X < MazeSizeX &&
            Next.Y >= 0 && Next.Y < MazeSizeY)
        {
            if (RecursiveCheckPath(Next, Target, VisitedPathCheck))
                return true;
        }
    }

    return false;
}
