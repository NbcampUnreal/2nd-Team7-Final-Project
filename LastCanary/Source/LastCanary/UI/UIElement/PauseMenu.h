#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "PauseMenu.generated.h"

/**
 * 
 */
class UButton;
UCLASS()
class LASTCANARY_API UPauseMenu : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	bool bHasActivated = false;

	UPROPERTY(meta = (BindWidget))
	UButton* ResumeButton;
	UPROPERTY(meta = (BindWidget))
	UButton* OptionButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* EnterAnimation;

	UFUNCTION()
	void OnResumeButtonClicked();
	UFUNCTION()
	void OnOptionButtonClicked();
	UFUNCTION()
	void OnExitButtonClicked();

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void DestroySessionAndGoTitleMenu();

	virtual void DestroySessionAndGoTitleMenu_Implementation();
};
