#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DesktopWindowBaseWidget.generated.h"

class UHorizontalBox;
class UImage;
class UButton;
class USizeBox;
class UCanvasPanel;
class ULCDesktopWindowManager;
class UDesktopWidget;
class UTexture2D;

/**
 * 기본 데스크탑 윈도우 위젯 클래스
 */
UCLASS()
class LASTCANARY_API UDesktopWindowBaseWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

	//-----------------
	// 윈도우 상태 관리
	//-----------------
public:
	UFUNCTION(BlueprintCallable)
	virtual void InitDesktopWindow();

	virtual void MinimizeWindow();
	virtual void CloseWindow();
	virtual void ToggleMaximizeRestore();

	bool IsMinimized() const;
	void SetMinimized(bool bInIsMinimized);

	bool IsMaximized() const;
	void SetMaximized(bool bInMaximized);

	void UpdateMaximizeButtonIcon();

	void SetWindowManager(ULCDesktopWindowManager* InManager);
	UDesktopWidget* GetDesktopWidget() const;

protected:
	UPROPERTY()
	bool bIsMinimized = false;

	UPROPERTY()
	bool bIsMaximized = false;

	//-----------------
	// 앱 정보 및 아이콘
	//-----------------
public:
	void SetAppInfo(const FName& InAppID, const FText& InAppName, UTexture2D* InIcon);
	UHorizontalBox* GetTitleTextContainer() const;
	FName GetAppID() const;
	FText GetAppName() const;
	UTexture2D* GetAppIcon() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "App Info")
	FName AppID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "App Info")
	FText AppName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "App Info")
	UTexture2D* AppIcon;

	UPROPERTY(EditAnywhere, Category = "Window Icons")
	FSlateBrush MaximizeBrush;

	UPROPERTY(EditAnywhere, Category = "Window Icons")
	FSlateBrush RestoreBrush;

	//-----------------
	// 레이아웃 저장 및 복원
	//-----------------
public:
	void SetOriginalPosition(const FVector2D& InPos);
	void SetOriginalSize(const FVector2D& InSize);
	void SetOriginalAnchors(const FAnchors& InAnchors);
	void SetOriginalAlignment(const FVector2D& InAlignment);

	const FVector2D GetOriginalPosition() const;
	const FVector2D GetOriginalSize() const;
	FAnchors GetOriginalAnchors() const { return OriginalAnchors; }
	FVector2D GetOriginalAlignment() const { return OriginalAlignment; }

protected:
	FVector2D OriginalPosition;
	FVector2D OriginalSize;
	FAnchors OriginalAnchors;
	FVector2D OriginalAlignment;

	//-----------------
	// 애니메이션
	//-----------------
public:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* MinimizeAnim;

	UFUNCTION(BlueprintCallable)
	void PlayMinimizeAnimation();

	UFUNCTION(BlueprintCallable)
	void PlayRestoreAnimation();

	void AnimateWindowTransform(bool bMaximize, FVector2D InTargetSize);
	void UpdateWindowTransformAnimation();

protected:
	FTimerHandle WindowTransformAnimTimer;
	float AnimationAlpha = 0.0f;
	float ElapsedTime = 0.0f;
	const float AnimationDuration = 0.1f;

	FVector2D StartPosition;
	FVector2D TargetPosition;
	FVector2D StartSize;
	FVector2D TargetSize;

	bool bAnimatingToMaximized = false;

	//-----------------
	// 드래그 관련
	//-----------------
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void StartDragging(const FVector2D& InMouseScreenPos);
	void UpdateDrag(const FVector2D& InMouseScreenPos);
	void StopDragging();
	bool IsInTitleBar(const FVector2D& ScreenPos) const;

	bool bDragging = false;
	FVector2D DragOffset;
	bool bWasMouseOutsideWindowContainer = false;

	//-----------------
	// 더블클릭 판별
	//-----------------
protected:
	FDateTime LastClickTime;
	FVector2D LastClickPosition;
	const float DoubleClickThreshold = 0.25f;
	const float MaxClickDelta = 5.f;

	//-----------------
	// 버튼 콜백
	//-----------------
protected:
	UFUNCTION()
	void OnMinimizeClicked();

	UFUNCTION()
	void OnMaximizeClicked();

	UFUNCTION()
	virtual void OnCloseClicked();

	//-----------------
	// 바인딩된 위젯들
	//-----------------
public:
protected:
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* TitleTextContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	UHorizontalBox* TitleBar;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* MinimizeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* MaximizeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ExitButton;

	//-----------------
	// 시스템 참조 및 초기화
	//-----------------
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	ULCDesktopWindowManager* WindowManager = nullptr;
};
