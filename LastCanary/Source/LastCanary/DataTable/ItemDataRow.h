#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
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

	// 추가로 넣은 부분
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AItemBase> ItemActorClass;
};
