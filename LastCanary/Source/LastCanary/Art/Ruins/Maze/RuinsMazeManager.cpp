#include "RuinsMazeManager.h"
#include "RuinsMazeWall.h"
#include "Actor/Gimmick/Trigger/LCUnifiedPlate.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Item/ResourceNode.h"
#include "Item/ResourceItem/ResourceItemSpawnManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"

ARuinsMazeManager::ARuinsMazeManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	MazeBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("MazeBounds"));
	RootComponent = MazeBounds;
}

void ARuinsMazeManager::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ARuinsMazeManager::GenerateMaze);
	}

	ResourceItemSpawnManagerRef = Cast<AResourceItemSpawnManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AResourceItemSpawnManager::StaticClass()));
	//if (ResourceItemSpawnManagerRef == nullptr)
	//{
	//	LOG_Item_WARNING(TEXT("[MazeManager] ResourceItemSpawnManager 찾기 실패"));
	//}
}

void ARuinsMazeManager::GenerateMaze()
{
	LOG_Art(Log, TEXT("=== 미로 생성 시작 ==="));

	if (WallClass == nullptr)
	{
		LOG_Art_ERROR(TEXT("WallClass가 지정되지 않았습니다."));
		return;
	}

	const int32 MaxRetry = 40;
	const int32 MinValidPaths = 7;
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

			if (Cell.bWallTop)
			{
				SpawnWall(CellWorld + FVector(0.f, CellSize / 2, 0.f), FRotator(0.f, 0.f, 0.f), FIntPoint(X, Y), "Top");
			}
			if (Cell.bWallBottom)
			{
				SpawnWall(CellWorld + FVector(0.f, -CellSize / 2, 0.f), FRotator(0.f, 0.f, 0.f), FIntPoint(X, Y), "Bottom");
			}
			if (Cell.bWallLeft)
			{
				SpawnWall(CellWorld + FVector(-CellSize / 2, 0.f, 0.f), FRotator(0.f, 90.f, 0.f), FIntPoint(X, Y), "Left");
			}
			if (Cell.bWallRight)
			{
				SpawnWall(CellWorld + FVector(CellSize / 2, 0.f, 0.f), FRotator(0.f, 90.f, 0.f), FIntPoint(X, Y), "Right");
			}
		}
	}

	IsPathToExitValid(EntranceCell, ExitCell);
	TryPlaceGimmicks();
	SpawnMonsterInMidPath();

	LOG_Art(Log, TEXT("=== 미로 생성 완료 ==="));
	SpawnNotesAfterMaze();
	SpawnTreasureBoxAfterMaze();
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
			if (Neighbors.Num() == 0)
			{
				break;
			}

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
	if (To.X > From.X)
	{
		MazeCells[From.X][From.Y].bWallRight = false;
		MazeCells[To.X][To.Y].bWallLeft = false;
		HideWall(From, "Right");
		HideWall(To, "Left");
		//Multicast_HideWall(From, "Right");
		//Multicast_HideWall(To, "Left");
	}
	else if (To.X < From.X)
	{
		MazeCells[From.X][From.Y].bWallLeft = false;
		MazeCells[To.X][To.Y].bWallRight = false;
		HideWall(From, "Left");
		HideWall(To, "Right");
		//Multicast_HideWall(From, "Left");
		//Multicast_HideWall(To, "Right");
	}
	else if (To.Y > From.Y)
	{
		MazeCells[From.X][From.Y].bWallTop = false;
		MazeCells[To.X][To.Y].bWallBottom = false;
		HideWall(From, "Top");
		HideWall(To, "Bottom");
		//Multicast_HideWall(From, "Top");
		//Multicast_HideWall(To, "Bottom");
	}
	else if (To.Y < From.Y)
	{
		MazeCells[From.X][From.Y].bWallBottom = false;
		MazeCells[To.X][To.Y].bWallTop = false;
		HideWall(From, "Bottom");
		HideWall(To, "Top");
		//Multicast_HideWall(From, "Bottom");
		//Multicast_HideWall(To, "Top");
	}
}

