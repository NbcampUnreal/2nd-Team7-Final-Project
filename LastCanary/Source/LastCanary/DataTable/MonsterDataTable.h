#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MonsterDataTable.generated.h"

class ABaseMonsterCharacter;

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
};
