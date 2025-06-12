#pragma once

#include "GimmickActivationType.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EGimmickActivationType : uint8
{
    /** ========== 자동 반복 기반 ========== */

    ActivateLooped          UMETA(DisplayName = " 자동 반복 작동"),

    /** ========== 상호작용 키 기반 ========== */

    ActivateOnPress         UMETA(DisplayName = "상호작용 - 누르면 작동"),
    ActivateWhileHolding    UMETA(DisplayName = "상호작용 - 누르고 있어야 작동"),

    /** ========== 오버랩 기반 ========== */

    ActivateOnStep         UMETA(DisplayName = "오버랩 - 밟으면 작동"),
    ActivateWhileStepping  UMETA(DisplayName = "오버랩 - 밟고 있어야 작동"),
    ActivateAfterDelay     UMETA(DisplayName = "오버랩 - 일정 시간 밟아야 작동"),

    /** ========== 기타 특정 조건 기반  ========== */

    ActivateOnConditionMet  UMETA(DisplayName = "기타 - 특정 조건 만족 시 작동")
};