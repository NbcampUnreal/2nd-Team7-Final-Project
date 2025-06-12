#include "RuinsMazeManager.h"
#include "RuinsMazeWall.h"
#include "Actor/Gimmick/Trigger/LCUnifiedPlate.h"
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
    LOG_Art(Log, TEXT("=== 미로 생성 시작 ==="));

    if (!WallClass)
    {
        LOG_Art_ERROR(TEXT("WallClass가 지정되지 않았습니다."));
        return;
    }

    const int32 MaxRetry = 80;
    const int32 MinValidPaths = 10;
    int32 RetryCount = 0;
    int32 PathCount = 0;

    do
    {
        // --- 초기화 ---
        MazeCells.Empty();

        const FVector BoundsExtent = MazeBounds->GetScaledBoxExtent();
        const FVector BoundsCenter = MazeBounds->GetComponentLocation();
        MazeOrigin = BoundsCenter - FVector(MazeSizeX * CellSize, MazeSizeY * CellSize, 0.f) * 0.5f;
        MazeOrigin.Z -= BoundsExtent.Z;

        // 로그 출력: 피벗 기준 Z 위치 확인
        LOG_Art(Log, TEXT("[MazeManager] GetActorLocation().Z          = %.1f"), GetActorLocation().Z);
        LOG_Art(Log, TEXT("[MazeManager] MazeBounds Center Z           = %.1f"), BoundsCenter.Z);
        LOG_Art(Log, TEXT("[MazeManager] MazeBounds Extent Z           = %.1f"), BoundsExtent.Z);
        LOG_Art(Log, TEXT("[MazeManager] MazeOrigin (지면 기준) Z      = %.1f"), MazeOrigin.Z);


        MazeCells.SetNum(MazeSizeX);
        for (int32 X = 0; X < MazeSizeX; ++X)
        {
            MazeCells[X].SetNum(MazeSizeY);
        }

        // --- 경로 생성 ---
        FindRandomEntranceAndExit();
        GenerateMainPath(EntranceCell);
        GenerateBranchesFromDeadEnds();
        ConnectLoopsToMainPath();

        // --- 경로 수 검사 ---
        PathCount = CountAllPathsToExit(EntranceCell, ExitCell);
        LOG_Art(Log, TEXT("재시도 #%d: 유효 경로 수 = %d"), RetryCount + 1, PathCount);

        RetryCount++;

    } while (PathCount < MinValidPaths && RetryCount < MaxRetry);

    if (PathCount < MinValidPaths)
    {
        LOG_Art_WARNING(TEXT("유효 경로 %d개 확보 실패 (최소 필요: %d) → 최종 생성 사용"), PathCount, MinValidPaths);
    }
    else
    {
        LOG_Art(Log, TEXT("미로 생성 성공: 유효 경로 수 %d (시도 %d회)"), PathCount, RetryCount);
    }

    // --- 입출구 벽 제거 ---
    MazeCells[EntranceCell.X][EntranceCell.Y].bWallLeft = false;
    MazeCells[ExitCell.X][ExitCell.Y].bWallRight = false;

    // --- 벽 스폰 ---
    for (int32 X = 0; X < MazeSizeX; ++X)
    {
        for (int32 Y = 0; Y < MazeSizeY; ++Y)
        {
            const FRuinsMazeCell& Cell = MazeCells[X][Y];
            FVector CellWorld = GetCellWorldPosition(FIntPoint(X, Y));

            if (Cell.bWallTop)     SpawnWall(CellWorld + FVector(0.f, CellSize / 2, 0.f), FRotator(0.f, 0.f, 0.f));
            if (Cell.bWallBottom)  SpawnWall(CellWorld + FVector(0.f, -CellSize / 2, 0.f), FRotator(0.f, 0.f, 0.f));
            if (Cell.bWallLeft)    SpawnWall(CellWorld + FVector(-CellSize / 2, 0.f, 0.f), FRotator(0.f, 90.f, 0.f));
            if (Cell.bWallRight)   SpawnWall(CellWorld + FVector(CellSize / 2, 0.f, 0.f), FRotator(0.f, 90.f, 0.f));
        }
    }

    IsPathToExitValid(EntranceCell, ExitCell);
    TryPlaceGimmicks();
    SpawnMonsterInMidPath();

    LOG_Art(Log, TEXT("=== 미로 생성 완료 ==="));
}

void ARuinsMazeManager::FindRandomEntranceAndExit()
{
    EntranceCell = FIntPoint(0, FMath::RandRange(0, MazeSizeY - 1));
    ExitCell = FIntPoint(MazeSizeX - 1, FMath::RandRange(0, MazeSizeY - 1));
    LOG_Art(Log, TEXT("입구: (%d, %d), 출구: (%d, %d)"), EntranceCell.X, EntranceCell.Y, ExitCell.X, ExitCell.Y);
}

