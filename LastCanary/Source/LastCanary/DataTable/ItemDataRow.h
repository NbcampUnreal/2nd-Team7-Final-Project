#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataRow.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 ItemID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ItemIcon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemPrice = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemDescription = FText::FromString(TEXT(""));

	// 추가로 넣은 부분(황주영)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack;
};
