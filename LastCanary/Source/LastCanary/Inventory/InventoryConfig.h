#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryConfig.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API UInventoryConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
    /** 기본 아이템 이름 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Items")
    FName DefaultItemRowName = FName("Default");

    /** 가방 기본 슬롯 수 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Backpack")
    int32 DefaultBackpackSlots = 20;

    /** 드랍 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
    float DropDistanceFromPlayer = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
    float MinDropHeight = 80.0f;

    /** 기본 내구도 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items")
    float DefaultDurability = 100.0f;

    /** 기본 수량 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items")
    int32 DefaultQuantity = 1;
};
