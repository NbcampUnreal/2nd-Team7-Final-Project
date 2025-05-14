// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UInventoryComponentBase : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UInventoryComponentBase();

//protected:
//    // Called when the game starts
//    virtual void BeginPlay() override;

//public:
//    UPROPERTY(BlueprintAssignable, Category = "Inventory")
//    FOnInventoryUpdated OnInventoryUpdated;
//
//    // 인벤토리 슬롯 배열
//    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
//    TArray<FInventorySlot> InventorySlots;
//
//    // 인벤토리 슬롯 최대 개수
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
//    int32 MaxSlots;
//
//    UPROPERTY()
//    UDataTable* ItemDataTable;
//
//    // 아이템 추가(수량 지정)
//    UFUNCTION(BlueprintCallable, Category = Inventory)
//    virtual bool AddItem(FName ItemRowName, int32 Amount);
//
//    // 아이템 제거(수량 지정)
//    UFUNCTION(BlueprintCallable, Category = Inventory)
//    virtual bool RemoveItem(FName ItemRowName, int32 Amount);
//
//    // 아이템 보유량 조회
//    UFUNCTION(BlueprintCallable, Category = Inventory)
//    virtual int32 GetItemCount(FName ItemRowName) const;
//
//    // 인벤토리 슬롯끼리 아이템 스왑
//    UFUNCTION(BlueprintCallable, Category = Inventory)
//    bool SwapItemSlots(int32 FromIndex, int32 ToIndex);
//
//    // 아이템 제거
//    UFUNCTION(BlueprintCallable, Category = Inventory)
//    bool RemoveItemAtSlot(int32 SlotIndex);
//
//    // 툴팁 위젯 관련
//    void ShowTooltipForItem(const FInventoryItemData& ItemData, UWidget* TargetWidget);
//
//    void HideTooltip();
//
//    UFUNCTION(BlueprintCallable, Category = Inventory)
//    bool TryAddItem(AItemBase* ItemActor);
//
//protected:
//    virtual bool PreAddCheck(AItemBase* ItemActor) PURE_VIRTUAL(UBaseInventoryComponent::PreAddCheck, return false;);
//    virtual bool StoreItem(AItemBase * ItemActor) PURE_VIRTUAL(UBaseInventoryComponent::StoreItem, return false;);
//    virtual void PostAddProcess(AItemBase * ItemActor) PURE_VIRTUAL(UBaseInventoryComponent::PostAddProcess, return;);
//
//
//
//
//private:
//    UPROPERTY()
//    UItemTooltipWidget* TooltipWidget;
//
//    UPROPERTY(EditDefaultsOnly)
//    TSubclassOf<UItemTooltipWidget> TooltipWidgetClass;
};
