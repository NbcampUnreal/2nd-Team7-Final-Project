#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "LCGimmickSpawnTypes.generated.h"

UENUM(BlueprintType)
enum class ESpawnerRole : uint8
{
	Trigger		UMETA(DisplayName = "Trigger"),
	Target		UMETA(DisplayName = "Target"),
	SelfContained UMETA(DisplayName = "SelfContained"),
};

USTRUCT(BlueprintType)
struct FSpawnCandidate
{
	GENERATED_BODY()

	/** 스폰할 기믹 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> Class;

	/** 확률 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight = 1.f;
};

USTRUCT(BlueprintType)
struct FGlobalSpawnQuota
{
	GENERATED_BODY()

	/** 대상 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> Class;

	/** 스폰 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxCount = 0;
};
