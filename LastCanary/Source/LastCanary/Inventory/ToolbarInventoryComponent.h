#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
#include "Inventory/BackpackManager.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Item/EquipmentItem/BackpackItem.h"
#include "ToolbarInventoryComponent.generated.h"

class ABaseCharacter;
struct FGameplayTag;

class UInventoryNetworkManager;
class UInventoryUIController;

UCLASS()
class LASTCANARY_API UToolbarInventoryComponent : public UInventoryComponentBase
{
    GENERATED_BODY()

public:
    UToolbarInventoryComponent();

protected:
    virtual void BeginPlay() override;

    //-----------------------------------------------------
    // 장착 시스템
    //-----------------------------------------------------

public:
    /** 장착된 아이템 컴포넌트 (일반 장비용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
    UChildActorComponent* EquippedItemComponent;

    /** 현재 장착된 슬롯 인덱스 */
    UPROPERTY(Replicated)
    int32 CurrentEquippedSlotIndex;

    /** 특정 메시에 아이템 설정 */
    void SetupEquippedItem(UChildActorComponent* ItemComponent, USkeletalMeshComponent* TargetMesh, FName SocketName, FItemDataRow* ItemData, FBaseItemSlotData* SlotData);

    FORCEINLINE int32 GetCurrentEquippedSlotIndex() { return CurrentEquippedSlotIndex; }
    void SetCurrentEquippedSlotIndex(int32 NewIndex);

    //-----------------------------------------------------
    // 인벤토리 오버라이드 구현
    //-----------------------------------------------------

public:
    virtual bool CanAddItem(AItemBase* ItemActor) override;
    virtual bool TryStoreItem(AItemBase* ItemActor) override;
    virtual void PostAddProcess() override;

    virtual bool TryAddItemSlot(FName ItemRowName, int32 Amount) override;
    virtual bool TryDecreaseItem(FName ItemRowName, int32 Amount) override;
    virtual int32 GetItemCount(FName ItemRowName) const override;
    virtual bool TrySwapItemSlots(int32 FromIndex, int32 ToIndex) override;
    virtual bool TryRemoveItemAtSlot(int32 SlotIndex) override;
    virtual bool TryAddItem(AItemBase* ItemActor) override;

    //-----------------------------------------------------
    // 툴바 특화 장비 기능
    //-----------------------------------------------------

public:
    UFUNCTION(BlueprintCallable, Category = "Toolbar|Equipment")
    void EquipItemAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Toolbar|Equipment")
    void UnequipCurrentItem();

    UFUNCTION(BlueprintPure, Category = "Toolbar|Equipment")
    AItemBase* GetCurrentEquippedItem() const;

    UFUNCTION(BlueprintPure, Category = "Toolbar|Equipment")
    int32 GetCurrentEquippedSlotIndex() const;

    FBaseItemSlotData* GetItemDataAtSlot(int32 SlotIndex);

    //-----------------------------------------------------
    // 장착 아이템 동기화
    //-----------------------------------------------------

public:
    /** 장착된 아이템의 내구도를 슬롯에 동기화 */
    UFUNCTION(BlueprintCallable, Category = "Toolbar|Sync")
    void SyncEquippedItemDurabilityToSlot();

    UFUNCTION()
    void SyncGunStateToSlot();

    UFUNCTION()
    void RestoreGunStateFromSlot(AGunBase* Gun, const FBaseItemSlotData& SlotData);

    //-----------------------------------------------------
    // 드랍 기능 (DropSystem으로 위임)
    //-----------------------------------------------------

public:
    UFUNCTION(BlueprintCallable, Category = "Toolbar|Drop")
    bool DropCurrentEquippedItem();

    UFUNCTION(Server, Reliable, Category = "Toolbar|Drop")
    void Server_DropItem(int32 SlotIndex, int32 Quantity);
    void Server_DropItem_Implementation(int32 SlotIndex, int32 Quantity);

    virtual bool TryDropItemAtSlot(int32 SlotIndex, int32 Quantity = 1) override;

