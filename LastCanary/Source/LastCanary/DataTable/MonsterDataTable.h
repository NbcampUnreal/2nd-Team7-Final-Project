#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AI/BaseMonsterCharacter.h"
#include "MonsterDataTable.generated.h"

USTRUCT(BlueprintType)
struct FMonsterDataTable : public FTableRowBase
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MonsterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABaseMonsterCharacter> MonsterActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> Level;

	/** 스폰 시 오프셋 (보물 상자 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SpawnOffset = FVector(0.f, 0.f, 100.f);
};
