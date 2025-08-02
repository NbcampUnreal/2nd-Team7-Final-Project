#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "TaskbarAppButton.generated.h"

/**
 * 심플한 작업 표시줄 앱 버튼 (아이콘 전용)
 */

class UButton;
class UImage;
class UUserWidget;

UCLASS()
class LASTCANARY_API UTaskbarAppButton : public ULCUserWidgetBase
{
    GENERATED_BODY()

public:
    void InitializeAppButton(UTexture2D* InIcon, UUserWidget* InAppWidget);

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

    UPROPERTY()
    UUserWidget* AppWidget;
};