    //-----------------------------------------------------
    // 백팩 매니저 시스템
    //-----------------------------------------------------

public:
    /** 백팩 매니저 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Managers")
    UBackpackManager* BackpackManager;

    /** UI 컨트롤러 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Managers")
    UInventoryUIController* UIController;

    // 가방 이벤트 (호환성 유지용)
    UPROPERTY(BlueprintAssignable, Category = "Backpack")
    FOnBackpackEquipped OnBackpackEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Backpack")
    FOnBackpackUnequipped OnBackpackUnequipped;

    /** 가방 관련 기능들 - BackpackManager로 위임 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Backpack")
    bool HasBackpackEquipped() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Backpack")
    bool AddItemToBackpack(FName ItemRowName, int32 Quantity, int32 BackpackSlotIndex = -1);

    /** 현재 장착된 가방의 슬롯 데이터 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Backpack")
    TArray<FBackpackSlotData> GetCurrentBackpackSlots() const;

    /** 가방 슬롯 데이터 업데이트 (UI에서 호출) */
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    bool UpdateCurrentBackpackSlots(const TArray<FBackpackSlotData>& NewSlots);

    /** 가방에서 아이템 제거 (UI에서 직접 호출용) */
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    bool RemoveItemFromBackpack(int32 BackpackSlotIndex, int32 Quantity);

    //-----------------------------------------------------
    // 백팩 드래그 & 드롭
    //-----------------------------------------------------

public:
    /** 가방 슬롯 간 스왑 */
    UFUNCTION(BlueprintCallable, Category = "Backpack|Operations")
    bool TrySwapBackpackSlots(int32 FromBackpackIndex, int32 ToBackpackIndex);

    /** 툴바 아이템을 가방으로 이동 */
    UFUNCTION(BlueprintCallable, Category = "Backpack|Operations")
    bool TryMoveToolbarItemToBackpack(int32 ToolbarIndex, int32 BackpackIndex);

    /** 가방 아이템을 툴바로 이동 */
    UFUNCTION(BlueprintCallable, Category = "Backpack|Operations")
    bool TryMoveBackpackItemToToolbar(int32 BackpackIndex, int32 ToolbarIndex);

    //-----------------------------------------------------
    // 데이터 지속성
    //-----------------------------------------------------

public:
    /** 인벤토리 슬롯들의 ItemID 배열 반환 (레벨 이동 시 사용) */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Persistence")
    TArray<int32> GetInventoryItemIDs() const;

    /** ItemID 배열로부터 인벤토리 복원 (레벨 이동 후 사용) */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Persistence")
    void SetInventoryFromItemIDs(const TArray<int32>& ItemIDs);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Persistence")
    TArray<int32> GetAllBackpackItemIDs() const;

    //-----------------------------------------------------
    // 네트워크 & UI
    //-----------------------------------------------------

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 장착한 장비이름을 UI로 전달하는 함수 */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUpdateItemText(const FText& ItemName);
    void MulticastUpdateItemText_Implementation(const FText& ItemName);

    //-----------------------------------------------------
    // 내부 구현 및 헬퍼
    //-----------------------------------------------------

protected:
    /** 매니저들 초기화 */
    void InitializeManagers();

    /** 가방 전용 장착 전처리 */
    void HandleBackpackEquip(int32 SlotIndex);

    /** 가방 전용 해제 전처리 */
    void HandleBackpackUnequip(int32 SlotIndex);

    /** 가방인지 확인 */
    bool IsBackpackItem(const FItemDataRow* ItemData) const;
    bool IsBackpackItem(FName ItemRowName) const;
    bool HasOtherEquippedItems() const;

    /** 수집품인지 확인 */
    bool IsCollectibleItem(const FItemDataRow* ItemData) const;

    /** 아이템 습득 및 드랍 시 플레이어 스테이트와 동기화 */
    void SyncInventoryToPlayerState();

    /** 가방메시 비가시화 RPC함수 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SetBackpackVisibility(bool bVisible);
    void Multicast_SetBackpackVisibility_Implementation(bool bVisible);

    /** 델리게이트 핸들러 */
    void OnBackpackEquippedHandler(const TArray<FBackpackSlotData>& BackpackSlots);
    void OnBackpackUnequippedHandler();
};
