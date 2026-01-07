#include "Character/Component/CharacterInputComponent.h"
#include "Character/BaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "Character/BasePlayerState.h"
#include "Camera/CameraComponent.h"
#include "Character/Component/CharacterStaminaComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/Component/CharacterInteractionComponent.h"
#include "Character/Component/CharacterAnimationComponent.h"
#include "Character/Component/CharacterCameraControlComponent.h"
#include "Character/Component/CharacterAttackComponent.h"
#include "Character/Component/CharacterADSComponent.h"

#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/Utility/AlsVector.h"
#include "Inventory/ToolbarInventoryComponent.h"

#include "LastCanary.h"

void UCharacterInputComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterInputComponent::Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity, float ZoomSensivity, bool _bIsAiming, float MouseSensitivityMultiplier, float MouseInvertMultiplier)
{
	if (!CheckCondition_LookMouse())
	{
		return;
	}

	const FVector2f Value{ ActionValue.Get<FVector2D>() };
	
	if (_bIsAiming)
	{
		GetBaseCharacter()->AddControllerYawInput(Value.X * ZoomSensivity * MouseSensitivityMultiplier * MouseInvertMultiplier);
		GetBaseCharacter()->AddControllerPitchInput(Value.Y * ZoomSensivity * MouseSensitivityMultiplier * MouseInvertMultiplier);
	}
	else
	{
		GetBaseCharacter()->AddControllerYawInput(Value.X * Sensivity * MouseSensitivityMultiplier * MouseInvertMultiplier);
		GetBaseCharacter()->AddControllerPitchInput(Value.Y * Sensivity * MouseSensitivityMultiplier * MouseInvertMultiplier);
	}

}

void UCharacterInputComponent::Handle_Move(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Move())
	{
		return;
	}

	const auto Value{ UAlsVector::ClampMagnitude012D(ActionValue.Get<FVector2D>()) };
	
	GetBaseCharacter()->CancelInteraction();
	GetBaseCharacter()->FrontInput = Value.Y;
	const auto ForwardDirection{ UAlsVector::AngleToDirectionXY(UE_REAL_TO_FLOAT(GetBaseCharacter()->GetViewState().Rotation.Yaw)) };
	const auto RightDirection{ UAlsVector::PerpendicularCounterClockwiseXY(ForwardDirection) };
	
	GetBaseCharacter()->AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}

