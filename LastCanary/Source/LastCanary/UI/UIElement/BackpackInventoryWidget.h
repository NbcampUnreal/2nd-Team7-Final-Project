// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/InventoryWidgetBase.h"
#include "BackpackInventoryWidget.generated.h"

class UGridPanel;

UCLASS()
class LASTCANARY_API UBackpackInventoryWidget : public UInventoryWidgetBase
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UGridPanel* BackpackSlotPanel;

	void RefreshInventoryUI() override;

protected:
	/** 그리드 레이아웃 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	int32 ColumnsPerRow = 6;
};
