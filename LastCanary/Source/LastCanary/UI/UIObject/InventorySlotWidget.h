#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "UI/UIObject/ItemTooltipWidget.h"
#include "DataType/BaseItemSlotData.h"
#include "Inventory/InventoryComponentBase.h"
#include "InventorySlotWidget.generated.h"

/**
 * 인벤토리 슬롯 UI 위젯 클래스
 * 개별 인벤토리 슬롯을 표시하고 상호작용을 처리합니다.
 */
UCLASS()
class LASTCANARY_API UInventorySlotWidget : public ULCUserWidgetBase
{
    GENERATED_BODY()

public:
    //-----------------------------------------------------
    // 바인딩된 UI 컴포넌트
    //-----------------------------------------------------

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ItemNameText;

    UPROPERTY(meta = (BindWidget))
    class UImage* ItemIconImage;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ItemQuantityText;

    UPROPERTY(meta = (BindWidget))
    class UBorder* SlotBorder;

    //-----------------------------------------------------
    // 슬롯 데이터
    //-----------------------------------------------------

    /** 슬롯에 표시할 아이템 데이터 */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
    FBaseItemSlotData ItemData;

    /** 아이템 정보를 가져올 데이터 테이블 */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
    UDataTable* ItemDataTable;

    /** 슬롯의 인덱스 번호 */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
    int32 SlotIndex = -1;

    /** 연결된 인벤토리 컴포넌트 참조 */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
    UInventoryComponentBase* InventoryComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    FLinearColor EquippedBorderColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.3f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    FLinearColor NormalBorderColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.3f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    FLinearColor EmptyBorderColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.3f);

    //-----------------------------------------------------
    // 툴팁 관련
    //-----------------------------------------------------

    /** 툴팁 위젯 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Tooltip")
    TSubclassOf<UItemTooltipWidget> TooltipWidgetClass;

    /** 툴팁 위젯 인스턴스 */
    UPROPERTY()
    UItemTooltipWidget* ItemTooltipWidget;

    //-----------------------------------------------------
    // 데이터 설정 함수
    //-----------------------------------------------------

    /** 슬롯의 아이템 데이터 설정 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Setup")
    void SetItemData(const FBaseItemSlotData& InItemSlotData, UDataTable* InItemDataTable);

    /** 연결된 인벤토리 컴포넌트 설정 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Setup")
    void SetInventoryComponent(UInventoryComponentBase* InInventoryComponent);

    //-----------------------------------------------------
    // UI 업데이트 및 상호작용
    //-----------------------------------------------------

    /** 슬롯 UI 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
    virtual void UpdateSlotUI();

    /** 아이템 사용 버튼 클릭 처리 */
    UFUNCTION()
    void OnUseButtonClicked();

    /** 툴팁 표시 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Tooltip")
    void ShowTooltip();

    /** 툴팁 숨김 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Tooltip")
    void HideTooltip();

    //-----------------------------------------------------
    // 마우스 상호작용 오버라이드
    //-----------------------------------------------------

    /** 마우스 버튼 클릭 처리 */
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    /** 마우스 진입 처리 */
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    /** 마우스 이탈 처리 */
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    /** 드래그 시작 처리 */
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

    /** 드롭 처리 */
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
    /** 툴팁 업데이트 타이머 */
    FTimerHandle TooltipUpdateTimer;

    /** 툴팁 위치 업데이트 */
    void UpdateTooltipPosition();

    /** 보더 색상 업데이트 */
    void UpdateBorderColor();

    // TODO : 추후 구현 예정인 블루프린트 이벤트 (현재 미사용)
    /*
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Events")
    void OnSlotHovered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Events")
    void OnSlotUnhovered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Events")
    void OnSlotSelected();

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Events")
    void OnSlotDragStarted();
    */
};