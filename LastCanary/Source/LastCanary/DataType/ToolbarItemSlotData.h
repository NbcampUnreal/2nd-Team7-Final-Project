#pragma once

#include "CoreMinimal.h"
#include "DataType/BaseItemSlotData.h"
#include "ToolbarItemSlotData.generated.h"

class AItemBase;

USTRUCT(BlueprintType)
struct FToolbarItemSlotData : public FBaseItemSlotData
{
	GENERATED_BODY()


	FToolbarItemSlotData()
		: FBaseItemSlotData()
	{
	}
};
