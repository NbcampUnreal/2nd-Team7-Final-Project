// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
#include "DataType/BaseItemSlotData.h"
#include "BackpackInventoryComponent.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API UBackpackInventoryComponent : public UInventoryComponentBase
{
	GENERATED_BODY()
	
public:
	UBackpackInventoryComponent();

    UFUNCTION(BlueprintCallable, Category = Inventory)
    virtual bool TryAddItemSlot(FName ItemRowName, int32 Amount);

    UFUNCTION(Blueprintcallable, Category = Inventory)
    virtual bool TryDecreaseItem(FName ItemRowName, int32 Amount);

    UFUNCTION(Blueprintcallable, Category = Inventory)
    virtual int32 GetItemCount(FName ItemRowName) const;

    UFUNCTION(BlueprintCallable, Category = Inventory)
    bool TrySwapItemSlots(int32 FromIndex, int32 ToIndex);

    UFUNCTION(Blueprintcallable, Category = Inventory)
    bool TryRemoveItemAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = Inventory)
    bool TryAddItem(AItemBase * ItemActor);

protected:
	virtual void BeginPlay() override;

	bool CanAddItem(AItemBase* ItemActor) override;
	bool TryStoreItem(AItemBase* ItemActor) override;
	void PostAddProcess() override;



};
