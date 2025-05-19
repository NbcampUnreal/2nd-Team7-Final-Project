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
	
	UGridPanel* BackpackSlotPanel;

public:
	void RefreshInventoryUI() override;
};
