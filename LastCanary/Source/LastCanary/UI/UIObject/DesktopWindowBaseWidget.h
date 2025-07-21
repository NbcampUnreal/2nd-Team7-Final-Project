#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DesktopWindowBaseWidget.generated.h"

class UButton;
class UBorder;
class UTexture2D;

/**
 * 바탕화면 UI 위젯의 공통 기반 클래스
 * - 드래그 이동, 최소화/최대화/닫기 버튼 제공
 * - Taskbar에 자동 등록되도록 구성
 */
UCLASS()
class LASTCANARY_API UDesktopWindowBaseWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 초기화 시 호출 (자식에서 Super::NativeConstruct 이후 호출) */
	UFUNCTION(BlueprintCallable)
	void InitDesktopWindow();

	/** 최소화 (실제 창을 숨기거나, 작업표시줄 등록 등 처리) */
	UFUNCTION()
	virtual void MinimizeWindow();

	/** 창 닫기 (기본 구현은 RemoveFromParent) */
	UFUNCTION()
	virtual void CloseWindow();

	/** 최대화/복원 토글 */
	UFUNCTION(BlueprintCallable)
	virtual void ToggleMaximizeRestore();

	// ------------ [App 정보 설정 / 접근자] -------------
	void SetAppInfo(const FName& InAppID, const FText& InAppName, UTexture2D* InIcon);

	UFUNCTION(BlueprintCallable)
	FName GetAppID() const { return AppID; }

	UFUNCTION(BlueprintCallable)
	FText GetAppName() const { return AppName; }

	UFUNCTION(BlueprintCallable)
	UTexture2D* GetAppIcon() const { return AppIcon; }

protected:
	UFUNCTION()
	virtual void OnMinimizeClicked();

	UFUNCTION()
	virtual void OnMaximizeClicked();

	UFUNCTION()
	virtual void OnCloseClicked();

	/** 마우스 위치로부터 드래그 시작 */
	void StartDragging(const FVector2D& InMousePos);
	/** 드래그 중 위치 갱신 */
	void UpdateDrag(const FVector2D& InMousePos);
	/** 드래그 종료 */
	void StopDragging();

	/** 타이틀 바 내에 마우스 위치가 포함되는지 */
	bool IsInTitleBar(const FVector2D& ScreenPos) const;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* MinimizeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* MaximizeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ExitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* TitleBar;

	/** 드래그 상태 */
	bool bDragging = false;
	FVector2D DragOffset;

	/** 복원용 원래 위치/크기 */
	FVector2D OriginalSize;
	FVector2D OriginalPosition;

	/** 최대화 여부 */
	bool bIsMaximized = false;

	/** 작업표시줄용 앱 정보 */
	UPROPERTY(EditDefaultsOnly, Category = "App")
	FName AppID;

	UPROPERTY(EditDefaultsOnly, Category = "App")
	FText AppName;

	UPROPERTY(EditDefaultsOnly, Category = "App")
	UTexture2D* AppIcon;
};
