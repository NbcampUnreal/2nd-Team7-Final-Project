#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Item/EquipmentItem/BackpackItem.h"
#include "ToolbarInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBackpackEquipped, const TArray<FBackpackSlotData>&, BackpackSlots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackpackUnequipped);

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

    // 가방 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Backpack")
    FOnBackpackEquipped OnBackpackEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Backpack")
    FOnBackpackUnequipped OnBackpackUnequipped;

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
    /** 현재 장착된 가방의 슬롯 데이터 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Backpack")
    TArray<FBackpackSlotData> GetCurrentBackpackSlots() const;

    /** 가방 슬롯 데이터 업데이트 (UI에서 호출) */
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    bool UpdateCurrentBackpackSlots(const TArray<FBackpackSlotData>& NewSlots);

    /** 가방에 아이템 추가 (UI에서 직접 호출용) */
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    bool AddItemToBackpack(FName ItemRowName, int32 Quantity, int32 BackpackSlotIndex);

    /** 가방에서 아이템 제거 (UI에서 직접 호출용) */
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    bool RemoveItemFromBackpack(int32 BackpackSlotIndex, int32 Quantity);

    /** 가방이 장착되어 있는지 확인 */
    UFUNCTION(BlueprintPure, Category = "Backapck|Equipment")
    bool HasBackpackEquipped() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Persistence")
    TArray<int32> GetAllBackpackItemIDs() const;

private:
    /** 가방 전용 장착 전처리 */
    void HandleBackpackEquip(int32 SlotIndex);

    /** 가방 전용 해제 전처리 */
    void HandleBackpackUnequip(int32 SlotIndex);

    /** 가방인지 확인 */
    bool IsBackpackItem(const FItemDataRow* ItemData) const;
    bool IsBackpackItem(FName ItemRowName) const;
    bool HasOtherEquippedItems() const;

    /** 가방메시 비가시화 RPC함수 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SetBackpackVisibility(bool bVisible);
    void Multicast_SetBackpackVisibility_Implementation(bool bVisible);

    /** 수집품인지 확인 */
    bool IsCollectibleItem(const FItemDataRow* ItemData) const;

public:
    /** 장착된 아이템의 내구도를 슬롯에 동기화 */
    UFUNCTION(BlueprintCallable, Category = "Toolbar|Sync")
    void SyncEquippedItemDurabilityToSlot();

    UFUNCTION()
    void SyncGunStateToSlot();

    UFUNCTION()
    void RestoreGunStateFromSlot(AGunBase* Gun, const FBaseItemSlotData& SlotData);

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
    bool Internal_DropEquippedItemAtSlot(int32 SlotIndex, int32 Quantity);

    /** 아이템 습득 시 플레이어 스테이트와 동기화 */
    void OnItemAcquired(const FName& ItemRowName);
    /** 아이템 드랍 시 플레이어 스테이트와 동기화 */
    void OnItemDropped(const FName& ItemRowName);

    //-----------------------------------------------------
    // 네트워크
    //-----------------------------------------------------
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // 장착한 장비이름을 UI로 전달하는 함수
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUpdateItemText(const FText& ItemName);
    void MulticastUpdateItemText_Implementation(const FText& ItemName);
};
