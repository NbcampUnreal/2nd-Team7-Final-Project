#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuinsMazeStructs.h"
#include "RuinsMazeManager.generated.h"

class UBoxComponent;
class ARuinsMazeWall;

/**
 * 신전 내부 미로를 관리하고 생성하는 매니저 클래스
 * 입구에서 출구까지 메인 경로를 만들고, 분기 및 루프를 포함한 복잡한 구조 생성
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ARuinsMazeManager : public AActor
{
    GENERATED_BODY()

public:
    ARuinsMazeManager();

protected:
    virtual void BeginPlay() override;

public:
    /** 미로 전체를 생성하는 함수 (BeginPlay에서 호출) */
    UFUNCTION(BlueprintCallable)
    void GenerateMaze();

private:
    /** 미로 범위를 결정할 Bounds (Box 컴포넌트) */
    UPROPERTY(VisibleAnywhere, Category = "Maze")
    TObjectPtr<UBoxComponent> MazeBounds;

    /** 스폰할 벽 클래스 */
    UPROPERTY(EditAnywhere, Category = "Maze")
    TSubclassOf<ARuinsMazeWall> WallClass;

    /** 셀 크기 (폭/높이 단위) */
    UPROPERTY(EditAnywhere, Category = "Maze")
    float CellSize = 400.f;

    /** 미로 크기 (X: 열 개수) */
    UPROPERTY(EditAnywhere, Category = "Maze")
    int32 MazeSizeX = 10;

    /** 미로 크기 (Y: 행 개수) */
    UPROPERTY(EditAnywhere, Category = "Maze")
    int32 MazeSizeY = 10;

    /** 루프 경로 최대 개수 */
    UPROPERTY(EditAnywhere, Category = "Maze")
    int32 MaxLoopCount = 4;

    /** 분기 경로 최대 길이 */
    UPROPERTY(EditAnywhere, Category = "Maze")
    int32 MaxBranchLength = 6;

    /** 전체 미로의 시작 기준 좌표 (왼쪽 하단) */
    FVector MazeOrigin;

    /** 셀 데이터 배열 */
    TArray<TArray<FRuinsMazeCell>> MazeCells;

    /** 입구 셀 좌표 */
    FIntPoint EntranceCell;

    /** 출구 셀 좌표 */
    FIntPoint ExitCell;

    /** ===== 경로 생성 관련 ===== */

    /** 메인 경로 생성 (입구 → 출구) */
    void GenerateMainPath(const FIntPoint& Start);

    /** 가지 경로 생성 (메인 경로에서 막다른 곳에 분기 생성) */
    void GenerateBranchesFromDeadEnds();

    /** 루프 생성 (가지 경로 일부를 다시 메인 경로로 연결) */
    void ConnectLoopsToMainPath();

    /** 셀 간 벽 제거 */
    void RemoveWallBetween(const FIntPoint& From, const FIntPoint& To);

    /** 벽 생성 함수 */
    void SpawnWall(const FVector& Location, const FRotator& Rotation);

    /** === 유틸리티 === */

    /** 입구/출구 셀 무작위 설정 */
    void FindRandomEntranceAndExit();

    /** 해당 셀의 방문하지 않은 이웃 반환 (셔플 포함) */
    TArray<FIntPoint> GetShuffledUnvisitedNeighbors(const FIntPoint& Cell);

    /** 현재 셀의 네 방향 이웃 반환 */
    TArray<FIntPoint> GetAllValidNeighbors(const FIntPoint& Cell);

    /** DeadEnd(막다른 셀) 수집 */
    TArray<FIntPoint> GetDeadEndCells() const;

    /** 두 셀 연결 시도 (기존 길이 있으면 활용) */
    bool TryConnectCells(const FIntPoint& From, const FIntPoint& To);

    /** 월드 좌표로 변환 */
    FVector GetCellWorldPosition(const FIntPoint& Cell) const;

    /** === 디버깅 === */

    /** 입구~출구 경로 유효성 검사 */
    bool IsPathToExitValid(const FIntPoint& Start, const FIntPoint& End);

    /** 재귀적 경로 검사 */
    bool RecursiveCheckPath(const FIntPoint& Current, const FIntPoint& Target, TSet<FIntPoint>& Visited);

    /** 입구~출구로 연결되는 모든 경로 수 계산 */
    int32 CountAllPathsToExit(const FIntPoint& Start, const FIntPoint& End);

    /** 재귀적 경로 수 계산 (내부 전용) */
    int32 RecursiveCountPaths(const FIntPoint& Current, const FIntPoint& Target, TSet<FIntPoint>& Visited);

    /** === 기믹 추가 === */

    /** 생성할 몬스터 클래스 (에디터에서 설정) */
    UPROPERTY(EditAnywhere, Category = "Maze|Monster")
    TSubclassOf<AActor> MonsterClass;

    /** 몬스터 생성 함수 */
    void SpawnMonsterInMidPath();

    /** 유효 경로 찾기 */
    TArray<FIntPoint> MainPathCells;

    /** 사용할 수 있는 기믹 조합 목록 */
    UPROPERTY(EditAnywhere, Category = "Maze|Gimmick")
    TArray<FGimmickSpawnInfo> GimmickSpawnList;

    /** 몇 칸마다 기믹을 생성할지 */
    UPROPERTY(EditAnywhere, Category = "Maze|Gimmick")
    int32 GimmickInterval = 5;

    /** 확률 기반으로 기믹 1개 선택 */
    FGimmickSpawnInfo* GetRandomGimmick();

    /** 셀 위치 기준 기믹 생성 시도 */
    void MaybeSpawnGimmickAtCell(const FIntPoint& Cell);

    /** 전체 셀 순회하며 기믹 생성 시도 (방문된 셀만) */
    void TryPlaceGimmicks();
};