void UCharacterInputComponent::Handle_Sprint(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Sprint())
	{
		return;
	}
	if (!IsValid(GetBaseCharacter()->StaminaComponent))
	{
		return;
	}

	const float Value = ActionValue.Get<float>();

	if (GetBaseCharacter()->CheckHardLandState())
	{
		GetBaseCharacter()->bIsSprinting = false;
		GetBaseCharacter()->SetDesiredGait(AlsGaitTags::Running);
		GetBaseCharacter()->StaminaComponent->StopStaminaDrain();
		GetBaseCharacter()->StaminaComponent->StartStaminaRecoverAfterDelay();
		return;
	}

	GetBaseCharacter()->StopGunAutoFire(); // 총 연사상태면 해제하기

	if (Value < 0.5f) //입력이 떼지는 거면 어차피 뛰는 거 아님..
	{
		GetBaseCharacter()->bIsSprinting = false;
		GetBaseCharacter()->SetDesiredGait(AlsGaitTags::Running);
		GetBaseCharacter()->StaminaComponent->StopStaminaDrain();
		GetBaseCharacter()->StaminaComponent->StartStaminaRecoverAfterDelay();
		return;
	}

	if (GetBaseCharacter()->StaminaComponent->bIsExhausted) //만약 지친 상태라면 불가
	{
		return;
	}
	//달리기 시작하면서 스테미나 소모 시작
	GetBaseCharacter()->StaminaComponent->StartStaminaDrain();
	GetBaseCharacter()->StaminaComponent->StopStaminaRecovery();
	GetBaseCharacter()->StaminaComponent->StopStaminaRecoverAfterDelay();


	//**당장은 홀드 방식에 대해서만으로 개발 진행.... 추후에 기능 분리 **//
	/*
	if (MyPlayerState->SprintInputMode == EInputMode::Hold)
	{
		if (Value < 0.5f) //입력이 떼지는 거면 어차피 뛰는 거 아님..
		{
			GetCharacter()->bIsSprinting = false;
			GetCharacter()->FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
			GetCharacter()->SetDesiredGait(AlsGaitTags::Running);
			GetCharacter()->StaminaComponent->StopStaminaDrain();
			GetCharacter()->StaminaComponent->StartStaminaRecoverAfterDelay();
			return;
		}
		GetCharacter()->FootSoundModifier = MyPlayerState->SprintingFootSoundModifier;

		//달리기 시작하면서 스테미나 소모 시작
		GetCharacter()->StaminaComponent->StartStaminaDrain();
		GetCharacter()->StaminaComponent->StopStaminaRecovery();
		GetCharacter()->StaminaComponent->StopStaminaRecoverAfterDelay();
	}
	else if (MyPlayerState->SprintInputMode == EInputMode::Toggle)
	{
		if (Value > 0.5f)
		{
			if (GetCharacter()->GetDesiredGait() == AlsGaitTags::Sprinting)
			{
				GetCharacter()->bIsSprinting = false;
				GetCharacter()->FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
				GetCharacter()->SetDesiredGait(AlsGaitTags::Running);
				GetCharacter()->StaminaComponent->StopStaminaDrain();
				GetCharacter()->StaminaComponent->StartStaminaRecoverAfterDelay();
			}
			else if (GetCharacter()->GetDesiredGait() == AlsGaitTags::Running)
			{
				GetCharacter()->FootSoundModifier = MyPlayerState->SprintingFootSoundModifier;
				GetCharacter()->Camera->AttachToComponent(GetCharacter()->SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				GetCharacter()->Camera->SetRelativeLocation(FVector::ZeroVector);
				GetCharacter()->Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
				GetCharacter()->StaminaComponent->StopStaminaRecovery();
				GetCharacter()->StaminaComponent->StopStaminaRecoverAfterDelay();
				GetCharacter()->StaminaComponent->StartStaminaDrain();
				//SetDesiredGait(AlsGaitTags::Sprinting);
			}
			else
			{
				GetCharacter()->FootSoundModifier = MyPlayerState->SprintingFootSoundModifier;
				GetCharacter()->Camera->AttachToComponent(GetCharacter()->SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				GetCharacter()->Camera->SetRelativeLocation(FVector::ZeroVector);
				GetCharacter()->Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
				GetCharacter()->StaminaComponent->StopStaminaRecovery();
				GetCharacter()->StaminaComponent->StopStaminaRecoverAfterDelay();
				GetCharacter()->StaminaComponent->StartStaminaDrain();
			}
		}
	}
	*/
}

void UCharacterInputComponent::Handle_Walk(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Walk())
	{
		return;
	}

	const float Value = ActionValue.Get<float>();

	if (Value > 0.5f)
	{
		GetBaseCharacter()->SetDesiredGait(AlsGaitTags::Walking);
	}
	else
	{
		GetBaseCharacter()->SetDesiredGait(AlsGaitTags::Running);
	}

	/*
	if (MyPlayerState->WalkInputMode == EInputMode::Hold)
	{
		if (Value > 0.5f)
		{
			GetCharacter()->FootSoundModifier = MyPlayerState->WalkingFootSoundModifier;
			GetCharacter()->SetDesiredGait(AlsGaitTags::Walking);
		}
		else
		{
			GetCharacter()->FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
			GetCharacter()->SetDesiredGait(AlsGaitTags::Running);
		}
	}
	else if (MyPlayerState->WalkInputMode == EInputMode::Toggle)
	{
		if (Value > 0.5f)
		{
			if (GetCharacter()->GetDesiredGait() == AlsGaitTags::Walking)
			{
				GetCharacter()->FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
				GetCharacter()->SetDesiredGait(AlsGaitTags::Running);
			}
			else if (GetCharacter()->GetDesiredGait() == AlsGaitTags::Running)
			{
				GetCharacter()->FootSoundModifier = MyPlayerState->WalkingFootSoundModifier;
				GetCharacter()->SetDesiredGait(AlsGaitTags::Walking);
			}
			else
			{
				GetCharacter()->FootSoundModifier = MyPlayerState->WalkingFootSoundModifier;
				GetCharacter()->SetDesiredGait(AlsGaitTags::Running);
			}
		}
	}
	*/
}