TArray<FIntPoint> ARuinsMazeManager::GetShuffledUnvisitedNeighbors(const FIntPoint& Cell)
{
	TArray<FIntPoint> Result;
	if (Cell.X > 0 && !MazeCells[Cell.X - 1][Cell.Y].bVisited)
	{
		Result.Add(FIntPoint(Cell.X - 1, Cell.Y));
	}
	if (Cell.X < MazeSizeX - 1 && !MazeCells[Cell.X + 1][Cell.Y].bVisited)
	{
		Result.Add(FIntPoint(Cell.X + 1, Cell.Y));
	}
	if (Cell.Y > 0 && !MazeCells[Cell.X][Cell.Y - 1].bVisited)
	{
		Result.Add(FIntPoint(Cell.X, Cell.Y - 1));
	}
	if (Cell.Y < MazeSizeY - 1 && !MazeCells[Cell.X][Cell.Y + 1].bVisited)
	{
		Result.Add(FIntPoint(Cell.X, Cell.Y + 1));
	}
	for (int32 i = 0; i < Result.Num(); ++i)
	{
		int32 j = FMath::RandRange(i, Result.Num() - 1);
		Result.Swap(i, j);
	}
	return Result;
}

TArray<FIntPoint> ARuinsMazeManager::GetAllValidNeighbors(const FIntPoint& Cell)
{
	TArray<FIntPoint> Neighbors;
	if (Cell.X > 0)
	{
		Neighbors.Add(FIntPoint(Cell.X - 1, Cell.Y));
	}
	if (Cell.X < MazeSizeX - 1)
	{
		Neighbors.Add(FIntPoint(Cell.X + 1, Cell.Y));
	}
	if (Cell.Y > 0)
	{
		Neighbors.Add(FIntPoint(Cell.X, Cell.Y - 1));
	}
	if (Cell.Y < MazeSizeY - 1)
	{
		Neighbors.Add(FIntPoint(Cell.X, Cell.Y + 1));
	}
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
			if (WallCount == 3)
			{
				Result.Add(FIntPoint(X, Y));
			}
		}
	}
	return Result;
}

bool ARuinsMazeManager::TryConnectCells(const FIntPoint& From, const FIntPoint& To)
{
	if (FMath::Abs(From.X - To.X) + FMath::Abs(From.Y - To.Y) != 1)
	{
		return false;
	}
	RemoveWallBetween(From, To);
	return true;
}

FVector ARuinsMazeManager::GetCellWorldPosition(const FIntPoint& Cell) const
{
	return MazeOrigin + FVector(Cell.X * CellSize, Cell.Y * CellSize, 0.f);
}

