#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
#include "ToolbarInventoryComponent.generated.h"

class ABaseCharacter;
struct FGameplayTag;

/**
 * 툴바 인벤토리 컴포넌트 클래스
 * 캐릭터가 빠르게 접근하고 사용할 수 있는 장비 아이템을 관리합니다.
 */
UCLASS()
class LASTCANARY_API UToolbarInventoryComponent : public UInventoryComponentBase
{
    GENERATED_BODY()

public:
    /** 생성자 */
    UToolbarInventoryComponent();

protected:
    /** 컴포넌트 초기화 */
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
    UChildActorComponent* EquippedItemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
    UChildActorComponent* EquippedBackpackComponent;

    UPROPERTY(Replicated)
    int32 CurrentEquippedSlotIndex;

    //-----------------------------------------------------
    // 인벤토리 내부 처리 (오버라이드)
    //-----------------------------------------------------

    virtual bool CanAddItem(AItemBase* ItemActor) override;
    virtual bool TryStoreItem(AItemBase* ItemActor) override;
    virtual void PostAddProcess() override;

    //-----------------------------------------------------
    // 인벤토리 공개 인터페이스 (오버라이드)
    //-----------------------------------------------------

public:
    virtual bool TryAddItemSlot(FName ItemRowName, int32 Amount) override;
    virtual bool TryDecreaseItem(FName ItemRowName, int32 Amount) override;
    virtual int32 GetItemCount(FName ItemRowName) const override;
    virtual bool TrySwapItemSlots(int32 FromIndex, int32 ToIndex) override;
    virtual bool TryRemoveItemAtSlot(int32 SlotIndex) override;
    virtual bool TryAddItem(AItemBase* ItemActor) override;

    //-----------------------------------------------------
    // 툴바 특화 기능
    //-----------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Toolbar|Equipment")
    void EquipItemAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Toolbar|Equipment")
    void UnequipCurrentItem();

    UFUNCTION(BlueprintPure, Category = "Toolbar|Equipment")
    AItemBase* GetCurrentEquippedItem() const;

    UFUNCTION(BlueprintPure, Category = "Toolbar|Equipment")
    AItemBase* GetCurrentEquippedBackpack() const;

    FBaseItemSlotData* GetItemDataAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintPure, Category = "Toolbar|Equipment")
    int32 GetCurrentEquippedSlotIndex() const;

    //-----------------------------------------------------
    // 툴바 특화 드랍 기능
    //-----------------------------------------------------

private:
    /** 실제 드랍 로직 (서버에서만 실행) */
    bool Internal_DropCurrentEquippedItem();

public:
    /** 현재 장착된 아이템 드랍 (클라이언트용) */
    UFUNCTION(BlueprintCallable, Category = "Toolbar|Drop")
    bool DropCurrentEquippedItem();

    /** 서버에서 현재 장착된 아이템 드랍 */
    UFUNCTION(Server, Reliable, Category = "Toolbar|Drop")
    void Server_DropCurrentEquippedItem();
    void Server_DropCurrentEquippedItem_Implementation();

    /** 특정 슬롯 아이템 드랍 (클라이언트용) */
    virtual bool TryDropItemAtSlot(int32 SlotIndex, int32 Quantity = 1) override;

    /** 서버에서 특정 슬롯 아이템 드랍 */
    UFUNCTION(Server, Reliable, Category = "Toolbar|Drop")
    void Server_DropItemAtSlot(int32 SlotIndex, int32 Quantity);
    void Server_DropItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity);

    //-----------------------------------------------------
    // 네트워크 기능
    //-----------------------------------------------------

    /** 리플리케이션 속성 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
