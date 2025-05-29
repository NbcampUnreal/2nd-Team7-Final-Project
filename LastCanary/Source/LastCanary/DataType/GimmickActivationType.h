#pragma once

#include "GimmickActivationType.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EGimmickActivationType : uint8
{
    ActivateOnStep         UMETA(DisplayName = "밟으면 작동 (유지됨)"),
    ActivateWhileStepping  UMETA(DisplayName = "밟고 있어야 작동"),
    ActivateAfterDelay     UMETA(DisplayName = "일정 시간 밟아야 작동"),
    ActivateTimed          UMETA(DisplayName = "밟으면 일정 시간 작동")
};