void ARuinsMazeManager::SpawnWall(const FVector& Location, const FRotator& Rotation, const FIntPoint& Cell, const FString& Direction)
{
	if (HasAuthority() == false)
	{
		return;
		//Multicast_SpawnWall(Location, Rotation, Cell, Direction);
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARuinsMazeWall* Wall = GetWorld()->SpawnActor<ARuinsMazeWall>(WallClass, Location, Rotation, Params);

	if (IsValid(Wall))
	{
		Wall->SetReplicates(true);
		WallCache.Add(TPair<FIntPoint, FString>(Cell, Direction), Wall);
		LOG_Art(Log, TEXT("[Wall] 스폰 성공 [%s:%s]"), *Cell.ToString(), *Direction);
	}
	else
	{
		LOG_Art_WARNING(TEXT("[Wall] 스폰 실패 [%s:%s]"), *Cell.ToString(), *Direction);
	}
}

bool ARuinsMazeManager::IsPathToExitValid(const FIntPoint& Start, const FIntPoint& End)
{
	TSet<FIntPoint> Visited;
	return RecursiveCheckPath(Start, End, Visited);
}

bool ARuinsMazeManager::RecursiveCheckPath(const FIntPoint& Current, const FIntPoint& Target, TSet<FIntPoint>& Visited)
{
	if (Current == Target)
	{
		return true;
	}
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
			{
				return true;
			}
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
	{
		return 1;
	}

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
	if (HasAuthority() == false)
	{
		return;
	}

	if (GimmickInterval <= 0 || (Cell.X + Cell.Y) % GimmickInterval != 0)
	{
		return;
	}

	FGimmickSpawnInfo* Chosen = GetRandomGimmick();

	if (Chosen == nullptr)
	{
		LOG_Art_WARNING(TEXT("[Gimmick] (%d, %d) 랜덤 기믹 선택 실패"), Cell.X, Cell.Y);
		return;
	}

	if (Chosen->GimmickClass == nullptr)
	{
		LOG_Art_WARNING(TEXT("[Gimmick] (%d, %d) GimmickClass 미지정 → 선택된 구조체: Weight=%.2f"),
			Cell.X, Cell.Y, Chosen->Weight);
		return;
	}

	int32& CurrentCount = GimmickSpawnCountMap.FindOrAdd(Chosen->GimmickClass);
	if (CurrentCount >= Chosen->MaxSpawnCount)
	{
		LOG_Art(Log, TEXT("[Gimmick] 최대 수량 도달 → %s (%d/%d)"), *Chosen->GimmickClass->GetName(), CurrentCount, Chosen->MaxSpawnCount);
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
		++CurrentCount;
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

//void ARuinsMazeManager::SpawnMonsterInMidPath()
//{
//	if (HasAuthority() == false)
//	{
//		return;
//	}
//	if (MonsterClass == nullptr)
//	{
//		LOG_Art_WARNING(TEXT("[Monster] MonsterClass 미지정 → 생성 생략"));
//		return;
//	}
//	if (MainPathCells.Num() == 0)
//	{
//		LOG_Art_WARNING(TEXT("[Monster] MainPathCells 없음 → 생성 생략"));
//		return;
//	}
//
//	const float CenterX = MazeSizeX / 2.f;
//	const float CenterY = MazeSizeY / 2.f;
//
//	const int32 MinDistFromEntranceX = MazeSizeX / 4;
//	const int32 MinDistFromExitX = MazeSizeX / 4;
//
//	FIntPoint BestCell = MainPathCells[0];
//	float MinDistSq = TNumericLimits<float>::Max();
//	bool bFound = false;
//
//	for (const FIntPoint& Cell : MainPathCells)
//	{
//		if (FMath::Abs(Cell.X - EntranceCell.X) < MinDistFromEntranceX)
//		{
//			continue;
//		}
//		if (FMath::Abs(Cell.X - ExitCell.X) < MinDistFromExitX)
//		{
//			continue;
//		}
//
//		TSet<FIntPoint> TempVisited;
//		if (!RecursiveCheckPath(Cell, ExitCell, TempVisited))
//		{
//			continue;
//		}
//
//		if (PlacedGimmickCells.Contains(Cell))
//		{
//			continue;
//		}
//
//		const float Dx = Cell.X - CenterX;
//		const float Dy = Cell.Y - CenterY;
//		const float DistSq = Dx * Dx + Dy * Dy;
//
//		if (DistSq < MinDistSq)
//		{
//			MinDistSq = DistSq;
//			BestCell = Cell;
//			bFound = true;
//		}
//	}
//
//	if (!bFound)
//	{
//		LOG_Art_WARNING(TEXT("[Monster] 조건에 맞는 유효 셀을 찾지 못해 스폰 생략"));
//		return;
//	}
//
//	const FVector SpawnLoc = GetCellWorldPosition(BestCell) + FVector(0, 0, 50.f);
//	FActorSpawnParameters Params;
//	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
//
//	AActor* Spawned = GetWorld()->SpawnActor<AActor>(MonsterClass, SpawnLoc, FRotator::ZeroRotator, Params);
//
//	if (IsValid(Spawned))
//	{
//		LOG_Art(Log, TEXT("[Monster] 몬스터 스폰 성공 → 셀: (%d, %d)"), BestCell.X, BestCell.Y);
//	}
//	else
//	{
//		LOG_Art_WARNING(TEXT("[Monster] 몬스터 스폰 실패 → 셀: (%d, %d)"), BestCell.X, BestCell.Y);
//	}
//}

void ARuinsMazeManager::SpawnMonsterInMidPath()
{
	if (HasAuthority() == false || MonsterClass == nullptr || MainPathCells.Num() == 0)
	{
		return;
	}

	TArray<FIntPoint> ValidCells;

	for (const FIntPoint& Cell : MainPathCells)
	{
		if (FMath::Abs(Cell.X - EntranceCell.X) < MazeSizeX / 4)
		{
			continue;
		}
		if (FMath::Abs(Cell.X - ExitCell.X) < MazeSizeX / 4)
		{
			continue;
		}
		TSet<FIntPoint> TempVisited;
		if (RecursiveCheckPath(Cell, ExitCell, TempVisited) == false)
		{
			continue;
		}
		if (PlacedGimmickCells.Contains(Cell))
		{
			continue;
		}

		ValidCells.Add(Cell);
	}

	if (ValidCells.Num() < MaxMonstersToSpawn)
	{
		LOG_Art_WARNING(TEXT("[Monster] 후보 셀 부족 → 요청: %d, 가능: %d"), MaxMonstersToSpawn, ValidCells.Num());
		return;
	}

	ValidCells.Sort([](const FIntPoint&, const FIntPoint&) { return FMath::RandBool(); });

	for (int32 i = 0; i < MaxMonstersToSpawn; ++i)
	{
		const FVector SpawnLoc = GetCellWorldPosition(ValidCells[i]) + FVector(0, 0, 50.f);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* Spawned = GetWorld()->SpawnActor<AActor>(MonsterClass, SpawnLoc, FRotator::ZeroRotator, Params);
		if (IsValid(Spawned))
		{
			LOG_Art(Log, TEXT("[Monster] 몬스터 스폰 성공 (%d/%d): (%d, %d)"), i + 1, MaxMonstersToSpawn, ValidCells[i].X, ValidCells[i].Y);
		}
	}
}

void ARuinsMazeManager::HideWall(const FIntPoint& Cell, const FString& Direction)
{
	if (HasAuthority() == false)
	{
		return;
	}

	auto Key = TPair<FIntPoint, FString>(Cell, Direction);
	ARuinsMazeWall** WallPtr = WallCache.Find(Key);

	if (WallPtr && IsValid(*WallPtr))
	{
		(*WallPtr)->SetActorHiddenInGame(true);
		(*WallPtr)->SetActorEnableCollision(false);
		return;
	}
	else
	{
		LOG_Art_WARNING(TEXT("❌ 게스트 HideWall 실패 - 위치: %s 방향: %s"), *Cell.ToString(), *Direction);
	}

	// fallback 검색 (서버일 경우만)
	//if (WallPtr == nullptr || IsValid(*WallPtr) == false)
	//{
	FVector WorldPos = GetCellWorldPosition(Cell);
	// 방향에 따라 약간 오프셋 줘서 근처 벽 찾기
	FVector Offset;
	if (Direction == "Top")
	{
		Offset = FVector(0, CellSize / 2, 0);
	}
	else if (Direction == "Bottom")
	{
		Offset = FVector(0, -CellSize / 2, 0);
	}
	else if (Direction == "Left")
	{
		Offset = FVector(-CellSize / 2, 0, 0);
	}
	else if (Direction == "Right")
	{
		Offset = FVector(CellSize / 2, 0, 0);
	}
	FVector SearchLocation = WorldPos + Offset;

	for (TActorIterator<ARuinsMazeWall> It(GetWorld()); It; ++It)
	{
		ARuinsMazeWall* Wall = *It;
		if (FVector::Dist(Wall->GetActorLocation(), SearchLocation) < 10.f)
		{
			Wall->SetActorHiddenInGame(true);
			Wall->SetActorEnableCollision(false);
			WallCache.Add(Key, Wall);
			LOG_Art(Log, TEXT("[Wall] 숨김 처리 fallback 성공 [%s:%s]"), *Cell.ToString(), *Direction);
			return;

			//WallCache.Add(Key, Wall);
			//WallPtr = &Wall;
			//break;
		}
	}
	//}
	//
	//if (WallPtr && IsValid(*WallPtr))
	//{
	//	(*WallPtr)->SetActorHiddenInGame(true);
	//	(*WallPtr)->SetActorEnableCollision(false);
	//	return;
	//}
	//else
	//{
	//	LOG_Art_WARNING(TEXT("❌ 게스트 HideWall 실패 - 위치: %s 방향: %s"), *Cell.ToString(), *Direction);
	//}
}

//void ARuinsMazeManager::Multicast_HideWall_Implementation(const FIntPoint& Cell, const FString& Direction)
//{
//	HideWall(Cell, Direction);
//}

//void ARuinsMazeManager::Multicast_SpawnWall_Implementation(const FVector& Location, const FRotator& Rotation, const FIntPoint& Cell, const FString& Direction)
//{
//	LOG_Art(Log, TEXT("📦 Multicast_SpawnWall 실행 [%s:%s]"), *Cell.ToString(), *Direction); // 이게 안 찍히면 RPC 자체 실패
//
//	FActorSpawnParameters Params;
//	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//
//	ARuinsMazeWall* Wall = GetWorld()->SpawnActor<ARuinsMazeWall>(WallClass, Location, Rotation, Params);
//	if (IsValid(Wall))
//	{
//		LOG_Art(Log, TEXT("✅ Wall 생성됨: %s → %s"), *Cell.ToString(), *Direction);
//		WallCache.Add(TPair<FIntPoint, FString>(Cell, Direction), Wall);
//	}
//	else
//	{
//		LOG_Art_WARNING(TEXT("❌ Wall 생성 실패: %s → %s"), *Cell.ToString(), *Direction);
//	}
//}

void ARuinsMazeManager::SpawnNotesAfterMaze()
{
	LOG_Item_WARNING(TEXT("[Notes] 미로 생성 후 노트 스폰 시작"));

	TArray<FIntPoint> CandidateCells;

	// 조건: DeadEnd + 방문한 셀 + 기믹 없는 셀
	for (const FIntPoint& Cell : GetDeadEndCells())
	{
		if (MazeCells[Cell.X][Cell.Y].bVisited && !PlacedGimmickCells.Contains(Cell))
		{
			CandidateCells.Add(Cell);
		}
	}

	if (CandidateCells.Num() < MaxNotesToSpawn)
	{
		LOG_Item_WARNING(TEXT("[Notes] 후보 셀 부족 → 요청: %d, 가능: %d"), MaxNotesToSpawn, CandidateCells.Num());
		return;
	}

	// 셀 섞기
	CandidateCells.Sort([](const FIntPoint&, const FIntPoint&) { return FMath::RandBool(); });

	for (int32 i = 0; i < MaxNotesToSpawn; ++i)
	{
		const FVector WorldPos = GetCellWorldPosition(CandidateCells[i]);
		if (IsValid(ResourceItemSpawnManagerRef))
		{
			ResourceItemSpawnManagerRef->SpawnNoteItemsAtLocation(1, WorldPos);
		}
	}

	//if (CandidateCells.Num() == 0)
	//{
	//	LOG_Item_WARNING(TEXT("[Notes] 노트 스폰 후보 셀이 없음"));
	//	return;
	//}

	//// 스폰할 노트 개수 제한
	//const int32 NumToSpawn = FMath::Min(MaxNotesToSpawn, CandidateCells.Num());

	//// 후보 셀 랜덤 섞기
	//for (int32 i = 0; i < CandidateCells.Num(); ++i)
	//{
	//	int32 j = FMath::RandRange(i, CandidateCells.Num() - 1);
	//	CandidateCells.Swap(i, j);
	//}

	//// SpawnManager 찾기
	//AResourceItemSpawnManager* SpawnManager = nullptr;
	//for (TActorIterator<AResourceItemSpawnManager> It(GetWorld()); It; ++It)
	//{
	//	SpawnManager = *It;
	//	break;
	//}
	//if (IsValid(SpawnManager) == false)
	//{
	//	LOG_Item_WARNING(TEXT("[Notes] ResourceItemSpawnManager를 찾을 수 없음 → 노트 스폰 생략"));
	//	return;
	//}
	//for (int32 i = 0; i < NumToSpawn; ++i)
	//{
	//	const FIntPoint& Cell = CandidateCells[i];
	//	FVector WorldPos = GetCellWorldPosition(Cell);
	//	SpawnManager->SpawnNoteItemsAtLocation(1, WorldPos); // 내부에서 랜덤 Row 선택
	//}

	LOG_Item_WARNING(TEXT("[Notes] 노트 스폰 완료: %d개 생성됨"), MaxNotesToSpawn);
}

//void ARuinsMazeManager::SpawnTreasureBoxAfterMaze()
//{
//	if (HasAuthority() == false)
//	{
//		return;
//	}
//
//	if (TreasureBoxSpawnCandidates.Num() == 0 || NumTreasureBoxToSpawn <= 0)
//	{
//		LOG_Art_WARNING(TEXT("[Chest] 스폰할 후보 상자가 없거나 개수가 0 이하"));
//		return;
//	}
//
//	LOG_Art(Log, TEXT("[Chest] 추가 액터 스폰 시작"));
//
//	// 후보 셀 리스트 생성: 방문됨 + 기믹 없음 + DeadEnd가 아닌 셀
//	TArray<FIntPoint> CandidateCells;
//	for (int32 X = 0; X < MazeSizeX; ++X)
//	{
//		for (int32 Y = 0; Y < MazeSizeY; ++Y)
//		{
//			FIntPoint Cell(X, Y);
//			const FRuinsMazeCell& MazeCell = MazeCells[X][Y];
//			if (MazeCell.bVisited && !PlacedGimmickCells.Contains(Cell))
//			{
//				CandidateCells.Add(Cell);
//			}
//		}
//	}
//
//	if (CandidateCells.Num() == 0)
//	{
//		LOG_Art_WARNING(TEXT("[Chest] 상자 배치할 셀이 없음"));
//		return;
//	}
//
//	// 셀 섞기
//	for (int32 i = 0; i < CandidateCells.Num(); ++i)
//	{
//		int32 j = FMath::RandRange(i, CandidateCells.Num() - 1);
//		CandidateCells.Swap(i, j);
//	}
//
//	// 최대 지정 개수만큼 스폰
//	const int32 SpawnCount = FMath::Min(NumTreasureBoxToSpawn, CandidateCells.Num());
//
//	for (int32 i = 0; i < SpawnCount; ++i)
//	{
//		const FIntPoint& Cell = CandidateCells[i];
//		const FVector SpawnLocation = GetCellWorldPosition(Cell) + FVector(0, 0, 50.0f); // 지면 위로 살짝 띄움
//
//		// 후보 중 하나 무작위 선택
//		const int32 RandIndex = FMath::RandRange(0, TreasureBoxSpawnCandidates.Num() - 1);
//		TSubclassOf<AResourceNode> SelectedClass = TreasureBoxSpawnCandidates[RandIndex];
//		if (!SelectedClass) continue;
//
//		FActorSpawnParameters Params;
//		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
//
//		AResourceNode* Node = GetWorld()->SpawnActor<AResourceNode>(SelectedClass, SpawnLocation, FRotator::ZeroRotator, Params);
//
//		if (IsValid(Node))
//		{
//			Node->ResourceItemSpawnManager = ResourceItemSpawnManagerRef;
//			LOG_Art(Log, TEXT("[Chest] 상자 스폰 성공 (%s) at (%d, %d)"), *Node->GetName(), Cell.X, Cell.Y);
//		}
//		else
//		{
//			LOG_Art_WARNING(TEXT("[Chest] 상자 스폰 실패 (%d, %d)"), Cell.X, Cell.Y);
//		}
//	}
//
//	LOG_Art(Log, TEXT("[Chest] 상자 스폰 완료"));
//}
void ARuinsMazeManager::SpawnTreasureBoxAfterMaze()
{
	if (HasAuthority() == false)
	{
		return;
	}

	TArray<FIntPoint> CandidateCells;
	for (int32 X = 0; X < MazeSizeX; ++X)
		for (int32 Y = 0; Y < MazeSizeY; ++Y)
		{
			FIntPoint Cell(X, Y);
			if (MazeCells[X][Y].bVisited && !PlacedGimmickCells.Contains(Cell))
				CandidateCells.Add(Cell);
		}

	if (CandidateCells.Num() < NumTreasureBoxToSpawn)
	{
		LOG_Art_WARNING(TEXT("[Chest] 후보 셀 부족 → 요청: %d, 가능: %d"), NumTreasureBoxToSpawn, CandidateCells.Num());
		return;
	}

	CandidateCells.Sort([](const FIntPoint&, const FIntPoint&) { return FMath::RandBool(); });

	for (int32 i = 0; i < NumTreasureBoxToSpawn; ++i)
	{
		const FVector SpawnLocation = GetCellWorldPosition(CandidateCells[i]) + FVector(0, 0, 50.0f);
		const int32 RandIndex = FMath::RandRange(0, TreasureBoxSpawnCandidates.Num() - 1);
		TSubclassOf<AResourceNode> SelectedClass = TreasureBoxSpawnCandidates[RandIndex];
		if (!SelectedClass) continue;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AResourceNode* Node = GetWorld()->SpawnActor<AResourceNode>(SelectedClass, SpawnLocation, FRotator::ZeroRotator, Params);

		if (IsValid(Node))
		{
			Node->ResourceItemSpawnManager = ResourceItemSpawnManagerRef;
			LOG_Art(Log, TEXT("[Chest] 상자 스폰 성공 (%s) at (%d, %d)"), *Node->GetName(), CandidateCells[i].X, CandidateCells[i].Y);
		}
	}
}
