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

    /** 트리거 액터 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    TSubclassOf<AActor> PlateClass;

    /** 작동 대상 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    TSubclassOf<AActor> TargetClass;

    /** 해당 조합의 등장 확률 가중치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    float Weight = 1.0f;

    /** Plate 기준 Target의 상대 위치 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    FVector TargetOffset = FVector(150.f, 0.f, 0.f);

    /** Plate의 회전값 (기믹 방향 설정용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    FRotator PlateRotation = FRotator::ZeroRotator;

    /** Target의 회전값 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    FRotator TargetRotation = FRotator::ZeroRotator;

    /** Plate의 Z 보정값 (지면에 붙이기 위함) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    float PlateZOffset = -50.f;

    /** Target의 Z 보정값 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
    float TargetZOffset = -50.f;
};

