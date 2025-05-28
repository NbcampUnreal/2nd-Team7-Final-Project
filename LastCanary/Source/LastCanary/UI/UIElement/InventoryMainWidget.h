#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InventoryMainWidget.generated.h"

class UToolbarInventoryWidget;
class UBackpackInventoryWidget;

UCLASS()
class LASTCANARY_API UInventoryMainWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	

	UPROPERTY(meta = (BindWidget))
	UToolbarInventoryWidget* ToolbarWidget;

	UPROPERTY(meta = (BindWidget))
	UBackpackInventoryWidget* BackpackWidget;

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void ShowToolbarOnly();

	UFUNCTION(BlueprintCallable)
	void ToggleBackpackInventory();

	UFUNCTION(BlueprintCallable)
	bool IsBackpackInventoryOpen() const;

private:
	bool bBackpackInventoryOpen = false;

	void AutoInitializeWithPlayer();

	UPROPERTY(EditAnywhere, Category = "WidgetClasses")
	TSubclassOf<UToolbarInventoryWidget> ToolbarWidgetClass;

	UPROPERTY(EditAnywhere, Category = "WidgetClasses")
	TSubclassOf<UBackpackInventoryWidget> BackpackWidgetClass;

public:
	void RefreshInventory();
};
