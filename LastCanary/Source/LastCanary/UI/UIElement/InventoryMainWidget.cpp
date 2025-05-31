#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/UIElement/ToolbarInventoryWidget.h"
#include "UI/UIElement/BackpackInventoryWidget.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"

#include "LastCanary.h"

void UInventoryMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AutoInitializeWithPlayer();

	// ShowToolbarOnly();
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

    if (UToolbarInventoryComponent* ToolbarComp = Pawn->FindComponentByClass<UToolbarInventoryComponent>())
    {
        if (ToolbarWidget)
        {
            ToolbarWidget->SetInventoryComponent(ToolbarComp);
            UE_LOG(LogTemp, Warning, TEXT("[AutoInitializeWithPlayer] 툴바 위젯 연결 완료"));
        }
    }
    
    if (UBackpackInventoryComponent* BackpackComp = Pawn->FindComponentByClass<UBackpackInventoryComponent>())
    {
        if (BackpackWidget)
        {
            BackpackWidget->SetInventoryComponent(BackpackComp);
            UE_LOG(LogTemp, Warning, TEXT("[AutoInitializeWithPlayer] 백팩 위젯 연결 완료"));
        }
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
    if (bBackpackInventoryOpen)
    {
        // 가방 숨기기
        if (BackpackWidget)
        {
            BackpackWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
        bBackpackInventoryOpen = false;
        LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] 가방 인벤토리 닫기"));
    }
    else
    {
        // 가방 표시 전에 컴포넌트 연결 확인 및 수정
        LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] === 가방 인벤토리 열기 시작 ==="));

        if (!BackpackWidget)
        {
            LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] ❌ BackpackWidget is null!"));
            return;
        }

        // ⭐ 올바른 BackpackInventoryComponent 찾기
        UBackpackInventoryComponent* CorrectBackpackComponent = GetCorrectBackpackComponent();
        if (!CorrectBackpackComponent)
        {
            LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] ❌ 유효한 BackpackInventoryComponent를 찾을 수 없음"));
            return;
        }

        // ⭐ UI에 올바른 컴포넌트 연결
        BackpackWidget->SetInventoryComponent(CorrectBackpackComponent);
        LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] ✅ BackpackInventoryComponent 연결 완료"));

        // 가방 표시 및 UI 새로고침
        BackpackWidget->SetVisibility(ESlateVisibility::Visible);
        BackpackWidget->RefreshInventoryUI();

        bBackpackInventoryOpen = true;
        LOG_Item_WARNING(TEXT("[ToggleBackpackInventory] ✅ 가방 인벤토리 열기 완료"));
    }
}

bool UInventoryMainWidget::IsBackpackInventoryOpen() const
{
    return bBackpackInventoryOpen;
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

UBackpackInventoryComponent* UInventoryMainWidget::GetCorrectBackpackComponent()
{
    LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] === 시작 ==="));

    // 1. 플레이어 캐릭터 가져오기
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] ❌ PlayerController is null"));
        return nullptr;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] ❌ PlayerPawn is null"));
        return nullptr;
    }

    ABaseCharacter* Character = Cast<ABaseCharacter>(PlayerPawn);
    if (!Character)
    {
        LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] ❌ BaseCharacter 캐스팅 실패"));
        return nullptr;
    }

    LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] ✅ Character 찾음: %s"), *Character->GetName());

    // 2. BackpackInventoryComponent 가져오기
    UBackpackInventoryComponent* BackpackComp = Character->GetBackpackInventoryComponent();
    if (!BackpackComp)
    {
        LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] ❌ BackpackInventoryComponent is null"));
        return nullptr;
    }

    LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] BackpackInventoryComponent 주소: %p"), BackpackComp);
    LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] 슬롯 수: %d"), BackpackComp->ItemSlots.Num());
    LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] MaxSlots: %d"), BackpackComp->MaxSlots);

    // ⭐ 가방이 실제로 장착되어 있는지 확인 (캐릭터에서 직접 확인)
    if (!Character->HasBackpackEquipped())
    {
        LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] ❌ 가방이 장착되지 않음"));
        return nullptr;
    }

    // ⭐ MaxSlots 확인 (가방이 해제되면 0으로 설정되어야 함)
    if (BackpackComp->MaxSlots <= 0)
    {
        LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] ❌ 가방이 장착되지 않음 (MaxSlots: %d)"), BackpackComp->MaxSlots);
        return nullptr;
    }

    // 5. 각 슬롯 내용 디버그 출력
    for (int32 i = 0; i < BackpackComp->ItemSlots.Num(); ++i)
    {
        const FBaseItemSlotData& SlotData = BackpackComp->ItemSlots[i];
        if (!SlotData.ItemRowName.IsNone() && SlotData.Quantity > 0)
        {
            LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] 슬롯 %d: %s (수량: %d)"),
                i, *SlotData.ItemRowName.ToString(), SlotData.Quantity);
        }
    }

    LOG_Item_WARNING(TEXT("[GetCorrectBackpackComponent] ✅ 유효한 BackpackInventoryComponent 반환"));
    return BackpackComp;
}
