#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataType/MapInfo.h"
#include "MapDataRow.generated.h"

USTRUCT(BlueprintType)
struct FMapDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 MapID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMapInfo MapInfo;
};