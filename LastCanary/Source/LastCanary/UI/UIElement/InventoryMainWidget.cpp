#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/UIElement/ToolbarInventoryWidget.h"
#include "UI/UIElement/BackpackInventoryWidget.h"
#include "UI/UIObject/InventorySlotWidget.h"
#include "UI/UIObject/ItemDropQuantityWidget.h"
#include "UI/UIObject/BackpackSlotWidget.h"
#include "UI/UIObject/GunAmmoWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "DataType/BaseItemSlotData.h"
#include "Inventory/ToolbarInventoryComponent.h"

#include "Blueprint/DragDropOperation.h"
#include "Components/TextBlock.h"

#include "LastCanary.h"

void UInventoryMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AutoInitializeWithPlayer();

	ShowToolbarOnly();

	RestoreGunAmmoUIState();
}

bool UInventoryMainWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 슬롯 위젯에서 드래그된 아이템인지 확인
	UInventorySlotWidget* SourceWidget = Cast<UInventorySlotWidget>(InOperation->Payload);
	if (!SourceWidget)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	// Default 아이템이면 드롭 불가
	if (SourceWidget->ItemData.ItemRowName.IsNone() ||
		SourceWidget->ItemData.ItemRowName == FName("Default"))
	{
		return false;
	}

	// 개수에 따라 처리 분기
	if (SourceWidget->ItemData.Quantity <= 1)
	{
		// 1개면 바로 드롭
		HandleDropOutsideSlots(SourceWidget, 1);
	}
	else
	{
		// 여러 개면 개수 선택 UI 표시
		ShowItemDropQuantityWidget(SourceWidget);
	}

	return true;
}

void UInventoryMainWidget::AutoInitializeWithPlayer()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoInitializeWithPlayer] PlayerController 또는 Pawn이 NULL"));
		return;
	}

	APawn* Pawn = PC->GetPawn();

	UToolbarInventoryComponent* ToolbarComp = Pawn->FindComponentByClass<UToolbarInventoryComponent>();
	if (!ToolbarComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AutoInitializeWithPlayer] 툴바 컴포넌트가 없습니다."));
		return;
	}

	if (ToolbarWidget)
	{
		ToolbarWidget->SetInventoryComponent(ToolbarComp);
	}

	if (BackpackWidget)
	{
		BackpackWidget->SetInventoryComponent(ToolbarComp);
	}
}

void UInventoryMainWidget::ShowToolbarOnly()
{
	if (ToolbarWidget)
	{
		ToolbarWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (BackpackWidget)
	{
		BackpackWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	bBackpackInventoryOpen = false;
}

void UInventoryMainWidget::ToggleBackpackInventory()
{
	if (!BackpackWidget)
	{
		LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] 가방 위젯이 없습니다."));

		return;
	}

	if (bBackpackInventoryOpen)
	{
		HideItemDropQuantityWidget();
		CancelCurrentDragOperation();
		BackpackWidget->HideTooltip();
		BackpackWidget->SetVisibility(ESlateVisibility::Collapsed);
		bBackpackInventoryOpen = false;
		LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] 가방 인벤토리 닫기 (드롭 위젯 포함)"));
	}
	else
	{
		BackpackWidget->SetVisibility(ESlateVisibility::Visible);
		CancelCurrentDragOperation();
		BackpackWidget->HideTooltip();
		BackpackWidget->RefreshInventoryUI();
		bBackpackInventoryOpen = true;
		LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] 가방 인벤토리 열기"));
	}
}

bool UInventoryMainWidget::IsBackpackInventoryOpen() const
{
	return bBackpackInventoryOpen;
}

