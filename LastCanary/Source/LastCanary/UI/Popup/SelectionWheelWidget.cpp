#include "UI/Popup/SelectionWheelWidget.h"
#include "UI/UIObject/SelectionWheelEntryWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void USelectionWheelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsEnabled(true);
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;         
	BuildWheel();
}

void USelectionWheelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 최신 레이아웃 강제 갱신
	ForceLayoutPrepass();

	// WheelCanvas 기준 지오메트리
	const FGeometry& WheelGeo =
		(WheelCanvas ? WheelCanvas->GetCachedGeometry() : GetCachedGeometry());

	const FVector2D CenterLocal = WheelGeo.GetLocalSize() * 0.5f;

	// WheelCanvas 로컬 → Absolute
	const FVector2D CenterAbs = WheelGeo.LocalToAbsolute(CenterLocal);

	// Absolute → Self 로컬 (디버그는 이걸로!)
	const FGeometry& SelfGeo = GetCachedGeometry();
	WheelCenterLocalInSelf = SelfGeo.AbsoluteToLocal(CenterAbs);

	// Local(center) -> Viewport(center)
	FVector2D CenterPixel, CenterViewport;
	USlateBlueprintLibrary::LocalToViewport(
		const_cast<USelectionWheelWidget*>(this),
		WheelGeo,
		CenterLocal,
		CenterPixel,
		CenterViewport
	);

	WheelCenterViewport = CenterViewport;

	// Viewport DPI Scale 캐시
	const float Scale = UWidgetLayoutLibrary::GetViewportScale(const_cast<USelectionWheelWidget*>(this));
	ViewportScaleCached = FVector2D(Scale, Scale);

	// 매 프레임 마우스 기반 선택 갱신
	UpdateSelectionFromMouse();
}

FReply USelectionWheelWidget::NativeOnPreviewMouseButtonDown(
	const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("Wheel Preview Mouse Down!"));

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ConfirmSelection();
		UE_LOG(LogTemp, Log, TEXT("InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton"))

		return FReply::Handled();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton"))
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

void USelectionWheelWidget::BuildWheel()
{
	EntryWidgets.Empty();

	if (!WheelCanvas || !EntryWidgetClass || Entries.Num() == 0)
	{
		return;
	}

	WheelCanvas->ClearChildren();

	const int32 NumEntries = Entries.Num();
	const float SegmentAngle = 360.f / (float)NumEntries;

	for (int32 Index = 0; Index < NumEntries; ++Index)
	{
		USelectionWheelEntryWidget* EntryWidget =
			CreateWidget<USelectionWheelEntryWidget>(GetWorld(), EntryWidgetClass);

		if (!EntryWidget) continue;

		EntryWidget->EntryData = Entries[Index];

		UCanvasPanelSlot* CanvasSlot = WheelCanvas->AddChildToCanvas(EntryWidget);
		if (!CanvasSlot) continue;

		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetAutoSize(true);

		// 인덱스 0 = 위쪽, 시계방향
		const float AngleDeg = Index * SegmentAngle - 90.f;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);

		const float X = FMath::Cos(AngleRad) * Radius;
		const float Y = FMath::Sin(AngleRad) * Radius;

		CanvasSlot->SetPosition(FVector2D(X, Y));

		EntryWidgets.Add(EntryWidget);
	}

	// 처음엔 아무것도 선택 안 된 상태
	CurrentIndex = INDEX_NONE;
	OnSelectionChanged(CurrentIndex);
}

void USelectionWheelWidget::UpdateSelectionFromMouse()
{
	if (Entries.Num() == 0) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	// 마우스도 Viewport 기준으로 가져옴
	FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);

	// Viewport 좌표끼리 계산
	const FVector2D DeltaScreen = MousePos - WheelCenterViewport;

	const float DeadZone = DeadZoneRadius;
	if (DeltaScreen.Size() < DeadZone)
	{
		if (CurrentIndex != INDEX_NONE)
		{
			CurrentIndex = INDEX_NONE;
			OnSelectionChanged(CurrentIndex);
		}
		return;
	}

	// 화면 좌표(Y 아래 +) -> 수학 좌표(Y 위 +)
	const FVector2D DeltaMath(DeltaScreen.X, -DeltaScreen.Y);

	float AngleRad = FMath::Atan2(DeltaMath.Y, DeltaMath.X);
	float AngleDeg = FMath::RadiansToDegrees(AngleRad);
	if (AngleDeg < 0.f) AngleDeg += 360.f;

	const float SegmentAngle = 360.f / (float)Entries.Num();

	float FromUpClockwise = 90.f - AngleDeg;

	FromUpClockwise += SegmentAngle * 0.5f;

	if (FromUpClockwise < 0.f) FromUpClockwise += 360.f;
	if (FromUpClockwise >= 360.f) FromUpClockwise -= 360.f;

	int32 NewIndex = FMath::FloorToInt(FromUpClockwise / SegmentAngle);

	if (NewIndex != CurrentIndex)
	{
		CurrentIndex = NewIndex;
		OnSelectionChanged(CurrentIndex);

		if (ChangeClickSound)
		{
			UGameplayStatics::PlaySound2D(this, ChangeClickSound);
		}
	}
}

void USelectionWheelWidget::ConfirmSelection()
{
	if (CurrentIndex != INDEX_NONE)
	{
		if (ConfirmPopSound)
		{
			UGameplayStatics::PlaySound2D(this, ConfirmPopSound);
		}
		if (ConfirmSound)
		{
			UGameplayStatics::PlaySound2D(this, ConfirmSound);
		}

		OnSelectionConfirmed(CurrentIndex);
	}
}


void USelectionWheelWidget::OnSelectionChanged_Implementation(int32 NewIndex)
{
	const int32 Num = EntryWidgets.Num();

	for (int32 i = 0; i < Num; ++i)
	{
		if (EntryWidgets[i])
		{
			const bool bHighlighted = (i == NewIndex && NewIndex != INDEX_NONE);
			EntryWidgets[i]->SetHighlighted(bHighlighted);
		}
	}
}

void USelectionWheelWidget::OnSelectionConfirmed_Implementation(int32 ConfirmedIndex)
{
	if (!Entries.IsValidIndex(ConfirmedIndex))
		return;

	const FSelectionWheelEntry& Selected = Entries[ConfirmedIndex];

	UE_LOG(LogTemp, Log, TEXT("Selection Wheel Confirmed: %s"),
		*Selected.Id.ToString());

	// 화면 디버그 메시지(임시)
	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("Wheel Confirmed: %s"),
			*Selected.DisplayName.ToString());

		GEngine->AddOnScreenDebugMessage(
			-1, 2.0f, FColor::Yellow, Msg);
	}

	// TODO: 실제 게임 로직 연결
}

int32 USelectionWheelWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
		OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const float DZ = DeadZoneRadius;

	const FVector2D CenterLocal = WheelCenterLocalInSelf;

	//FSlateDrawElement::MakeBox(
	//	OutDrawElements,
	//	LayerId + 1,
	//	AllottedGeometry.ToPaintGeometry(
	//		FVector2D(CenterLocal.X - DZ, CenterLocal.Y - DZ),
	//		FVector2D(DZ * 2.f, DZ * 2.f)
	//	),
	//	FCoreStyle::Get().GetBrush("WhiteBrush"),
	//	ESlateDrawEffect::None,
	//	FLinearColor(1.f, 0.f, 0.f, 0.7f)
	//);

	return LayerId + 2;
}
