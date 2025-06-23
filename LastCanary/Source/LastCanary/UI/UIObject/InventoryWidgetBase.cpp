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

	CreateSharedTooltipWidget();

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

void UInventoryWidgetBase::CreateSharedTooltipWidget()
{
	if (!TooltipWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InventoryWidgetBase::CreateSharedTooltipWidget] TooltipWidgetClass가 설정되지 않음"));
		return;
	}

	if (!SharedTooltipWidget)
	{
		SharedTooltipWidget = CreateWidget<UItemTooltipWidget>(this, TooltipWidgetClass);
		if (!SharedTooltipWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("[InventoryWidgetBase::CreateSharedTooltipWidget] 공유 툴팁 위젯 생성 실패"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[InventoryWidgetBase::CreateSharedTooltipWidget] 공유 툴팁 위젯 생성 성공"));
		}
	}
}

void UInventoryWidgetBase::OnInventoryChanged()
{
	HideTooltip();
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

void UInventoryWidgetBase::ShowTooltipForSlot(const FBaseItemSlotData& ItemData, UWidget* SourceWidget)
{
	// 빈 슬롯이거나 Default 아이템이면 툴팁 표시하지 않음
	if (ItemData.ItemRowName.IsNone() || ItemData.ItemRowName == FName("Default"))
	{
		HideTooltip();
		return;
	}

	if (!SharedTooltipWidget)
	{
		CreateSharedTooltipWidget();
		if (!SharedTooltipWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("[ShowTooltipForSlot] 공유 툴팁 위젯이 없음"));
			return;
		}
	}

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShowTooltipForSlot] ItemDataTable이 NULL"));
		return;
	}

	FItemDataRow* ItemRowData = ItemDataTable->FindRow<FItemDataRow>(ItemData.ItemRowName, TEXT("ShowTooltipForSlot"));
	if (!ItemRowData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShowTooltipForSlot] 아이템 데이터를 찾을 수 없음: %s"), *ItemData.ItemRowName.ToString());
		return;
	}

	// ⭐ 툴팁 데이터 설정 및 표시
	SharedTooltipWidget->SetTooltipData(*ItemRowData, ItemData);

	if (!SharedTooltipWidget->IsInViewport())
	{
		SharedTooltipWidget->AddToViewport(10);
	}

	// 툴팁 위치 업데이트 시작
	UpdateTooltipPosition();
	GetWorld()->GetTimerManager().SetTimer(TooltipUpdateTimer,
		this, &UInventoryWidgetBase::UpdateTooltipPosition,
		0.016f, true);

	UE_LOG(LogTemp, Log, TEXT("[ShowTooltipForSlot] 툴팁 표시: %s"), *ItemData.ItemRowName.ToString());
}

void UInventoryWidgetBase::HideTooltip()
{
	// 타이머 정리
	if (TooltipUpdateTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TooltipUpdateTimer);
	}

	// 툴팁 숨김
	if (SharedTooltipWidget && SharedTooltipWidget->IsInViewport())
	{
		SharedTooltipWidget->RemoveFromParent();
		UE_LOG(LogTemp, Log, TEXT("[HideTooltip] 툴팁 숨김"));
	}
}

bool UInventoryWidgetBase::IsTooltipVisible() const
{
	return SharedTooltipWidget && SharedTooltipWidget->IsInViewport();
}

void UInventoryWidgetBase::UpdateTooltipPosition()
{
	if (!SharedTooltipWidget || !SharedTooltipWidget->IsInViewport())
	{
		return;
	}

	FVector2D FinalPosition = CalculateTooltipScreenPosition();
	SharedTooltipWidget->SetPositionInViewport(FinalPosition);

	//FVector2D MousePosition;
	//if (APlayerController* PC = GetOwningPlayer())
	//{
	//	PC->GetMousePosition(MousePosition.X, MousePosition.Y);

	//	FVector2D TooltipPosition = MousePosition + FVector2D(15.0f, -50.0f);

	//	// 화면 경계 처리
	//	float MinY = 100.0f;
	//	TooltipPosition.Y = FMath::Max(TooltipPosition.Y, MinY);

	//	float MinX = 50.0f;
	//	TooltipPosition.X = FMath::Max(TooltipPosition.X, MinX);

	//	FVector2D ViewportSize;
	//	if (GEngine && GEngine->GameViewport)
	//	{
	//		GEngine->GameViewport->GetViewportSize(ViewportSize);

	//		FVector2D EstimatedTooltipSize(230.0f, 230.0f);

	//		float MaxX = ViewportSize.X - EstimatedTooltipSize.X - 10.0f;
	//		float MaxY = ViewportSize.Y - EstimatedTooltipSize.Y - 10.0f;

	//		TooltipPosition.X = FMath::Clamp(TooltipPosition.X, MinX, MaxX);
	//		TooltipPosition.Y = FMath::Clamp(TooltipPosition.Y, MinY, MaxY);
	//	}

	//	SharedTooltipWidget->SetPositionInViewport(TooltipPosition);
	//}
}

FVector2D UInventoryWidgetBase::CalculateTooltipScreenPosition() const
{
	FVector2D MousePosition(0.f, 0.f);
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->GetMousePosition(MousePosition.X, MousePosition.Y);
	}

	FVector2D ViewportSize(1920.f, 1080.f);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const FVector2D EstimatedTooltipSize(230.f, 230.f);
	const float PaddingOffset = 10.f;

	// 방향 결정
	bool bShowLeft = (MousePosition.X + EstimatedTooltipSize.X + PaddingOffset > ViewportSize.X);
	bool bShowAbove = (MousePosition.Y + EstimatedTooltipSize.Y + PaddingOffset > ViewportSize.Y);

	FVector2D Offset;
	Offset.X = bShowLeft ? -EstimatedTooltipSize.X - 15.f : 15.f;
	Offset.Y = bShowAbove ? -EstimatedTooltipSize.Y - 15.f : 15.f;

	FVector2D DesiredPosition = MousePosition + Offset;

	// Clamp 처리
	float MinX = PaddingOffset;
	float MinY = PaddingOffset;
	float MaxX = ViewportSize.X - EstimatedTooltipSize.X - PaddingOffset;
	float MaxY = ViewportSize.Y - EstimatedTooltipSize.Y - PaddingOffset;

	DesiredPosition.X = FMath::Clamp(DesiredPosition.X, MinX, MaxX);
	DesiredPosition.Y = FMath::Clamp(DesiredPosition.Y, MinY, MaxY);

	return DesiredPosition;
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

	// ⭐ 부모 인벤토리 위젯 참조 설정
	SlotWidget->SetParentInventoryWidget(this);

	return SlotWidget;
}

FBaseItemSlotData UInventoryWidgetBase::ConvertBackpackSlotToBaseSlot(const FBackpackSlotData& BackpackSlot)
{
	FBaseItemSlotData Result;
	Result.ItemRowName = BackpackSlot.ItemRowName;
	Result.Quantity = BackpackSlot.Quantity;
	Result.Durability = 0.0f;
	Result.bIsValid = BackpackSlot.IsValid();
	Result.bIsEquipped = false;
	// 기타 필드는 기본값 또는 무시
	return Result;
}
