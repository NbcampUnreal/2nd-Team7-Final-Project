#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuinsMazeStructs.h"
#include "RuinsMazeManager.generated.h"

class UBoxComponent;
class ARuinsMazeWall;

/**
 * 신전 내부 미로를 관리하고 생성하는 매니저 클래스
 * DFS 알고리즘을 통해 미로를 생성하고, 셀 정보 기반으로 벽 Actor를 스폰
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
    /** 미로를 생성하는 함수 (BeginPlay에서 호출됨) */
    UFUNCTION(BlueprintCallable)
    void GenerateMaze();

private:
    /** 미로 전체 크기를 결정할 기준 박스 콜리전 */
    UPROPERTY(VisibleAnywhere, Category = "Maze")
    TObjectPtr<UBoxComponent> MazeBounds;

    /** 스폰할 벽 클래스 (반드시 ARuinsMazeWall을 상속해야 함) */
    UPROPERTY(EditAnywhere, Category = "Maze")
    TSubclassOf<ARuinsMazeWall> WallClass;

    /** 셀 간격 (1셀의 너비 및 높이) */
    UPROPERTY(EditAnywhere, Category = "Maze")
    float CellSize = 400.f;

    /** 셀 개수 - X 방향 (에디터에서 수정 가능) */
    UPROPERTY(EditAnywhere, Category = "Maze")
    int32 MazeSizeX = 10;

    /** 셀 개수 - Y 방향 (에디터에서 수정 가능) */
    UPROPERTY(EditAnywhere, Category = "Maze")
    int32 MazeSizeY = 10;

    /** 실제 미로 시작 위치 (Bounds 기준 좌측 하단) */
    FVector MazeOrigin;

    /** 셀 상태를 저장할 2차원 배열 (방문 여부 및 벽 상태 포함) */
    TArray<TArray<FRuinsMazeCell>> MazeCells;

    /** DFS 기반 메인 경로 생성 */
    void GenerateMainPath(const FIntPoint& StartCell);

    /** 지정한 셀의 네 방향 중, 방문하지 않은 방향을 랜덤으로 섞어 반환 */
    TArray<FIntPoint> GetShuffledUnvisitedNeighbors(const FIntPoint& Cell);

    /** 벽 생성 함수 */
    void SpawnWall(const FVector& Location, const FRotator& Rotation);

    /** === 디버깅 용 ==== */

    /** 입구에서 출구까지 실제 경로가 존재하는지 검사 */
    bool IsPathToExitValid(const FIntPoint& Start, const FIntPoint& End);

    /** DFS로 경로 존재 여부를 재귀 확인 */
    bool RecursiveCheckPath(const FIntPoint& Current, const FIntPoint& Target, TSet<FIntPoint>& VisitedPathCheck);
};