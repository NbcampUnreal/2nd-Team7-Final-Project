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
	ABasePlayerState* MyPlayerState = GetBaseCharacter()->GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	GetBaseCharacter()->SpeedMultiplier = GetBaseCharacter()->CalculateMovementSpeedMultiplier();

	//스테이트에 바뀐 값 저장
	CrouchSpeed = MyPlayerState->DefaultCrouchSpeed * GetBaseCharacter()->SpeedMultiplier;
	WalkSpeed = MyPlayerState->DefaultWalkSpeed * GetBaseCharacter()->SpeedMultiplier;
	RunSpeed = MyPlayerState->DefaultRunSpeed * GetBaseCharacter()->SpeedMultiplier;
	SprintSpeed = MyPlayerState->DefaultSprintSpeed * GetBaseCharacter()->SpeedMultiplier;
	JumpZVelocity = MyPlayerState->DefaultJumpZVelocity * GetBaseCharacter()->SpeedMultiplier;

	MyPlayerState->CrouchSpeed = CrouchSpeed;
	MyPlayerState->WalkSpeed = WalkSpeed;
	MyPlayerState->RunSpeed = RunSpeed;
	MyPlayerState->SprintSpeed = SprintSpeed;
	MyPlayerState->JumpZVelocity = JumpZVelocity;

	/*
	
	->AlsCharacterMovement->SetPlayerMovementSpeed(CrouchSpeed, WalkSpeed, RunSpeed, SprintSpeed);
	GetCharacter()->AlsCharacterMovement->JumpZVelocity = JumpZVelocity;
	*/
}

void UCharacterSpeedControlComponent::SetCharacterMovementSpeed()
{
}