void UInventoryMainWidget::ShowToolbarSlotItemText(const FText& ItemName)
{
	if (SlotItemText == nullptr)
	{
		return;
	}

	SlotItemText->SetText(FText::Format(NSLOCTEXT("UI", "CurrentSlotItem", "{0}"), ItemName));
	SlotItemText->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().ClearTimer(SlotItemTextTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(SlotItemTextTimerHandle, this, &UInventoryMainWidget::HideToolbarSlotItemText, 1.5f, false);
}

void UInventoryMainWidget::HideToolbarSlotItemText()
{
	if (SlotItemText)
	{
		SlotItemText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryMainWidget::ShowItemDropQuantityWidget(UInventorySlotWidget* SourceWidget)
{
	if (!ItemDropQuantityWidgetClass || !SourceWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShowItemDropQuantityWidget] ItemDropQuantityWidgetClass가 설정되지 않음"));
		return;
	}

	// 기존 위젯이 있으면 제거
	if (CurrentDropQuantityWidget)
	{
		CurrentDropQuantityWidget->RemoveFromParent();
		CurrentDropQuantityWidget = nullptr;
	}

	// 새 위젯 생성
	CurrentDropQuantityWidget = CreateWidget<UItemDropQuantityWidget>(this, ItemDropQuantityWidgetClass);
	if (!CurrentDropQuantityWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[ShowItemDropQuantityWidget] 개수 선택 위젯 생성 실패"));
		return;
	}

	// 소스 위젯 임시 저장
	PendingDropSourceWidget = SourceWidget;

	// 아이템 데이터 가져오기
	FText ItemName = FText::FromString(TEXT("Item"));
	if (SourceWidget->ItemDataTable)
	{
		FItemDataRow* ItemRowData = SourceWidget->ItemDataTable->FindRow<FItemDataRow>(
			SourceWidget->ItemData.ItemRowName, TEXT("ShowItemDropQuantityWidget"));
		if (ItemRowData)
		{
			ItemName = FText::FromString(ItemRowData->ItemName.ToString());
		}
	}

	// 위젯 설정
	CurrentDropQuantityWidget->SetupWidget(ItemName, SourceWidget->ItemData.Quantity);

	// 델리게이트 바인딩
	CurrentDropQuantityWidget->OnQuantityConfirmed.AddDynamic(this, &UInventoryMainWidget::OnQuantityConfirmed);
	CurrentDropQuantityWidget->OnQuantityCanceled.AddDynamic(this, &UInventoryMainWidget::OnQuantityCanceled);

	// 화면에 추가 (높은 우선순위로)
	CurrentDropQuantityWidget->AddToViewport(100);

	UE_LOG(LogTemp, Log, TEXT("[ShowItemDropQuantityWidget] 개수 선택 UI 표시"));
}

void UInventoryMainWidget::HideItemDropQuantityWidget()
{
	if (CurrentDropQuantityWidget)
	{
		// 델리게이트 해제
		CurrentDropQuantityWidget->OnQuantityConfirmed.RemoveAll(this);
		CurrentDropQuantityWidget->OnQuantityCanceled.RemoveAll(this);

		// 위젯 제거
		CurrentDropQuantityWidget->RemoveFromParent();
		CurrentDropQuantityWidget = nullptr;
	}

	// 대기 중인 소스 위젯도 정리
	PendingDropSourceWidget = nullptr;
}

void UInventoryMainWidget::HandleDropOutsideSlots(UInventorySlotWidget* SourceWidget, int32 Quantity)
{
	if (!SourceWidget || !SourceWidget->InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HandleDropOutsideSlots] SourceWidget 또는 InventoryComponent가 null"));
		return;
	}

	// 백팩 슬롯인지 확인
	if (UBackpackSlotWidget* CurrentBackpackWidget = Cast<UBackpackSlotWidget>(SourceWidget))
	{
		// 백팩에서 아이템 제거
		UToolbarInventoryComponent* ToolbarComp = Cast<UToolbarInventoryComponent>(SourceWidget->InventoryComponent);
		if (ToolbarComp)
		{
			bool bSuccess = ToolbarComp->DropItemFromBackpack(CurrentBackpackWidget->BackpackSlotIndex, Quantity);
			UE_LOG(LogTemp, Log, TEXT("[HandleDropOutsideSlots] 백팩 아이템 드롭 결과: %s"),
				bSuccess ? TEXT("성공") : TEXT("실패"));
		}
	}
	else
	{
		// 일반 툴바 슬롯에서 드롭
		bool bSuccess = SourceWidget->InventoryComponent->TryDropItemAtSlot(SourceWidget->SlotIndex, Quantity);
		UE_LOG(LogTemp, Log, TEXT("[HandleDropOutsideSlots] 툴바 아이템 드롭 결과: %s (슬롯: %d, 개수: %d)"),
			bSuccess ? TEXT("성공") : TEXT("실패"), SourceWidget->SlotIndex, Quantity);
	}
}

void UInventoryMainWidget::CancelCurrentDragOperation()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication& SlateApp = FSlateApplication::Get();
		if (SlateApp.IsDragDropping())
		{
			SlateApp.CancelDragDrop();
		}
	}
}

