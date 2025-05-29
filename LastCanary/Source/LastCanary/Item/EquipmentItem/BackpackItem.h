#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Inventory/BackpackInventoryComponent.h"
#include "BackpackItem.generated.h"


UCLASS()
class LASTCANARY_API ABackpackItem : public AEquipmentItemBase
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	UBackpackInventoryComponent* BackpackInventoryComponent;

public:
	// TODO : 인벤토리 컴포넌트와 상호작용하는 함수 추가 필요
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UBackpackInventoryComponent* GetBackpackInventoryComponent() const;

	void CopyInventoryData(UBackpackInventoryComponent* NewBackpackInventoryComponent);
};
