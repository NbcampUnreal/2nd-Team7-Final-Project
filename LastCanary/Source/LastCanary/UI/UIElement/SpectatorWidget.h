#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InputAction.h"
#include "SpectatorWidget.generated.h"

class UTextBlock;
class UInputAction;
class UInputMappingContext;
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
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* IMC_Default;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Move;
	
private:
	FString GetDirectionalKeyName(UInputAction* InputAction, float DirectionThreshold) const;

public:
	void UpdatePlayerName(FString Name);
};