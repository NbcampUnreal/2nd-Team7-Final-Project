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
class UOverlay;
class ALCRoomPlayerController;
class ULCDesktopWindowManager;

/**
 * 바탕화면 위젯 (싱글 클릭: 강조, 더블 클릭: 앱 실행)
 */
UCLASS()
class LASTCANARY_API UDesktopWidget : public ULCUserWidgetBase
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    FORCEINLINE UTaskbarWidget* GetTaskbarWidget() const { return TaskbarWidget; }

    FORCEINLINE void SetWindowManager(ULCDesktopWindowManager* InManager) { WindowManager = InManager; }
    FORCEINLINE ULCDesktopWindowManager* GetWindowManager() const { return WindowManager; }

    UCanvasPanel* GetRootCanvas() const { return RootCanvas; }

    void AddWindow(UUserWidget* NewWindow);

protected:
    UPROPERTY()
    ULCDesktopWindowManager* WindowManager; // Init 시점에 할당 필요
    
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* RootCanvas;
    UPROPERTY(meta = (BindWidget))
    UOverlay* WindowContainer;

    // 위젯 바인딩
    UPROPERTY(meta = (BindWidget))
    UButton* ShopIconButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseDesktopButton;

    UPROPERTY(meta = (BindWidget))
    UTaskbarWidget* TaskbarWidget;

    // 버튼 생성용 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Taskbar")
    TSubclassOf<UTaskbarAppButton> TaskbarAppButtonClass;

private:
    UFUNCTION()
    void OnShopIconSingleClicked();        // 단일 클릭 강조
    void HandleShopAppLaunch();            // 상점 앱 실행

    UFUNCTION()
    void OnCloseDesktopClicked();          // 데스크탑 닫기

    ALCRoomPlayerController* GetRoomPC() const;

    int32 ClickCount = 0;
    FTimerHandle DoubleClickTimerHandle;

    UFUNCTION()
    void ResetClickCount();
};
