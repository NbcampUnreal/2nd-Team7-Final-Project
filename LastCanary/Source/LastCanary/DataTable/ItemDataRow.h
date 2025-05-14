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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSellInShop = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBuy = true;
};
