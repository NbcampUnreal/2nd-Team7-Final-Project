#pragma once

#include "CoreMinimal.h"
#include "Art/Ruins/RuinsGimmickBase.h"
#include "RuinsMazeManager.generated.h"

class ARuinsMazeWall;
class UBoxComponent;

/**
 * MazeBounds 내부 셀 배열을 기반으로 미로를 생성하고,
 * 입구/출구는 해당 셀 내부에 포함되어야만 유효한 구조를 갖는 미로 생성 매니저
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ARuinsMazeManager : public ARuinsGimmickBase
{
    GENERATED_BODY()

public:
    ARuinsMazeManager();

protected:
    virtual void BeginPlay() override;

    /** 미로 생성 시작 함수 (Bounds 기반 셀 계산 → 경로 생성) */
    void GenerateMaze();

    /** MazeEntrance / MazeExit 태그를 기반으로 입출구 셀 좌표 설정 */
    void FindEntranceAndExit();

    /** 백트래킹 방식으로 미로 경로 생성 */
    void GenerateRecursive(const FIntPoint& Cell);

    /** 상하좌우 방향 셔플 반환 */
    TArray<FIntPoint> GetShuffledDirections();

    /** 유효한 셀인지 확인 */
    bool IsValidCell(const FIntPoint& Cell) const;

    /** 셀 간 벽 제거 → 시각화용 벽 생성 */
    void RemoveWallBetween(const FIntPoint& From, const FIntPoint& To);

    /** 셀 경계 사이에 벽을 스폰 */
    void SpawnWall(const FIntPoint& From, const FIntPoint& To);

    /** 월드 좌표 → MazeOrigin 기준 셀 좌표 변환 */
    FIntPoint WorldToCell(const FVector& WorldLocation) const;

    /** 셀 좌표 → 월드 위치 변환 */
    FVector CellToWorld(const FIntPoint& Cell) const;

protected:
    /** 셀 하나의 월드 단위 크기 (기본 200). 블루프린트에서 수정 가능 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maze", meta = (ClampMin = "50.0", UIMin = "50.0"))
    float CellSize = 200.f;

    /** 벽 스폰에 사용할 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Maze")
    TSubclassOf<ARuinsMazeWall> WallClass;

    /** 미로가 생성될 박스 영역 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maze")
    TObjectPtr<UBoxComponent> MazeBounds;

    /** 미로 경로 방문 여부 저장 배열 */
    TArray<TArray<bool>> VisitedCells;

    /** 셀 사이에 생성된 벽 저장 */
    TMap<FIntPoint, AActor*> SpawnedWalls;

    /** MazeBounds 좌하단에서 시작되는 미로 시작 월드 위치 */
    FVector MazeOrigin;

    /** 미로 셀 배열의 크기 */
    int32 MazeSizeX;
    int32 MazeSizeY;
};
