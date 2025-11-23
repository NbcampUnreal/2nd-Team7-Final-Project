#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "SelectionWheelWidget.generated.h"

USTRUCT(BlueprintType)
struct FSelectionWheelEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
};

class USelectionWheelEntryWidget;
class UCanvasPanel;

UCLASS()
class LASTCANARY_API USelectionWheelWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintCallable)
	void UpdateSelectionFromMouse();

	UFUNCTION(BlueprintCallable)
	void ConfirmSelection();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Wheel")
	TArray<FSelectionWheelEntry> Entries;

	UPROPERTY(BlueprintReadOnly, Category = "Selection Wheel")
	int32 CurrentIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Wheel")
	float Radius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Wheel")
	TSubclassOf<USelectionWheelEntryWidget> EntryWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* WheelCanvas;


protected:
	UPROPERTY()
	TArray<USelectionWheelEntryWidget*> EntryWidgets;

	void BuildWheel();

	UFUNCTION(BlueprintNativeEvent, Category = "Selection Wheel")
	void OnSelectionChanged(int32 NewIndex);
	virtual void OnSelectionChanged_Implementation(int32 NewIndex);

	UFUNCTION(BlueprintNativeEvent, Category = "Selection Wheel")
	void OnSelectionConfirmed(int32 ConfirmedIndex);
	virtual void OnSelectionConfirmed_Implementation(int32 ConfirmedIndex);

	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Wheel")
	float DeadZoneRadius = 40.f;

	UPROPERTY()
	FVector2D WheelCenterViewport = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D ViewportScaleCached = FVector2D(1.f, 1.f);

	UPROPERTY()
	FVector2D WheelCenterLocalInSelf = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Wheel|Sound")
	USoundBase* ChangeClickSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Wheel|Sound")
	USoundBase* ConfirmPopSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Wheel|Sound")
	USoundBase* ConfirmSound = nullptr;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Selection Wheel")
	bool bWheelOpen = false;
};
