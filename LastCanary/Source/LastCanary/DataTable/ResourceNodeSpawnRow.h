#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ResourceNodeSpawnRow.generated.h"

class AResourceNode;
USTRUCT(BlueprintType)
struct FResourceNodeSpawnRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResourceNode")
    TSubclassOf<AResourceNode> ResourceNodeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ResourceNode", meta = (ClampMin = 0.0))
    float SpawnProbability = 10.0f;
};