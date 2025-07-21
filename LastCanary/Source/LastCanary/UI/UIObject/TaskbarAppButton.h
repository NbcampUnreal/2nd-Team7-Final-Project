#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "TaskbarAppButton.generated.h"

/**
 * 작업 표시줄 앱 버튼 위젯 클래스
 */

class UButton;
class UImage;
class UTextBlock;
class UUserWidget;

UCLASS()
class LASTCANARY_API UTaskbarAppButton : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
    void InitializeAppButton(FName InAppID, const FText& InAppName, UTexture2D* InIcon, UUserWidget* InAppWidget);

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnAppButtonClicked();

    UFUNCTION()
    void OnAppButtonRightClicked();

protected:
    UPROPERTY(meta = (BindWidget))
    UButton* AppButton;

    UPROPERTY(meta = (BindWidget))
    UImage* AppIcon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* AppName;

    // 내부 상태
    UPROPERTY()
    FName AppID;

    UPROPERTY()
    UUserWidget* AppWidget;
};