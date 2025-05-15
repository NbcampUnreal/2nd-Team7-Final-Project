#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataType/ItemSlot.h"
#include "UI/UIObject/ItemTooltipWidget.h"
#include "InventoryComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

struct FInventoryItemData;
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
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
    TArray<FItemSlot> ItemSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
    int32 MaxSlots;

    UPROPERTY()
    UDataTable* ItemDataTable;

    UFUNCTION(BlueprintCallable, Category = Inventory)
    virtual bool AddItem(FName ItemRowName, int32 Amount);

    UFUNCTION(Blueprintcallable, Category = Inventory)
    virtual bool DecreaseItem(FName ItemRowName, int32 Amount);

    UFUNCTION(Blueprintcallable, Category = Inventory)
    virtual int32 GetItemCount(FName ItemRowName) const;

    UFUNCTION(BlueprintCallable, Category = Inventory)
    bool SwapItemSlots(int32 FromIndex, int32 ToIndex);

    UFUNCTION(Blueprintcallable, Category = Inventory)
    bool RemoveItemAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = Inventory)
    bool TryAddItem(AItemBase* ItemActor);

    // 툴팁 함수들은 UI매니저에서 관리할 듯?(아마도 지워야할지도, 보류중)
    void ShowTooltipForItem(const FInventoryItemData& ItemData, UWidget* TargetWidget);

    void HideTooltip();

protected:
    virtual bool PreAddCheck(AItemBase* ItemActot) PURE_VIRTUAL(UInventoryComponentBase::PreAddCheck, return false;);
    virtual bool StoreItem(AItemBase* ItemActor) PURE_VIRTUAL(UInventoryCompontnentBase::StoreItem, return false;);
    virtual void PostAddProcess(AItemBase* ItemActor) PURE_VIRTUAL(UInventoryComponentBase::PostAddProcess, return;);
};
