// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
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

protected:
	bool CanAddItem(AItemBase* ItemActor) override;
	bool TryStoreItem(AItemBase* ItemActor) override;
	void PostAddProcess() override;



};
