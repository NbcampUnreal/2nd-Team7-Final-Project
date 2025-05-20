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

    /** 캐싱된 소유자 캐릭터 */
    UPROPERTY()
    ABaseCharacter* CachedOwnerCharacter;

    //-----------------------------------------------------
    // 인벤토리 내부 처리 (오버라이드)
    //-----------------------------------------------------

    /** 아이템을 툴바에 추가할 수 있는지 확인 */
    virtual bool CanAddItem(AItemBase* ItemActor) override;

    /** 아이템을 툴바에 저장 */
    virtual bool TryStoreItem(AItemBase* ItemActor) override;

    /** 아이템 추가 후 처리 */
    virtual void PostAddProcess() override;

    //-----------------------------------------------------
    // 장비 관련 헬퍼 함수
    //-----------------------------------------------------

    /** 특정 소켓이 사용 가능한지 확인 */
    bool IsSocketAvailable(ABaseCharacter* OwnerChar, FName SocketName) const;

    /** 특정 태그의 아이템이 장착되어 있는지 확인 */
    bool IsEquipped(ABaseCharacter* OwnerChar, const FGameplayTag& Tag) const;

    /** 장착된 아이템 등록 */
    void RegisterEquippedItem(AItemBase* Item, FName SocketName);

    /** 장착된 아이템 등록 해제 */
    void UnregisterEquippedItem(FName SocketName);

public:
    //-----------------------------------------------------
    // 인벤토리 공개 인터페이스 (오버라이드)
    //-----------------------------------------------------

    /** 아이템을 툴바에 추가 (데이터 기반) */
    virtual bool TryAddItemSlot(FName ItemRowName, int32 Amount) override;

    /** 툴바에서 아이템 수량 감소 */
    virtual bool TryDecreaseItem(FName ItemRowName, int32 Amount) override;

    /** 툴바에서 특정 아이템의 수량 확인 */
    virtual int32 GetItemCount(FName ItemRowName) const override;

    /** 두 슬롯 간 아이템 교환 */
    virtual bool TrySwapItemSlots(int32 FromIndex, int32 ToIndex) override;

    /** 특정 슬롯의 아이템 제거 */
    virtual bool TryRemoveItemAtSlot(int32 SlotIndex) override;

    /** 실제 아이템 액터를 툴바에 추가 */
    virtual bool TryAddItem(AItemBase* ItemActor) override;

    //-----------------------------------------------------
    // 툴바 특화 기능
    //-----------------------------------------------------

    /** 특정 슬롯의 아이템 장착 */
    UFUNCTION(BlueprintCallable, Category = "Toolbar|Equipment")
    bool EquipItemAtSlot(int32 SlotIndex);

    /** 아이템 데이터 테이블 참조 획득 */
    UFUNCTION(BlueprintPure, Category = "Toolbar|Data")
    UDataTable* GetItemDataTable() const;

    /** 모든 클라이언트에 아이템 장착 시각화 전파 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_HandleItemPickup(AItemBase* ItemToDestroy, FName ItemRowName, FName SocketName);
    void Multicast_HandleItemPickup_Implementation(AItemBase* ItemToDestroy, FName ItemRowName, FName SocketName);

    //-----------------------------------------------------
    // 네트워크 기능
    //-----------------------------------------------------

    /** 리플리케이션 속성 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 미구현 또는 향후 구현 예정 기능
    /*
    UFUNCTION(BlueprintCallable, Category = "Toolbar|Equipment")
    bool EquipItem(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Toolbar|Equipment")
    void UnequipCurrentItem();

    UFUNCTION(BlueprintCallable, Category = "Toolbar|Equipment")
    void UseEquippedItem();

    UFUNCTION(BlueprintPure, Category = "Toolbar|Equipment")
    FBaseItemSlotData* GetEquippedItem();

    UPROPERTY(ReplicatedUsing = OnRepEquippedSlotIndex)
    int32 CurrentEquippedIndex = INDEX_NONE;

    UFUNCTION()
    void OnRepEquippedSlotIndex();
    */
};