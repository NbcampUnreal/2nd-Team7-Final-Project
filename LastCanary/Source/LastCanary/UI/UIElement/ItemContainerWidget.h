// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/InventoryWidgetBase.h"
#include "Item/ItemContainer.h"
#include "ItemContainerWidget.generated.h"

class UGridPanel;
class UTextBlock;
class UButton;
class UItemContainerSlotWidget;
class UToolbarInventoryWidget;
class UScrollBox;
class AItemContainer;
class UToolbarInventoryComponent;

UCLASS()
class LASTCANARY_API UItemContainerWidget : public UInventoryWidgetBase
{
    GENERATED_BODY()

public:
    //-----------------------------------------------------
    // 바인딩된 UI 컴포넌트
    //-----------------------------------------------------

    UPROPERTY(meta = (BindWidget))
    UGridPanel* ContainerSlotPanel;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ContainerNameText;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* ContainerScrollBox;

    //-----------------------------------------------------
    // 위젯 클래스 설정
    //-----------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes")
    TSubclassOf<UItemContainerSlotWidget> ContainerSlotWidgetClass;

    //-----------------------------------------------------
    // 컨테이너 데이터
    //-----------------------------------------------------

protected:
    /** 현재 연결된 컨테이너 */
    UPROPERTY(BlueprintReadOnly, Category = "Container")
    AItemContainer* CurrentContainer;

    //-----------------------------------------------------
    // 레이아웃 설정
    //-----------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    int32 ColumnsPerRow = 5;

public:
    //-----------------------------------------------------
    // 엔진 오버라이드
    //-----------------------------------------------------

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** 드롭 처리 (컨테이너 외부로 드롭 시 아이템 버리기) */
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    //-----------------------------------------------------
    // 컨테이너 초기화 및 설정
    //-----------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Container")
    void SetupContainerWidget(AItemContainer* Container, const TArray<FContainerItemData>& ContainerItems);

    UFUNCTION(BlueprintCallable, Category = "Container")
    void RefreshContainerUI();

    void RefreshInventoryUI();

    //-----------------------------------------------------
    // 슬롯 위젯 생성 및 관리
    //-----------------------------------------------------

protected:
    UFUNCTION(BlueprintCallable, Category = "Container")
    UItemContainerSlotWidget* CreateContainerSlotWidget(int32 SlotIndex, const FContainerItemData& ItemData);

    UFUNCTION()
    void OnCloseButtonClicked();

    //-----------------------------------------------------
    // 드래그앤드롭 헬퍼 함수
    //-----------------------------------------------------

public:
    UFUNCTION(BlueprintCallable, Category = "Container|DragDrop")
    void HandleItemMoveToContainer(class UInventorySlotWidget* SourceWidget, int32 ContainerSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Container|DragDrop")
    void HandleItemMoveToPlayer(UItemContainerSlotWidget* SourceWidget, int32 PlayerSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Container|DragDrop")
    void HandleContainerSlotSwap(UItemContainerSlotWidget* SourceWidget, int32 TargetSlotIndex);

    //-----------------------------------------------------
    // 유틸리티 함수
    //-----------------------------------------------------

protected:
    void LoadItemDataTable();
    void ClearContainerSlots();

    APlayerController* GetOwnerPlayerController() const;

private:
    /** 현재 컨테이너 슬롯 데이터 캐시 */
    TArray<FContainerItemData> CachedContainerItems;
};
