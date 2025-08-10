#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterInputComponent.generated.h"

struct FInputActionValue;

UCLASS()
class LASTCANARY_API UCharacterInputComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

private:
	bool bEnableInput = true;
public:
	bool IsInputEnabled() const { return bEnableInput; }
	void SetInputEnabled(bool bEnabled) { bEnableInput = bEnabled; }
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity, float ZoomSensivity, bool _bIsAiming, float MouseSensitivityMultiplier, float MouseInvertMultiplier);
	void Handle_Move(const FInputActionValue& ActionValue);
	void Handle_Sprint(const FInputActionValue& ActionValue);
	void Handle_Walk(const FInputActionValue& ActionValue);
	void Handle_Crouch(const FInputActionValue& ActionValue);
	void Handle_Jump(const FInputActionValue& ActionValue);
	void Handle_Aim(const FInputActionValue& ActionValue);
	void Handle_Interact(const FInputActionValue& ActionValue);
	void Handle_ViewMode();
	void Handle_Reload();
	void Handle_VoiceChatting(const FInputActionValue& ActionValue);

public:
	bool CheckCondition_LookMouse();
	bool CheckCondition_Move();
	bool CheckCondition_Sprint();
	bool CheckCondition_Walk();
	bool CheckCondition_Crouch();
	bool CheckCondition_Jump();
	bool CheckCondition_Aim();
	bool CheckCondition_Interact();
	bool CheckCondition_ViewMode();
	bool CheckCondition_Reload();
	bool CheckCondition_VoiceChatting();

	bool Check_PlayerController();
	bool Check_PlayerState();
	bool Check_InputEnabled();

	bool Check_DefaultCondition();
};
