#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "UI/UIObject/TaskbarAppButton.h"
#include "DesktopWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class ULCUIManager;
class UTaskbarWidget;
class UCanvasPanel;
class ALCRoomPlayerController;
class ULCDesktopWindowManager;

/**
 * 바탕화면 위젯 (싱글 클릭: 강조, 더블 클릭: 앱 실행)
 */
UCLASS()
class LASTCANARY_API UDesktopWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

	//-----------------
	// 오버라이드 함수
	//-----------------
public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	//-----------------
	// 접근자 / 설정자
	//-----------------
public:
	FORCEINLINE void SetWindowManager(ULCDesktopWindowManager* InManager) { WindowManager = InManager; }
	FORCEINLINE ULCDesktopWindowManager* GetWindowManager() const { return WindowManager; }
	FORCEINLINE UCanvasPanel* GetRootCanvas() const { return RootCanvas; }
	FORCEINLINE UCanvasPanel* GetWindowContainer() const { return WindowContainer; }
	FORCEINLINE UTaskbarWidget* GetTaskbarWidget() const { return TaskbarWidget; }

	//-----------------
	// 창 추가
	//-----------------
public:
	void AddWindow(UUserWidget* NewWindow);

	//-----------------
	// 바인딩된 위젯
	//-----------------
protected:
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* RootCanvas;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* WindowContainer;

	UPROPERTY(meta = (BindWidget))
	UButton* ShopIconButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CharacterCustomizationIconButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseDesktopButton;

	UPROPERTY(meta = (BindWidget))
	UTaskbarWidget* TaskbarWidget;

	//-----------------
	// 매니저 및 클래스
	//-----------------
protected:
	UPROPERTY()
	ULCDesktopWindowManager* WindowManager;

	UPROPERTY(EditDefaultsOnly, Category = "Taskbar")
	TSubclassOf<UTaskbarAppButton> TaskbarAppButtonClass;

	//-----------------
	// 버튼 클릭 처리
	//-----------------
private:
	UFUNCTION()
	void OnShopIconSingleClicked();

	UFUNCTION()
	void HandleShopAppLaunch();

	UFUNCTION()
	void OnCharacterCustomizationIconSingleClicked();

	UFUNCTION()
	void HandleCharacterCustomizationLaunch();

	UFUNCTION()
	void OnCloseDesktopClicked();

	//-----------------
	// 더블 클릭 처리 및 강조 관리
	//-----------------
private:
	int32 ClickCount = 0;
	FTimerHandle DoubleClickTimerHandle;

	UFUNCTION()
	void ResetClickCount();

	UFUNCTION()
	void HighlightAppButton(UButton* TargetButton);

	UFUNCTION()
	void ResetAppButtonHighlight(UButton* Button);

	UFUNCTION()
	void ResetAllAppButtonHighlights();

	UButton* CurrentlyHighlightedButton = nullptr;

	//-----------------
	// 내부 유틸리티
	//-----------------
private:
	ALCRoomPlayerController* GetRoomPC() const;

public:
	void PowerOn();

	UPROPERTY(Transient, meta = (BindWidgetAnim), EditAnywhere, BlueprintReadWrite)
	UWidgetAnimation* PlayPowerOnAnim;
};
