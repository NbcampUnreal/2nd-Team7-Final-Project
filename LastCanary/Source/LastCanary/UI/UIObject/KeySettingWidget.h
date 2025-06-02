#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "KeySettingWidget.generated.h"

class UInputKeySelector;
class UTextBlock;
struct FEnhancedActionKeyMapping;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_MoveLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_MoveRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_StrafeLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_StrafeRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Walk;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Sprint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Jump;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Crouch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Interact;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Aim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_ThrowItem;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_Voice;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_ChangeShootingMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_SelectQuickSlot1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_SelectQuickSlot2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_SelectQuickSlot3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_SelectQuickSlot4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_OpenPauseMenu;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input") UInputAction* IA_OpenExitDrone;


	// 바인딩 위젯
#pragma region MoveActionKeyWidget
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
	UTextBlock* ActionLabelMoveLeft;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyMoveLeft;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorMoveLeft;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelMoveRight;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyMoveRight;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorMoveRight;
#pragma endregion 

#pragma region StrafeActionWidget
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
#pragma endregion 

#pragma region WalkActionWidget
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelWalk;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyWalk;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorWalk;
#pragma endregion

#pragma region SprintActionWidget
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelSprint;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeySprint;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorSprint;
#pragma endregion

#pragma region JumpActionWidget
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelJump;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyJump;
	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorJump;
#pragma endregion

#pragma region CrouchActionWidget
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelCrouch;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyCrouch;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorCrouch;
#pragma endregion

#pragma region InteractActionWidget
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelInteract;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyInteract;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorInteract;
#pragma endregion

#pragma region AimActionWidget

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelAim;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyAim;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorAim;
#pragma endregion

#pragma region ThrowItemActionWidget

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelThrowItem;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyThrowItem;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorThrowItem;
#pragma endregion

#pragma region VoiceActionWidget

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelVoice;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyVoice;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorVoice;
#pragma endregion

#pragma region ChangeShootingModeWidget

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelChangeShootingMode;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyChangeShootingMode;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorChangeShootingMode;
#pragma endregion

#pragma region ChangeQuickSlotActionWidget
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelSelectQuickSlot1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeySelectQuickSlot1;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorSelectQuickSlot1;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelSelectQuickSlot2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeySelectQuickSlot2;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorSelectQuickSlot2;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelSelectQuickSlot3;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeySelectQuickSlot3;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorSelectQuickSlot3;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelSelectQuickSlot4;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeySelectQuickSlot4;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorSelectQuickSlot4;
#pragma endregion
	
#pragma region OpenPauseMenuActionWidget
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelOpenPauseMenu;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyOpenPauseMenu;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorOpenPauseMenu;
#pragma endregion

#pragma region ExitDroneActionWidget
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabelExitDrone;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentKeyExitDrone;

	UPROPERTY(meta = (BindWidget))
	UInputKeySelector* SelectorExitDrone;
#pragma endregion


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
	void OnKeySelectedMoveLeft(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedMoveRight(FInputChord SelectedKey);
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
	UFUNCTION()
	void OnKeySelectedAim(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedThrowItem(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedVoice(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedChangeShootingMode(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedChangeQuickSlot1(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedChangeQuickSlot2(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedChangeQuickSlot3(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedChangeQuickSlot4(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedOpenPauseMenu(FInputChord SelectedKey);
	UFUNCTION()
	void OnKeySelectedExitDrone(FInputChord SelectedKey);

public:
	//매핑 결과 업데이트 함수
	UFUNCTION(BlueprintCallable)
	void RefreshMappings(const TArray<FEnhancedActionKeyMapping>& KeyMappings);


	UFUNCTION(BlueprintCallable)
	void UpdateMappings(FName DisplayName, FKey Key);

	UFUNCTION(BlueprintCallable)
	void InitialMappings();
	//TMap<FName, FKey> PendingMappings;

	//void QueueMapping(FName MappingName, FKey Key);
	//void ApplyPendingMappings();
};
