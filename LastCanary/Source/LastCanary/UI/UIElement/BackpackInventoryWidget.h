#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/InventoryWidgetBase.h"
#include "BackpackInventoryWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UGridPanel;
class UBackpackSlotWidget;
UCLASS()
class LASTCANARY_API UBackpackInventoryWidget : public UInventoryWidgetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UGridPanel* BackpackSlotPanel;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* WeightText;
    UPROPERTY(meta = (BindWidget))
    UProgressBar* WeightProgressBar;

    /** 가방 슬롯 위젯 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes")
    TSubclassOf<UBackpackSlotWidget> BackpackSlotWidgetClass;

    void RefreshInventoryUI() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    int32 ColumnsPerRow = 5;

    /** 가방 슬롯 위젯 생성 헬퍼 함수 */
    UFUNCTION(BlueprintCallable)
    UBackpackSlotWidget* CreateBackpackSlotWidget(int32 BackpackSlotIndex, const FBackpackSlotData& SlotData);

    ///** 가방→툴바 이동을 위한 일반 슬롯 위젯 생성 함수 추가 */
    //UFUNCTION(BlueprintCallable)
    //UInventorySlotWidget* CreateToolbarSlotWidget(int32 BackpackSlotIndex, const FBackpackSlotData& SlotData);
};