bool UInventoryMainWidget::IsDragInProgress() const
{
	if (FSlateApplication::IsInitialized())
	{
		return FSlateApplication::Get().IsDragDropping();
	}
	return false;
}

void UInventoryMainWidget::OnQuantityConfirmed(int32 Quantity)
{
	if (PendingDropSourceWidget)
	{
		HandleDropOutsideSlots(PendingDropSourceWidget, Quantity);
	}

	HideItemDropQuantityWidget();

	UE_LOG(LogTemp, Log, TEXT("[OnQuantityConfirmed] 드롭 확인: %d개"), Quantity);
}

void UInventoryMainWidget::OnQuantityCanceled()
{
	HideItemDropQuantityWidget();

	UE_LOG(LogTemp, Log, TEXT("[OnQuantityCanceled] 드롭 취소됨"));
}

void UInventoryMainWidget::RefreshInventory()
{
	if (ToolbarWidget)
	{
		ToolbarWidget->RefreshInventoryUI();
	}

	if (BackpackWidget)
	{
		BackpackWidget->RefreshInventoryUI();
	}
}

void UInventoryMainWidget::SetGunAmmoUIVisibility(bool bVisible, int32 CurrentAmmo, int32 MaxAmmo, EFireMode CurrentFireMode, const TArray<EFireMode>& AvailableFireModes)
{
	if (!GunAmmoWidget)
	{
		LOG_Item_WARNING(TEXT("[SetGunAmmoUIVisibility] GunAmmoWidget이 null"));
		return;
	}

	if (bVisible && MaxAmmo > 0)
	{
		GunAmmoWidget->ShowAmmoUI(CurrentAmmo, MaxAmmo, CurrentFireMode, AvailableFireModes);
		LOG_Item_WARNING(TEXT("[SetGunAmmoUIVisibility] 탄환 UI 표시: %d/%d, 발사모드: %d"), CurrentAmmo, MaxAmmo, (int32)CurrentFireMode);
	}
	else
	{
		GunAmmoWidget->HideAmmoUI();
		LOG_Item_WARNING(TEXT("[SetGunAmmoUIVisibility] 탄환 UI 숨김"));
	}
}

void UInventoryMainWidget::InitializeGunAmmoUI()
{
	if (GunAmmoWidget)
	{
		GunAmmoWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryMainWidget::RestoreGunAmmoUIState()
{
	InitializeGunAmmoUI();

	// 현재 장착된 총기가 있는지 확인
	UToolbarInventoryComponent* ToolbarComp = GetOwnerToolbarComponent();
	if (!ToolbarComp)
	{
		LOG_Item_WARNING(TEXT("[RestoreGunAmmoUIState] ToolbarComponent를 찾을 수 없음"));
		return;
	}

	// 현재 장착된 아이템 확인
	AItemBase* CurrentEquippedItem = ToolbarComp->GetCurrentEquippedItem();
	if (!CurrentEquippedItem)
	{
		LOG_Item_WARNING(TEXT("[RestoreGunAmmoUIState] 장착된 아이템 없음"));
		return;
	}

	// 총기인지 확인
	AGunBase* EquippedGun = Cast<AGunBase>(CurrentEquippedItem);
	if (!EquippedGun)
	{
		LOG_Item_WARNING(TEXT("[RestoreGunAmmoUIState] 장착된 아이템이 총기가 아님"));
		return;
	}

	// 총기 UI 복원
	int32 CurrentAmmo = FMath::RoundToInt(EquippedGun->Durability);
	int32 MaxAmmo = FMath::RoundToInt(EquippedGun->MaxDurability);

	if (GunAmmoWidget && MaxAmmo > 0)
	{
		GunAmmoWidget->ShowAmmoUI(CurrentAmmo, MaxAmmo, EquippedGun->CurrentFireMode, EquippedGun->AvailableFireModes);
		LOG_Item_WARNING(TEXT("[RestoreGunAmmoUIState] 총기 UI 복원 완료: %d/%d"), CurrentAmmo, MaxAmmo);
	}
}

UToolbarInventoryComponent* UInventoryMainWidget::GetOwnerToolbarComponent() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->GetPawn())
	{
		return nullptr;
	}

	return PC->GetPawn()->FindComponentByClass<UToolbarInventoryComponent>();
}
