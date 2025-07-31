#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LCDesktopWindowManager.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;
class UUserWidget;
class UDesktopWidget;

/**
 * 창 관리 전용 매니저 (실제 데스크탑 UI 관리)
 */
UCLASS()
class LASTCANARY_API ULCDesktopWindowManager : public UObject
{
	GENERATED_BODY()

public:
	/** 최상위 캔버스를 지정하고 데스크탑 생성 */
	void Init(UCanvasPanel* InRootCanvas);

	void OpenWindow(UUserWidget* Widget);
	void CloseWindow(UUserWidget* Widget);
	void MinimizeWindow(UUserWidget* Widget);
	void ToggleMaximizeRestore(UUserWidget* Widget);

	FORCEINLINE void SetDesktopWidget(UDesktopWidget* InDesktopWidget) { DesktopWidget = InDesktopWidget;  }
	FORCEINLINE UDesktopWidget* GetDesktopWidget() const { return DesktopWidget; }

protected:
	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY()
	TObjectPtr<UDesktopWidget> DesktopWidget;

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> OpenedWindows;

private:
	int CurrentZOrder = 100;

public:
	int GetNextZOrder();

	bool IsTopMost(UUserWidget* Widget) const;
};
