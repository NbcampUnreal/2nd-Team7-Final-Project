#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InputAction.h"
#include "SpectatorWidget.generated.h"

class UTextBlock;
class UInputAction;
UCLASS()
class LASTCANARY_API USpectatorWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** UI 텍스트 요소 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PrevKeyText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NextKeyText;

	/** 매핑된 키를 가져오기 위한 InputAction 참조 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_SpectatePrev;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_SpectateNext;

private:
	FString GetCurrentKeyNameForAction(UInputAction* InputAction) const;
};
