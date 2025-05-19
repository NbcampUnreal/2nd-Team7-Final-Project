#include "UI/UIObject/InventoryWidgetBase.h"
#include "Inventory/InventoryComponentBase.h"
#include "LastCanary.h"

void UInventoryWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInventoryWidgetBase::OninventoryChanged()
{
	RefreshInventoryUI();
}

void UInventoryWidgetBase::SetInventoryComponent(UInventoryComponentBase* NewInventoryComponent)
{
	if (!NewInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[InventoryWidgetBase::SetInventoryComponent] NewInventoryComponent is null!"));
		return;
	}

	InventoryComponent = NewInventoryComponent;

	if (!InventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[InventoryWidgetBase::SetInventoryComponent] InventoryComponent is null!"));
		return;
	}

	if (!InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &UInventoryWidgetBase::OninventoryChanged))
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidgetBase::OninventoryChanged);
	}

	RefreshInventoryUI();
}

UInventoryComponentBase* UInventoryWidgetBase::GetInventoryComponent() const
{
	return InventoryComponent;
}
