#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "TaskbarWidget.generated.h"

/**
 * 작업 표시줄 위젯
 */

class UHorizontalBox;
class UTaskbarAppButton;
class UUserWidget;
class UTexture2D;
class UDesktopWindowBaseWidget;

UCLASS()
class LASTCANARY_API UTaskbarWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
	// 앱 등록 / 제거
	void RegisterApp(UUserWidget* AppWidget, const FName& AppID, const FText& AppName, UTexture2D* Icon);
	void RemoveApp(const FName& AppID);
	bool IsAppRegistered(const FName& AppID) const;

	// DesktopWindow 위젯을 위한 Taskbar 버튼 자동 생성
	void AddAppButtonFor(UDesktopWindowBaseWidget* Window);

	// 외부에서 Taskbar 버튼 클래스 주입 가능하게
	void SetTaskbarAppButtonClass(TSubclassOf<UTaskbarAppButton> InClass);

protected:
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* AppSlotBox;

	UPROPERTY()
	TSubclassOf<UTaskbarAppButton> TaskbarAppButtonClass;

	UPROPERTY()
	TMap<FName, UTaskbarAppButton*> ActiveAppButtons;
};
