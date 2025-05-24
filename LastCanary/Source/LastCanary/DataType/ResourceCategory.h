#pragma once

#include "CoreMinimal.h"
#include "ResourceCategory.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EResourceCategory : uint8
{
    None        UMETA(DisplayName = "None"),         // 기본값
    Ore         UMETA(DisplayName = "Ore"),          // 철, 아다만티움 등
    Gem         UMETA(DisplayName = "Gem"),          // 루비, 사파이어 등
    Plant       UMETA(DisplayName = "Plant"),        // 약초, 버섯류
    Creature    UMETA(DisplayName = "Creature"),     // 벌레, 뼈, 괴물 파편
    Artifact    UMETA(DisplayName = "Artifact"),     // 고대 유물류
    Other       UMETA(DisplayName = "Other")         // 분류 불명
};