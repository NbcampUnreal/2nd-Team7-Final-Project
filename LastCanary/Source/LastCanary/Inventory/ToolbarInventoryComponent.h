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

public:
    //-----------------------------------------------------
    // 인벤토리 공개 인터페이스 (오버라이드)
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
    AItemBase* GetCurrentEquippedBackpack() const;

    FBaseItemSlotData* GetItemDataAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintPure, Category = "Toolbar|Equipment")
    int32 GetCurrentEquippedSlotIndex() const;

    //-----------------------------------------------------
    // 네트워크 기능
    //-----------------------------------------------------

    /** 리플리케이션 속성 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
