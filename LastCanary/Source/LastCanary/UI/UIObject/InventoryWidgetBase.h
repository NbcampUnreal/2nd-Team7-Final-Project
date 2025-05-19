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

	//UPROPERTY(meta = (BindWidget))
	//class UHorizontalBox* ToolbarSlotBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* ItemDataTable;

	virtual void RefreshInventoryUI() PURE_VIRTUAL(UInventoryWIdgetBase::RefreshInventoryUI, return;);

	UFUNCTION()
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OninventoryChanged();

	UFUNCTION()
	void SetInventoryComponent(UInventoryComponentBase* NewInventoryComponent);


	UInventoryComponentBase* GetInventoryComponent() const;

};
