#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BossMonsterRow.generated.h"

USTRUCT(BlueprintType)
struct FBossMonsterRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BossName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> BossClass; // 보스의 Blueprint 클래스

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDataTable> CheckListTable;
};
