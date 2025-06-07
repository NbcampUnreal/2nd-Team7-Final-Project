#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
#include "ToolbarInventoryComponent.generated.h"

class ABaseCharacter;
struct FGameplayTag;

UCLASS()
class LASTCANARY_API UToolbarInventoryComponent : public UInventoryComponentBase
{
    GENERATED_BODY()

public:
    UToolbarInventoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    /** 장착된 아이템 컴포넌트 (일반 장비용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
    UChildActorComponent* EquippedItemComponent;

    /** 특정 메시에 아이템 설정 */
    void SetupEquippedItem(UChildActorComponent* ItemComponent, USkeletalMeshComponent* TargetMesh, FName SocketName, FItemDataRow* ItemData, FBaseItemSlotData* SlotData);

    /** 현재 장착된 슬롯 인덱스 */
    UPROPERTY(Replicated)
    int32 CurrentEquippedSlotIndex;

public:
    FORCEINLINE int32 GetCurrentEquippedSlotIndex() { return CurrentEquippedSlotIndex; }
    void SetCurrentEquippedSlotIndex(int32 NewIndex);
    //-----------------------------------------------------
    // 인벤토리 오버라이드
    //-----------------------------------------------------
    virtual bool CanAddItem(AItemBase* ItemActor) override;
    virtual bool TryStoreItem(AItemBase* ItemActor) override;
    virtual void PostAddProcess() override;

public:
    //-----------------------------------------------------
    // 인벤토리 공개 API
    //-----------------------------------------------------
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
    int32 GetCurrentEquippedSlotIndex() const;

    FBaseItemSlotData* GetItemDataAtSlot(int32 SlotIndex);

public:
    /** 장착된 아이템의 내구도를 슬롯에 동기화 */
    UFUNCTION(BlueprintCallable, Category = "Toolbar|Sync")
    void SyncEquippedItemDurabilityToSlot();

    //-----------------------------------------------------
    // 드랍 기능
    //-----------------------------------------------------
    UFUNCTION(Server, Reliable)
    void Server_DropEquippedItemAtSlot(int32 SlotIndex, int32 Quantity);
    void Server_DropEquippedItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Toolbar|Drop")
    bool DropCurrentEquippedItem();

    UFUNCTION(Server, Reliable, Category = "Toolbar|Drop")
    void Server_DropCurrentEquippedItem();
    void Server_DropCurrentEquippedItem_Implementation();

    virtual bool TryDropItemAtSlot(int32 SlotIndex, int32 Quantity = 1) override;

    UFUNCTION(Server, Reliable, Category = "Toolbar|Drop")
    void Server_DropItemAtSlot(int32 SlotIndex, int32 Quantity);
    void Server_DropItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity);

private:
    bool Internal_DropCurrentEquippedItem();

private:
    bool Internal_DropEquippedItemAtSlot(int32 SlotIndex, int32 Quantity);

    //-----------------------------------------------------
    // 네트워크
    //-----------------------------------------------------
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
