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

	virtual void RefreshInventoryUI() PURE_VIRTUAL(UInventoryWIdgetBase::RefreshInventoryUI, return;);

	UFUNCTION()
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnInventoryChanged();

	UFUNCTION(BlueprintCallable)
	void SetInventoryComponent(UInventoryComponentBase* NewInventoryComponent);


	UInventoryComponentBase* GetInventoryComponent() const;

protected:
	/** 슬롯 위젯 생성 헬퍼 함수 */
	UFUNCTION(BlueprintCallable)
	UInventorySlotWidget* CreateSlotWidget(int32 SlotIndex, const FBaseItemSlotData& SlotData);
};
