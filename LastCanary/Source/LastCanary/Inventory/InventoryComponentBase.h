#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/UIObject/ItemTooltipWidget.h"
#include "Item/ItemBase.h"
#include "InventoryComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

class ULCUserWidgetBase;
class UItemTooltipWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UInventoryComponentBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponentBase();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(ReplicatedUsing = OnRep_ItemSlots, VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
    TArray<FBaseItemSlotData> ItemSlots;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
    int32 MaxSlots;

    UPROPERTY()
    UDataTable* ItemDataTable;

    virtual bool TryAddItemSlot(FName ItemRowName, int32 Amount) PURE_VIRTUAL(UInventoryComponentBase::TryAddItemSlot, return false;);
    virtual bool TryDecreaseItem(FName ItemRowName, int32 Amount) PURE_VIRTUAL(UInventoryComponentBase::TryDecreaseItem, return false;);
    virtual int32 GetItemCount(FName ItemRowName) const PURE_VIRTUAL(UInventoryComponentBase::GetItemCount, return 0;);
    bool TrySwapItemSlots(int32 FromIndex, int32 ToIndex) PURE_VIRTUAL(UInventoryComponentBase::TrySwapItemSlots, return false;);
    bool TryRemoveItemAtSlot(int32 SlotIndex) PURE_VIRTUAL(UInventoryComponentBase::TryRemoveItemAtSlot, return false;);
    bool TryAddItem(AItemBase* ItemActor) PURE_VIRTUAL(UInventoryComponentBase::TryAddItem, return false;);

    // 툴팁 함수들은 UI매니저에서 관리할 듯?(아마도 지워야할지도, 보류중)
    void ShowTooltipForItem(const FBaseItemSlotData& ItemData, UWidget* TargetWidget);

    void HideTooltip();

    UFUNCTION()
    void OnRep_ItemSlots();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual bool CanAddItem(AItemBase* ItemActot) PURE_VIRTUAL(UInventoryComponentBase::PreAddCheck, return false;);
    virtual bool TryStoreItem(AItemBase* ItemActor) PURE_VIRTUAL(UInventoryCompontnentBase::StoreItem, return false;);
    virtual void PostAddProcess() PURE_VIRTUAL(UInventoryComponentBase::PostAddProcess, return;);
};
