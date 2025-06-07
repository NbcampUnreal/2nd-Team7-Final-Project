#pragma once

#include "CoreMinimal.h"
#include "ResourceCategory.generated.h"

/**
 *
 */
UENUM(BlueprintType)
enum class EResourceCategory : uint8
{
	// 기본값
	None        UMETA(DisplayName = "None"),
	//광물 등급 : 낮음
	Shard       UMETA(DisplayName = "Shard"),
	//광물 등급 : 보통
	Core        UMETA(DisplayName = "Core"),
	//광물 등급 : 높음
	Prism       UMETA(DisplayName = "Prism"),
	// 분류 불명
	Other       UMETA(DisplayName = "Other")
};