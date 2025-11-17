#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataType/LoadingInfo.h"
#include "LoadingDataRow.generated.h"

USTRUCT(BlueprintType)
struct LASTCANARY_API FLoadingDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLoadingInfo LoadingInfo;
};
