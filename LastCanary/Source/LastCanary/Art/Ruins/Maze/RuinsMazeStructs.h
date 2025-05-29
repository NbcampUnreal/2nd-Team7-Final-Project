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
