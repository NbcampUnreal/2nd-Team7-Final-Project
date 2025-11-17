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
	None				UMETA(DisplayName = "None"),
	//광물 등급 : 낮음
	Shard			    UMETA(DisplayName = "Shard"),
	//광물 등급 : 보통
	Core			    UMETA(DisplayName = "Core"),
	//광물 등급 : 높음
	Prism			    UMETA(DisplayName = "Prism"),
	// 금속 장신구류 (팔찌, 반지 등)
	Jewelry				UMETA(DisplayName = "Jewelry"),
	// 금화, 은화, 금괴 등
	Treasure			UMETA(DisplayName = "Treasure"),
	// 제사용 그릇, 성배 등
	RitualWare			UMETA(DisplayName = "RitualWare"),
	// 조각상, 상징 조각 등
	StatueOrSymbol		UMETA(DisplayName = "StatueOrSymbol"),
	// 의식용 무기/도구
	WeaponOrTool		UMETA(DisplayName = "WeaponOrTool"),
	// 분류 불명
	Other			    UMETA(DisplayName = "Other"),
};