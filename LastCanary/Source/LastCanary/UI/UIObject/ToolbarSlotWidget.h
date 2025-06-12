#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/InventorySlotWidget.h"
#include "ToolbarSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API UToolbarSlotWidget : public UInventorySlotWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TWeakObjectPtr<AItemBase> ItemActor;

	void SetItemActor(AItemBase* NewItemActor);
};