void UCharacterInputComponent::Handle_Crouch(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Crouch())
	{
		return;
	}

	GetBaseCharacter()->CancelInteraction();
	
	const float Value = ActionValue.Get<float>();
	if (Value > 0.5f)
	{
		GetBaseCharacter()->SetDesiredStance(AlsStanceTags::Crouching);
	}
	else
	{
		GetBaseCharacter()->SetDesiredStance(AlsStanceTags::Standing);
	}

	/*  당장은 홀드 방식만 채용
	if (MyPlayerState->CrouchInputMode == EInputMode::Hold)
	{
		if (Value > 0.5f)
		{
			GetCharacter()->FootSoundModifier = MyPlayerState->CrouchingFootSoundModifier;
			GetCharacter()->SetDesiredStance(AlsStanceTags::Crouching);
		}
		else
		{
			GetCharacter()->FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
			GetCharacter()->SetDesiredStance(AlsStanceTags::Standing);
		}
	}
	else if (MyPlayerState->CrouchInputMode == EInputMode::Toggle)
	{
		if (Value > 0.5f)
		{
			if (GetCharacter()->GetDesiredStance() == AlsStanceTags::Standing)
			{
				GetCharacter()->FootSoundModifier = MyPlayerState->CrouchingFootSoundModifier;
				GetCharacter()->SetDesiredStance(AlsStanceTags::Crouching);
			}
			else if (GetCharacter()->GetDesiredStance() == AlsStanceTags::Crouching)
			{
				GetCharacter()->FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
				GetCharacter()->SetDesiredStance(AlsStanceTags::Standing);
			}
		}
	}
	*/
}

void UCharacterInputComponent::Handle_Jump(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Jump())
	{
		return;
	}

	const float Value = ActionValue.Get<float>();
	
	GetBaseCharacter()->CancelInteraction();

	if (Value > 0.5f)
	{
		if (GetBaseCharacter()->StopRagdolling())
		{
			return;
		}
		if (GetBaseCharacter()->StartMantlingGrounded())
		{
			GetBaseCharacter()->SetDesiredAiming(false);
			//GetCharacter()->SpringArm->AttachToComponent(GetCharacter()->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
			return;
		}
		if (GetBaseCharacter()->GetStance() == AlsStanceTags::Crouching)
		{
			GetBaseCharacter()->SetDesiredStance(AlsStanceTags::Standing);
			return;
		}
		if (GetBaseCharacter()->StaminaComponent->CanJump())
		{
			GetBaseCharacter()->Jump();
			if (!GetBaseCharacter()->CanJump())
			{
				return;
			}
			GetBaseCharacter()->StaminaComponent->ConsumeStaminaOnJump();
		}
	}
	else
	{
		GetBaseCharacter()->StopJumping();
	}
}

void UCharacterInputComponent::Handle_Aim(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Aim())
	{
		return;
	}
	
	AItemBase* EquippedItem = GetBaseCharacter()->ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!EquippedItem)
	{
		return;
	}
	
	AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem);
	if (!EquipmentItem)
	{
		return;
	}
	if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
	{
		//곡괭이를 들면 근접공격
		if (GetBaseCharacter()->AttackComponent)
		{
			if (ActionValue.Get<float>() > 0.5f)
			{
				GetBaseCharacter()->AttackComponent->Handle_Attack(EAttackType::ItemAttack);
			}
			return;
		}
	}
	
		

	//총을 들면 우클릭
	
	
	if (GetBaseCharacter()->bIsSprinting || GetBaseCharacter()->bIsReloading || GetBaseCharacter()->bIsClose || GetBaseCharacter()->bIsMantling)
	{
		GetBaseCharacter()->CameraControlComponent->StopAiming();
		return;
	}
	AGunBase* Gun = Cast<AGunBase>(EquipmentItem);
	if (!IsValid(Gun))
	{
		return;
	}
	if (Gun)
	{
		USkeletalMeshComponent* RifleMesh = Gun->GetSkeletalMeshComponent();
		GetBaseCharacter()->CurrentRifleMesh = RifleMesh;
		
		/*
		if (ActionValue.Get<float>() > 0.5f && GetBaseCharacter()->bIsCloseToWall == false)
		{
			if (GetBaseCharacter()->ADSComponent)
			{
				//GetBaseCharacter()->ADSComponent->SwitchADS(true);
			}
		}
		else
		{
			if (GetBaseCharacter()->ADSComponent)
			{
				//GetBaseCharacter()->ADSComponent->SwitchADS(false);
			}
		}
		*/
	}
	/*
	if(EquipmentItem)
	if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle"))
		|| EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol"))
		|| EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
	{
		AGunBase* RifleItem = Cast<AGunBase>(EquippedItem);
		
	}
	*/
}