void ARuinsMazeManager::GenerateMainPath(const FIntPoint& Start)
{
    MazeCells[Start.X][Start.Y].bVisited = true;

    MainPathCells.Add(Start);

    for (const FIntPoint& Neighbor : GetShuffledUnvisitedNeighbors(Start))
    {
        if (!MazeCells[Neighbor.X][Neighbor.Y].bVisited)
        {
            RemoveWallBetween(Start, Neighbor);
            GenerateMainPath(Neighbor);
        }
    }
}

void ARuinsMazeManager::GenerateBranchesFromDeadEnds()
{
    TArray<FIntPoint> DeadEnds = GetDeadEndCells();
    for (const FIntPoint& Cell : DeadEnds)
    {
        int32 BranchLength = FMath::RandRange(1, MaxBranchLength);
        FIntPoint Current = Cell;

        for (int32 i = 0; i < BranchLength; ++i)
        {
            TArray<FIntPoint> Neighbors = GetShuffledUnvisitedNeighbors(Current);
            if (Neighbors.Num() == 0) break;

            FIntPoint Next = Neighbors[0];
            RemoveWallBetween(Current, Next);
            MazeCells[Next.X][Next.Y].bVisited = true;
            Current = Next;
        }
    }
}

void ARuinsMazeManager::ConnectLoopsToMainPath()
{
    TArray<FIntPoint> DeadEnds = GetDeadEndCells();
    int32 Loops = 0;

    for (const FIntPoint& Cell : DeadEnds)
    {
        if (Loops >= MaxLoopCount) break;

        for (const FIntPoint& Neighbor : GetAllValidNeighbors(Cell))
        {
            if (MazeCells[Neighbor.X][Neighbor.Y].bVisited && TryConnectCells(Cell, Neighbor))
            {
                ++Loops;
                break;
            }
        }
    }
}

void ARuinsMazeManager::RemoveWallBetween(const FIntPoint& From, const FIntPoint& To)
{
    if (To.X > From.X) { MazeCells[From.X][From.Y].bWallRight = false; MazeCells[To.X][To.Y].bWallLeft = false; }
    else if (To.X < From.X) { MazeCells[From.X][From.Y].bWallLeft = false; MazeCells[To.X][To.Y].bWallRight = false; }
    else if (To.Y > From.Y) { MazeCells[From.X][From.Y].bWallTop = false; MazeCells[To.X][To.Y].bWallBottom = false; }
    else if (To.Y < From.Y) { MazeCells[From.X][From.Y].bWallBottom = false; MazeCells[To.X][To.Y].bWallTop = false; }
}

TArray<FIntPoint> ARuinsMazeManager::GetShuffledUnvisitedNeighbors(const FIntPoint& Cell)
{
    TArray<FIntPoint> Result;
    if (Cell.X > 0 && !MazeCells[Cell.X - 1][Cell.Y].bVisited) Result.Add(FIntPoint(Cell.X - 1, Cell.Y));
    if (Cell.X < MazeSizeX - 1 && !MazeCells[Cell.X + 1][Cell.Y].bVisited) Result.Add(FIntPoint(Cell.X + 1, Cell.Y));
    if (Cell.Y > 0 && !MazeCells[Cell.X][Cell.Y - 1].bVisited) Result.Add(FIntPoint(Cell.X, Cell.Y - 1));
    if (Cell.Y < MazeSizeY - 1 && !MazeCells[Cell.X][Cell.Y + 1].bVisited) Result.Add(FIntPoint(Cell.X, Cell.Y + 1));
    for (int32 i = 0; i < Result.Num(); ++i) { int32 j = FMath::RandRange(i, Result.Num() - 1); Result.Swap(i, j); }
    return Result;
}

TArray<FIntPoint> ARuinsMazeManager::GetAllValidNeighbors(const FIntPoint& Cell)
{
    TArray<FIntPoint> Neighbors;
    if (Cell.X > 0) Neighbors.Add(FIntPoint(Cell.X - 1, Cell.Y));
    if (Cell.X < MazeSizeX - 1) Neighbors.Add(FIntPoint(Cell.X + 1, Cell.Y));
    if (Cell.Y > 0) Neighbors.Add(FIntPoint(Cell.X, Cell.Y - 1));
    if (Cell.Y < MazeSizeY - 1) Neighbors.Add(FIntPoint(Cell.X, Cell.Y + 1));
    return Neighbors;
}

