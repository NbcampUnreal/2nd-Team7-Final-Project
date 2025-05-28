#include "UI/UIObject/InventoryWidgetBase.h"
#include "Inventory/InventoryComponentBase.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "LastCanary.h"

void UInventoryWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (!SlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventoryWidgetBase::NativeConstruct] SlotWidgetClass가 설정되지 않음. 블루프린트에서 설정하세요."));
	}

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				ItemDataTable = GISubsystem->GetItemDataTable();
				if (!ItemDataTable)
				{
					LOG_Item_WARNING(TEXT("[InventoryWidgetBase::NativeConstruct] ItemDataTable이 서브시스템에서 로드되지 않음"));
				}
			}
		}
	}
}

void UInventoryWidgetBase::OnInventoryChanged()
{
	RefreshInventoryUI();
}

void UInventoryWidgetBase::SetInventoryComponent(UInventoryComponentBase* NewInventoryComponent)
{
	if (InventoryComponent && InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &UInventoryWidgetBase::OnInventoryChanged))
	{
		InventoryComponent->OnInventoryUpdated.RemoveDynamic(this, &UInventoryWidgetBase::OnInventoryChanged);
	}

	InventoryComponent = NewInventoryComponent;

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidgetBase::OnInventoryChanged);
		RefreshInventoryUI();
		LOG_Item_WARNING(TEXT("[InventoryWidgetBase::SetInventoryComponent] 인벤토리 컴포넌트 설정 완료"));
	}
	else
	{
		LOG_Item_WARNING(TEXT("[InventoryWidgetBase::SetInventoryComponent] NewInventoryComponent is null!"));
	}
}

UInventoryComponentBase* UInventoryWidgetBase::GetInventoryComponent() const
{
	return InventoryComponent;
}

UInventorySlotWidget* UInventoryWidgetBase::CreateSlotWidget(int32 SlotIndex, const FBaseItemSlotData& SlotData)
{
	if (!SlotWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[InventoryWidgetBase::CreateSlotWidget] SlotWidgetClass가 설정되지 않음"));
		return nullptr;
	}

	UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
	if (!SlotWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[InventoryWidgetBase::CreateSlotWidget] 슬롯 위젯 생성 실패: %d"), SlotIndex);
		return nullptr;
	}

	// 슬롯 데이터 설정
	SlotWidget->SetItemData(SlotData, ItemDataTable);
	SlotWidget->SetInventoryComponent(InventoryComponent);
	SlotWidget->SlotIndex = SlotIndex;

	return SlotWidget;
}