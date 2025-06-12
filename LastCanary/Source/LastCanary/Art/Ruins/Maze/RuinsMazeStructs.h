#pragma once

#include "CoreMinimal.h"
#include "RuinsMazeStructs.generated.h"

/**
 * 셀 단위 미로 정보 (방향별 벽 여부 + 방문 여부)
 */
USTRUCT()
struct FRuinsMazeCell
{
    GENERATED_BODY()

    /** 이 셀이 DFS 탐색 중 방문되었는지 여부 */
    bool bVisited = false;

    // true일 경우 해당 방향에 벽 존재
    bool bWallTop = true;
    bool bWallBottom = true;
    bool bWallLeft = true;
    bool bWallRight = true;
};

/**
 * 미로에 배치 가능한 기믹의 스폰 정보 구조체
 * PlateClass → 트리거, TargetClass → 실행 대상 (문, 함정 등)
 */
USTRUCT(BlueprintType)
struct FGimmickSpawnInfo
{
    GENERATED_BODY()

    /** 스폰할 액터 클래스 (기믹 또는 보상, 기타) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    TSubclassOf<AActor> GimmickClass;

    /** 해당 기믹의 등장 확률 가중치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    float Weight = 1.0f;

    /** 기믹의 회전값 (Yaw 방향 등 설정용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    FRotator Rotation = FRotator::ZeroRotator;

    /** Z 오프셋 (지면 위로 띄우거나 바닥에 붙일 때 사용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    float ZOffset = -50.f;
};

