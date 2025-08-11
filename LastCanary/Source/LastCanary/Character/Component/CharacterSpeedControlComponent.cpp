#include "Character/Component/CharacterSpeedControlComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/BasePlayerState.h"

#include "LastCanary.h"

UCharacterSpeedControlComponent::UCharacterSpeedControlComponent()
{
	InitIngameMovementSpeed();
}

void UCharacterSpeedControlComponent::InitIngameMovementSpeed()
{
	WalkSpeed = DefaultWalkSpeed;
	RunSpeed = DefaultRunSpeed;
	SprintSpeed = DefaultSprintSpeed;
	CrouchSpeed = DefaultCrouchSpeed;
	JumpZVelocity = DefaultJumpZVelocity;
}

void UCharacterSpeedControlComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCharacterSpeedControlComponent::Client_SetMovementSetting_Implementation()
{
	ABasePlayerState* MyPlayerState = GetCharacter()->GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	GetCharacter()->SpeedMultiplier = GetCharacter()->CalculateMovementSpeedMultiplier();

	//스테이트에 바뀐 값 저장
	CrouchSpeed = MyPlayerState->DefaultCrouchSpeed * GetCharacter()->SpeedMultiplier;
	WalkSpeed = MyPlayerState->DefaultWalkSpeed * GetCharacter()->SpeedMultiplier;
	RunSpeed = MyPlayerState->DefaultRunSpeed * GetCharacter()->SpeedMultiplier;
	SprintSpeed = MyPlayerState->DefaultSprintSpeed * GetCharacter()->SpeedMultiplier;
	JumpZVelocity = MyPlayerState->DefaultJumpZVelocity * GetCharacter()->SpeedMultiplier;

	MyPlayerState->CrouchSpeed = CrouchSpeed;
	MyPlayerState->WalkSpeed = WalkSpeed;
	MyPlayerState->RunSpeed = RunSpeed;
	MyPlayerState->SprintSpeed = SprintSpeed;
	MyPlayerState->JumpZVelocity = JumpZVelocity;

	/*
	GetCharacter()->AlsCharacterMovement->SetPlayerMovementSpeed(CrouchSpeed, WalkSpeed, RunSpeed, SprintSpeed);
	GetCharacter()->AlsCharacterMovement->JumpZVelocity = JumpZVelocity;
	*/
}

void UCharacterSpeedControlComponent::SetCharacterMovementSpeed()
{
}
