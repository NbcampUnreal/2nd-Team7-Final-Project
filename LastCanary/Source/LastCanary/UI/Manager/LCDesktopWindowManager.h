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
	//-----------------
	// 초기화 및 설정
	//-----------------

	void Init(UCanvasPanel* InRootCanvas);
	FORCEINLINE void SetDesktopWidget(UDesktopWidget* InDesktopWidget) { DesktopWidget = InDesktopWidget; }
	FORCEINLINE UDesktopWidget* GetDesktopWidget() const { return DesktopWidget; }

	//-----------------
	// 윈도우 열기 / 닫기 / 최소화 / 최대화
	//-----------------

	void OpenWindow(UUserWidget* Widget);
	void CloseWindow(UUserWidget* Widget);
	void MinimizeWindow(UUserWidget* Widget);
	void ToggleMaximizeRestore(UUserWidget* Widget);

	//-----------------
	// 정렬 / 상태 확인
	//-----------------

	int GetNextZOrder();
	bool IsTopMost(UUserWidget* Widget) const;

protected:
	//-----------------
	// 위젯 및 캔버스 참조
	//-----------------

	UPROPERTY()
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY()
	TObjectPtr<UDesktopWidget> DesktopWidget;

	//-----------------
	// 열린 윈도우 목록
	//-----------------

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> OpenedWindows;

private:
	//-----------------
	// 내부 상태
	//-----------------

	int CurrentZOrder = 100;
};
