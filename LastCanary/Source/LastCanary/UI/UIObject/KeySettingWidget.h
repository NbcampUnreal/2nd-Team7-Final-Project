#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "KeySettingWidget.generated.h"

class UInputKeySelector;
class UTextBlock;

UCLASS()
class LASTCANARY_API UKeySettingWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* MappingContext;

	// 액션들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_MoveForward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_MoveBackward;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_StrafeLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_StrafeRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Walk;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Sprint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Jump;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Crouch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Interact;

	// 바인딩 위젯
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelMoveForward;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyMoveForward;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorMoveForward;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelMoveBackward;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyMoveBackward;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorMoveBackward;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelStrafeLeft;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyStrafeLeft;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorStrafeLeft;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelStrafeRight;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyStrafeRight;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorStrafeRight;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelWalk;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyWalk;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorWalk;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelSprint;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeySprint;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorSprint;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelJump;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyJump;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorJump;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelCrouch;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyCrouch;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorCrouch;

	//UPROPERTY(meta = (BindWidget)) 
	UTextBlock* ActionLabelInteract;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyInteract;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorInteract;

	// 유틸
	void InitRow(const FString& DisplayName, UInputAction* Action, UTextBlock* Label, UTextBlock* KeyText, UInputKeySelector* Selector);
	FKey GetMappedKey(UInputAction* Action) const;
	void SetMappedKey(UInputAction* Action, FKey NewKey);

	// 바인딩 이벤트 핸들러
	UFUNCTION()
	void OnKeySelectedMoveForward(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedMoveBackward(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedStrafeLeft(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedStrafeRight(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedWalk(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedSprint(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedJump(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedCrouch(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedInteract(FInputChord SelectedKey);
};
