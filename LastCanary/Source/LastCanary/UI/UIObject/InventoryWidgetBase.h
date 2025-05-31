// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "UI/UIObject/InventorySlotWidget.h"
#include "Inventory/InventoryComponentBase.h"
#include "InventoryWidgetBase.generated.h"

UCLASS(Abstract)
class LASTCANARY_API UInventoryWidgetBase : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    UInventoryComponentBase* InventoryComponent;

    /** 각 위젯에서 사용할 슬롯 위젯 클래스 (블루프린트에서 설정) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    /** 아이템 데이터 테이블 (자동으로 로드됨) */
    UPROPERTY(BlueprintReadOnly, Category = "Data")
    UDataTable* ItemDataTable;

    // ⭐ 공유 툴팁 관련 추가
    /** 공유 툴팁 위젯 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes")
    TSubclassOf<UItemTooltipWidget> TooltipWidgetClass;

    /** 공유 툴팁 위젯 인스턴스 */
    UPROPERTY()
    UItemTooltipWidget* SharedTooltipWidget;

    virtual void RefreshInventoryUI() PURE_VIRTUAL(UInventoryWIdgetBase::RefreshInventoryUI, return;);

    UFUNCTION()
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnInventoryChanged();

    UFUNCTION(BlueprintCallable)
    void SetInventoryComponent(UInventoryComponentBase * NewInventoryComponent);

    UInventoryComponentBase* GetInventoryComponent() const;

    // ⭐ 공유 툴팁 관리 함수들
    /** 슬롯에서 툴팁 표시 요청 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Tooltip")
    void ShowTooltipForSlot(const FBaseItemSlotData & ItemData, UWidget * SourceWidget);

    /** 툴팁 숨김 */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Tooltip")
    void HideTooltip();

    /** 툴팁이 현재 표시 중인지 확인 */
    UFUNCTION(BlueprintPure, Category = "Inventory|Tooltip")
    bool IsTooltipVisible() const;

protected:
    /** 슬롯 위젯 생성 헬퍼 함수 */
    UFUNCTION(BlueprintCallable)
    UInventorySlotWidget* CreateSlotWidget(int32 SlotIndex, const FBaseItemSlotData & SlotData);

    /** 툴팁 위치 업데이트 타이머 */
    FTimerHandle TooltipUpdateTimer;

    /** 툴팁 위치 업데이트 */
    void UpdateTooltipPosition();

    /** 공유 툴팁 위젯 생성 */
    void CreateSharedTooltipWidget();
};