TArray<FIntPoint> ARuinsMazeManager::GetDeadEndCells() const
{
    TArray<FIntPoint> Result;
    for (int32 X = 0; X < MazeSizeX; ++X)
    {
        for (int32 Y = 0; Y < MazeSizeY; ++Y)
        {
            const FRuinsMazeCell& Cell = MazeCells[X][Y];
            int32 WallCount = (int32)Cell.bWallTop + Cell.bWallBottom + Cell.bWallLeft + Cell.bWallRight;
            if (WallCount == 3) Result.Add(FIntPoint(X, Y));
        }
    }
    return Result;
}

bool ARuinsMazeManager::TryConnectCells(const FIntPoint& From, const FIntPoint& To)
{
    if (FMath::Abs(From.X - To.X) + FMath::Abs(From.Y - To.Y) != 1) return false;
    RemoveWallBetween(From, To);
    return true;
}

FVector ARuinsMazeManager::GetCellWorldPosition(const FIntPoint& Cell) const
{
    return MazeOrigin + FVector(Cell.X * CellSize, Cell.Y * CellSize, 0.f);
}

void ARuinsMazeManager::SpawnWall(const FVector& Location, const FRotator& Rotation)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    GetWorld()->SpawnActor<ARuinsMazeWall>(WallClass, Location, Rotation, Params);
}

bool ARuinsMazeManager::IsPathToExitValid(const FIntPoint& Start, const FIntPoint& End)
{
    TSet<FIntPoint> Visited;
    return RecursiveCheckPath(Start, End, Visited);
}

bool ARuinsMazeManager::RecursiveCheckPath(const FIntPoint& Current, const FIntPoint& Target, TSet<FIntPoint>& Visited)
{
    if (Current == Target) return true;
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
            if (RecursiveCheckPath(Next, Target, Visited)) return true;
        }
    }
    return false;
}

int32 ARuinsMazeManager::CountAllPathsToExit(const FIntPoint& Start, const FIntPoint& End)
{
    TSet<FIntPoint> Visited;
    int32 PathCount = RecursiveCountPaths(Start, End, Visited);
    LOG_Art(Log, TEXT("총 유효 경로 수: %d"), PathCount);
    return PathCount;
}

int32 ARuinsMazeManager::RecursiveCountPaths(const FIntPoint& Current, const FIntPoint& Target, TSet<FIntPoint>& Visited)
{
    if (Current == Target)
        return 1;

    Visited.Add(Current);
    int32 Count = 0;

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
            Count += RecursiveCountPaths(Next, Target, Visited);
        }
    }

    Visited.Remove(Current);
    return Count;
}

FGimmickSpawnInfo* ARuinsMazeManager::GetRandomGimmick()
{
    float TotalWeight = 0.f;

    for (const FGimmickSpawnInfo& Info : GimmickSpawnList)
    {
        TotalWeight += Info.Weight;
    }

    if (TotalWeight <= 0.f)
    {
        LOG_Art_WARNING(TEXT("[GetRandomGimmick] 전체 Weight가 0 이하 → 기믹 없음"));
        return nullptr;
    }

    float Rand = FMath::FRandRange(0.f, TotalWeight);
    float Accum = 0.f;

    for (FGimmickSpawnInfo& Info : GimmickSpawnList)
    {
        Accum += Info.Weight;
        if (Rand <= Accum)
        {
            if (!Info.GimmickClass)
            {
                LOG_Art_WARNING(TEXT("[GetRandomGimmick] 선택된 기믹 클래스가 유효하지 않음"));
                return nullptr;
            }

            return &Info;
        }
    }

    LOG_Art_WARNING(TEXT("[GetRandomGimmick] 선택 실패 (Rand=%.2f, Total=%.2f)"), Rand, TotalWeight);
    return nullptr;
}

void ARuinsMazeManager::MaybeSpawnGimmickAtCell(const FIntPoint& Cell)
{
    if (!HasAuthority())
        return;

    if (GimmickInterval <= 0 || (Cell.X + Cell.Y) % GimmickInterval != 0)
        return;

    FGimmickSpawnInfo* Chosen = GetRandomGimmick();

    if (!Chosen)
    {
        LOG_Art_WARNING(TEXT("[Gimmick] (%d, %d) 랜덤 기믹 선택 실패"), Cell.X, Cell.Y);
        return;
    }

    if (!Chosen->GimmickClass)
    {
        LOG_Art_WARNING(TEXT("[Gimmick] (%d, %d) GimmickClass 미지정 → 선택된 구조체: Weight=%.2f"),
            Cell.X, Cell.Y, Chosen->Weight);
        return;
    }


    const FVector CellLocation = GetCellWorldPosition(Cell);
    const float MazeGroundZ = MazeOrigin.Z - CellSize * 0.5f;
    const FVector SpawnLoc = FVector(CellLocation.X, CellLocation.Y, MazeGroundZ + Chosen->ZOffset);

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* Gimmick = GetWorld()->SpawnActor<AActor>(Chosen->GimmickClass, SpawnLoc, Chosen->Rotation, Params);

    if (IsValid(Gimmick))
    {
        LOG_Art(Log, TEXT("[Gimmick] (%d, %d) 기믹 스폰 완료: %s"), Cell.X, Cell.Y, *GetNameSafe(Gimmick));
        PlacedGimmickCells.Add(Cell); 
    }
    else
    {
        LOG_Art_WARNING(TEXT("[Gimmick] (%d, %d) 기믹 스폰 실패"), Cell.X, Cell.Y);
    }
}

