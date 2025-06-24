#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/BaseItemSlotData.h"
#include "InventoryUIController.generated.h"

class UInventoryComponentBase;
class UInventoryMainWidget;
class ULCUIManager;

/**
 * 인벤토리 UI 관련 로직을 담당하는 컨트롤러
 */
UCLASS(BlueprintType)
class LASTCANARY_API UInventoryUIController : public UObject
{
    GENERATED_BODY()

public:
    UInventoryUIController();

    /** UI 컨트롤러 초기화 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void Initialize(UInventoryComponentBase* InOwnerInventory);

    //-----------------------------------------------------
    // UI 업데이트
    //-----------------------------------------------------

    /** 인벤토리 UI 전체 새로고침 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void RefreshInventoryUI();

    /** 특정 슬롯 UI 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void UpdateSlotUI(int32 SlotIndex);

    /** 장착된 아이템 텍스트 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void UpdateEquippedItemText(const FText& ItemName);

    /** 무게 정보 UI 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void UpdateWeightUI(float CurrentWeight);

    //-----------------------------------------------------
    // 툴팁 관리
    //-----------------------------------------------------

    /** 아이템 툴팁 표시 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void ShowItemTooltip(const FBaseItemSlotData& ItemData, UWidget* TargetWidget);

    /** 툴팁 숨김 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void HideTooltip();

    //-----------------------------------------------------
    // 가방 UI
    //-----------------------------------------------------

    /** 가방 UI 활성화 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void ShowBackpackUI(const TArray<FBackpackSlotData>& BackpackSlots);

    /** 가방 UI 비활성화 */
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void HideBackpackUI();

private:
    /** PlayerController 가져오기 */
    APlayerController* GetOwnerPlayerController() const;

    /** 입력 모드 설정 */
    void SetInputModeGameAndUI();
    void SetInputModeGameOnly();

    //-----------------------------------------------------
    // 네트워크 UI 업데이트
    //-----------------------------------------------------

public:
    /** 모든 클라이언트에 UI 업데이트 전송 */
    UFUNCTION(NetMulticast, Reliable, Category = "Inventory UI")
    void Multicast_UpdateItemText(const FText& ItemName);
    void Multicast_UpdateItemText_Implementation(const FText& ItemName);

    /** 특정 클라이언트에 UI 업데이트 전송 */
    UFUNCTION(Client, Reliable, Category = "Inventory UI")
    void Client_RefreshUI();
    void Client_RefreshUI_Implementation();

private:
    /** 소유자 인벤토리 참조 */
    UPROPERTY()
    UInventoryComponentBase* OwnerInventory;

    /** UI 매니저 캐시 */
    UPROPERTY()
    ULCUIManager* CachedUIManager;

    /** 인벤토리 위젯 캐시 */
    UPROPERTY()
    UInventoryMainWidget* CachedInventoryWidget;

    /** UI 매니저 가져오기 */
    ULCUIManager* GetUIManager();

    /** 인벤토리 위젯 가져오기 */
    UInventoryMainWidget* GetInventoryWidget();

    /** 로컬 플레이어 확인 */
    bool IsLocalPlayer() const;
};
