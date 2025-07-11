#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DesktopWidget.generated.h"

/**
 * 
 */
class UButton;
class ULCUIManager;
class ALCRoomPlayerController;

UCLASS()
class LASTCANARY_API UDesktopWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;

protected:
    // 바탕화면 상점 아이콘 버튼
    UPROPERTY(meta = (BindWidget))
    UButton* ShopIconButton;
    // 컴퓨터 닫기 버튼
    UPROPERTY(meta = (BindWidget))
    UButton* CloseDesktopButton;

private:
    UFUNCTION()
    void OnShopIconClicked();
    UFUNCTION()
    void OnCloseDesktopClicked();

    ALCRoomPlayerController* GetRoomPC() const;
};