void UCharacterInputComponent::Handle_Interact(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Interact())
	{
		return;
	}

	if (GetBaseCharacter()->InteractionComponent->CurrentFocusedActor->Implements<UInteractableInterface>())
	{
		AActor* actor = GetBaseCharacter()->InteractionComponent->CurrentFocusedActor;
		if (!IsValid(actor))
		{
			return;
		}

		if (GetPlayerController())
		{
			//CancelInteraction();
			//IInteractableInterface::Execute_Interact(CurrentFocusedActor, PC);
			LOG_Char_WARNING(TEXT("Handle_Interact: Called Interact on %s"), *actor->GetName());
			//GetCharacter()->InteractAfterPlayMontage(actor);
			//GetCharacter()->AnimationComponent->PlayInteractMontage(actor);

			GetBaseCharacter()->InteractionComponent->Handle_Interact();
		}
	}
}

void UCharacterInputComponent::Handle_ViewMode()
{
	if (!CheckCondition_ViewMode())
	{
		return;
	}
	//아래 코드 카메라 컴포넌트 함수로 변경
	GetBaseCharacter()->bIsFPSCamera = !(GetBaseCharacter()->bIsFPSCamera);
	GetBaseCharacter()->SetCameraMode(GetBaseCharacter()->bIsFPSCamera);
}

void UCharacterInputComponent::Handle_Reload()
{
	if (!CheckCondition_Reload())
	{
		return;
	}
	/*
	
	AItemBase* EquippedItem = GetCharacter()->ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!IsValid(EquippedItem))
	{
		return;
	}
	AGunBase* Gun = Cast<AGunBase>(EquippedItem);
	if (!IsValid(Gun))
	{
		return;
	}
	GetCharacter()->RequestReload(Gun);
	*/
}

void UCharacterInputComponent::Handle_VoiceChatting(const FInputActionValue& ActionValue)
{
}


bool UCharacterInputComponent::CheckCondition_LookMouse()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (GetBaseCharacter()->bIsPlayingInteractionMontage)
	{
		return false;
	}
	if (GetBaseCharacter()->GetLocomotionAction() == AlsLocomotionActionTags::Mantling)
	{
		return false;
	}

	return true;
}

bool UCharacterInputComponent::CheckCondition_Move()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (GetBaseCharacter()->CheckHardLandState())
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::CheckCondition_Sprint()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::CheckCondition_Walk()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (GetBaseCharacter()->CheckHardLandState())
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::CheckCondition_Crouch()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	
	if (GetBaseCharacter()->CheckHardLandState())
	{
		return false;
	}

	return true;
}

bool UCharacterInputComponent::CheckCondition_Jump()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (GetBaseCharacter()->CheckHardLandState())
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::CheckCondition_Aim()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (GetBaseCharacter()->CheckHardLandState())
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::CheckCondition_Interact()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (!GetBaseCharacter()->InteractionComponent->CurrentFocusedActor)
	{
		return false;
	}

	LOG_Char_WARNING(TEXT("Interacted with: %s"), *GetBaseCharacter()->InteractionComponent->CurrentFocusedActor->GetName());

	if (GetBaseCharacter()->bIsPlayingInteractionMontage)
	{
		return false;
	}

	return true;
}

bool UCharacterInputComponent::CheckCondition_ViewMode()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	
	return true;
}

bool UCharacterInputComponent::CheckCondition_Reload()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (GetBaseCharacter()->bIsReloading)
	{
		return false;
	}
	if (GetBaseCharacter()->bIsUsingItem)
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::CheckCondition_VoiceChatting()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::Check_PlayerController()
{
	if (!GetPlayerController())
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::Check_PlayerState()
{
	if (GetBaseCharacter()->CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::Check_InputEnabled()
{
	if (!IsInputEnabled())
	{
		return false;
	}
	return true;
}

bool UCharacterInputComponent::Check_DefaultCondition()
{
	if (!Check_PlayerController())
	{
		return false;
	}
	if (!Check_PlayerState())
	{
		return false;
	}
	if (!Check_InputEnabled())
	{
		return false;
	}
	return true;
}
