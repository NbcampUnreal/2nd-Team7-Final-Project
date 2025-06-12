#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/UIObject/ItemTooltipWidget.h"
#include "Item/ItemBase.h"
#include "InventoryComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeightChanged, float, NewWeight, float, WeightDifference);

class ULCUserWidgetBase;
class UItemTooltipWidget;
class ABaseCharacter;
class UItemSpawnerComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTCANARY_API UInventoryComponentBase : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponentBase();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Internal")
    void InitializeSlots();

protected:
    virtual void BeginPlay() override;

    /** 아이템 스포너 컴포넌트 */
    UPROPERTY()
    UItemSpawnerComponent* ItemSpawner;

public:
    /** 캐릭터로부터 ItemSpawner 참조 가져오기 */
    UItemSpawnerComponent* GetItemSpawner() const;

protected:
    /** 캐싱된 소유자 캐릭터 */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Cache")
    ABaseCharacter* CachedOwnerCharacter;

    /** 소유자 캐릭터 캐싱 */
    void CacheOwnerCharacter();

    /** 소유자 캐릭터가 유효한지 확인 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Cache")
    bool IsOwnerCharacterValid() const;

public:
    /** 캐싱된 소유자 캐릭터 반환 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Cache")
    ABaseCharacter* GetCachedOwnerCharacter() const;

    //-----------------------------------------------------
    // 인벤토리 데이터
    //-----------------------------------------------------

public:
    /** 인벤토리 내 아이템 슬롯 데이터 배열 */
    UPROPERTY(ReplicatedUsing = OnRep_ItemSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
    TArray<FBaseItemSlotData> ItemSlots;

    /** 인벤토리의 최대 슬롯 수 */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory|Data")
    int32 MaxSlots;

    /** 아이템 정보 참조용 데이터 테이블 */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
    UDataTable* ItemDataTable;

    //-----------------------------------------------------
    // 이벤트 및 델리게이트
    //-----------------------------------------------------

    /** 인벤토리 상태가 변경될 때 호출되는 델리게이트 */
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryUpdated OnInventoryUpdated;

    /** 아이템 슬롯 데이터가 복제될 때 호출 */
    UFUNCTION()
    void OnRep_ItemSlots();

    //-----------------------------------------------------
    // 외부 인터페이스 함수 (공개 API)
    //-----------------------------------------------------

    /** 아이템 데이터를 기반으로 인벤토리에 아이템 추가 시도 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Operations")
    virtual bool TryAddItemSlot(FName ItemRowName, int32 Amount) PURE_VIRTUAL(UInventoryComponentBase::TryAddItemSlot, return false;);

    /** 특정 아이템의 수량 감소 시도 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Operations")
    virtual bool TryDecreaseItem(FName ItemRowName, int32 Amount) PURE_VIRTUAL(UInventoryComponentBase::TryDecreaseItem, return false;);

    /** 특정 아이템의 현재 보유 수량 확인 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Query")
    virtual int32 GetItemCount(FName ItemRowName) const PURE_VIRTUAL(UInventoryComponentBase::GetItemCount, return 0;);

    FItemDataRow* GetItemRowByName(FName RowName);

    int32 GetMaxSlots() const;

    /** 두 슬롯 간 아이템 교환 시도 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Operations")
    virtual bool TrySwapItemSlots(int32 FromIndex, int32 ToIndex) PURE_VIRTUAL(UInventoryComponentBase::TrySwapItemSlots, return false;);

    /** 특정 슬롯의 아이템 제거 시도 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Operations")
    virtual bool TryRemoveItemAtSlot(int32 SlotIndex) PURE_VIRTUAL(UInventoryComponentBase::TryRemoveItemAtSlot, return false;);

    /** 실제 아이템 액터를 인벤토리에 추가 시도 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Operations")
    virtual bool TryAddItem(AItemBase* ItemActor) PURE_VIRTUAL(UInventoryComponentBase::TryAddItem, return false;);

    //-----------------------------------------------------
    // UI 관련 함수 (임시, UI 매니저로 이동 예정)
    //-----------------------------------------------------

    /** 아이템 데이터에 대한 툴팁 표시 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void ShowTooltipForItem(const FBaseItemSlotData& ItemData, UWidget* TargetWidget);

    /** 툴팁 숨김 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    void HideTooltip();

public:
    //-----------------------------------------------------
    // 아이템 드랍 기능 (RPC 패턴 적용)
    //-----------------------------------------------------

    /** 특정 슬롯의 아이템을 캐릭터 앞에 드랍 (클라이언트용) */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Operations")
    virtual bool TryDropItemAtSlot(int32 SlotIndex, int32 Quantity = 1);

    /** 특정 아이템을 캐릭터 앞에 드랍 (클라이언트용) */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Operations")
    virtual bool TryDropItem(FName ItemRowName, int32 Quantity = 1);

    /** 서버에서 특정 슬롯 아이템 드랍 */
    UFUNCTION(Server, Reliable, Category = "Inventory|Operations")
    void Server_TryDropItemAtSlot(int32 SlotIndex, int32 Quantity);
    void Server_TryDropItemAtSlot_Implementation(int32 SlotIndex, int32 Quantity);

    /** 서버에서 특정 아이템 드랍 */
    UFUNCTION(Server, Reliable, Category = "Inventory|Operations")
    void Server_TryDropItem(FName ItemRowName, int32 Quantity);
    void Server_TryDropItem_Implementation(FName ItemRowName, int32 Quantity);

protected:
    /** 실제 슬롯 드랍 로직 (서버에서만 실행) */
    virtual bool Internal_TryDropItemAtSlot(int32 SlotIndex, int32 Quantity);

    /** 실제 아이템 드랍 로직 (서버에서만 실행) */
    virtual bool TryDropItem_Internal(FName ItemRowName, int32 Quantity);

    /** 드랍 위치 계산 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Utility")
    FVector CalculateDropLocation() const;


    //-----------------------------------------------------
    // 무게 관리 시스템
    //-----------------------------------------------------
public:
    /** 무게 변경 델리게이트 */
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Weight")
    FOnWeightChanged OnWeightChanged;

    /** 현재 총 무게 반환 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Weight")
    float GetTotalWeight() const { return CurrentTotalWeight; }

protected:
    /** 현재 총 무게 */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Weight")
    float CurrentTotalWeight = 0.0f;

    /** 무게 즉시 계산 및 갱신 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Weight")
    virtual void UpdateWeight();

    /** 아이템의 실제 무게 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Weight")
    float GetItemWeight(FName ItemRowName) const;

    //-----------------------------------------------------
    // 네트워크 기능
    //-----------------------------------------------------
public:
    /** 리플리케이션 속성 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    //-----------------------------------------------------
    // 내부 동작 함수 (하위 클래스 구현)
    //-----------------------------------------------------

    /** 아이템 추가 가능 여부 확인 */
    virtual bool CanAddItem(AItemBase* ItemActor) PURE_VIRTUAL(UInventoryComponentBase::CanAddItem, return false;);

    /** 아이템 저장 처리 */
    virtual bool TryStoreItem(AItemBase* ItemActor) PURE_VIRTUAL(UInventoryComponentBase::TryStoreItem, return false;);

    /** 아이템 추가 후 처리 */
    virtual void PostAddProcess() PURE_VIRTUAL(UInventoryComponentBase::PostAddProcess, return;);

    //-----------------------------------------------------
    // 빈 슬롯 아이템 관리
    //-----------------------------------------------------
protected:
    /** Default 아이템 Row Name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Default")
    FName DefaultItemRowName = FName("Default");

public:
    /** Default 아이템인지 확인 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Utility")
    bool IsDefaultItem(FName ItemRowName) const;

    /** 빈 슬롯을 Default 아이템으로 설정 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Utility")
    void SetSlotToDefault(int32 SlotIndex);

protected:
    /** ItemRowName을 ItemID로 변환 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Utility")
    int32 GetItemIDFromRowName(FName ItemRowName) const;

    /** ItemID를 ItemRowName으로 변환 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Utility")
    FName GetItemRowNameFromID(int32 ItemID) const;

    void ClearInventorySlots();
};