void ARuinsMazeManager::TryPlaceGimmicks()
{
    LOG_Art(Log, TEXT("[Gimmick] 전체 셀 순회 후 기믹 배치 시작"));

    TArray<FIntPoint> Candidates;

    for (int32 X = 0; X < MazeSizeX; ++X)
    {
        for (int32 Y = 0; Y < MazeSizeY; ++Y)
        {
            FIntPoint Coord(X, Y);

            if (IsValidGimmickSpawnCell(Coord))
            {
                Candidates.Add(Coord);
            }
        }
    }

    for (int32 i = 0; i < Candidates.Num(); ++i)
    {
        int32 j = FMath::RandRange(i, Candidates.Num() - 1);
        Candidates.Swap(i, j);
    }

    for (const FIntPoint& Coord : Candidates)
    {
        MaybeSpawnGimmickAtCell(Coord);
    }

    LOG_Art(Log, TEXT("[Gimmick] 전체 기믹 배치 완료"));
}


bool ARuinsMazeManager::IsValidGimmickSpawnCell(const FIntPoint& Cell) const
{
    return
        MazeCells.IsValidIndex(Cell.X) &&
        MazeCells[Cell.X].IsValidIndex(Cell.Y) &&
        MazeCells[Cell.X][Cell.Y].bVisited &&
        ((Cell.X + Cell.Y) % GimmickInterval == 0);
}

void ARuinsMazeManager::SpawnMonsterInMidPath()
{
    if (!HasAuthority())
        return;

    if (!MonsterClass)
    {
        LOG_Art_WARNING(TEXT("[Monster] MonsterClass 미지정 → 생성 생략"));
        return;
    }

    if (MainPathCells.Num() == 0)
    {
        LOG_Art_WARNING(TEXT("[Monster] MainPathCells 없음 → 생성 생략"));
        return;
    }

    const float CenterX = MazeSizeX / 2.f;
    const float CenterY = MazeSizeY / 2.f;

    const int32 MinDistFromEntranceX = MazeSizeX / 4;
    const int32 MinDistFromExitX = MazeSizeX / 4;

    FIntPoint BestCell = MainPathCells[0];
    float MinDistSq = TNumericLimits<float>::Max();
    bool bFound = false;

    for (const FIntPoint& Cell : MainPathCells)
    {
        if (FMath::Abs(Cell.X - EntranceCell.X) < MinDistFromEntranceX) continue;
        if (FMath::Abs(Cell.X - ExitCell.X) < MinDistFromExitX) continue;

        TSet<FIntPoint> TempVisited;
        if (!RecursiveCheckPath(Cell, ExitCell, TempVisited)) continue;

        if (PlacedGimmickCells.Contains(Cell)) continue;

        const float Dx = Cell.X - CenterX;
        const float Dy = Cell.Y - CenterY;
        const float DistSq = Dx * Dx + Dy * Dy;

        if (DistSq < MinDistSq)
        {
            MinDistSq = DistSq;
            BestCell = Cell;
            bFound = true;
        }
    }

    if (!bFound)
    {
        LOG_Art_WARNING(TEXT("[Monster] 조건에 맞는 유효 셀을 찾지 못해 스폰 생략"));
        return;
    }

    const FVector SpawnLoc = GetCellWorldPosition(BestCell) + FVector(0, 0, 50.f);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* Spawned = GetWorld()->SpawnActor<AActor>(MonsterClass, SpawnLoc, FRotator::ZeroRotator, Params);

    if (IsValid(Spawned))
    {
        LOG_Art(Log, TEXT("[Monster] 몬스터 스폰 성공 → 셀: (%d, %d)"), BestCell.X, BestCell.Y);
    }
    else
    {
        LOG_Art_WARNING(TEXT("[Monster] 몬스터 스폰 실패 → 셀: (%d, %d)"), BestCell.X, BestCell.Y);
    }
}

