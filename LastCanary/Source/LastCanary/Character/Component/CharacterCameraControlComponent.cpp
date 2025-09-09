#include "Character/Component/CharacterCameraControlComponent.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/SpringArmComponent.h"

void UCharacterCameraControlComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCharacterCameraControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

}

USpringArmComponent* UCharacterCameraControlComponent::GetCharacterSpringArm()
{
	return GetCharacter()->SpringArm;
}

USkeletalMeshComponent* UCharacterCameraControlComponent::CharacterMesh()
{
	return CachedCharacter->GetMesh();
}

void UCharacterCameraControlComponent::SwitchToFirstPerson()
{
	//GetCharacterSpringArm()->AttachToComponent(CharacterMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
	SetSpringArmTargetLength(0.0f);
}

void UCharacterCameraControlComponent::SwitchToThirdPerson()
{
	//GetCharacterSpringArm()->AttachToComponent(CharacterMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
	SetSpringArmTargetLength(200.0f);
}

void UCharacterCameraControlComponent::SetCameraMode(bool bIsFirstPersonView)
{
	if (bIsFirstPersonView)
	{
		EmoteMode = false;
		SetTransparentHeadMesh(true);
		//SwapHeadMaterialTransparent(true);
		SwitchToFirstPerson();
		SetSpringArmTargetLength(0.0f);
	}
	else
	{
		EmoteMode = true;
		SetTransparentHeadMesh(false);
		//SwapHeadMaterialTransparent(false);
		SwitchToThirdPerson();
	}
}

void UCharacterCameraControlComponent::SetCameraEmoteMode(bool bIsFirstPersonView)
{
	if (bIsFirstPersonView)
	{
		EmoteMode = false;
		GetCharacterSpringArm()->bDoCollisionTest = false;
		GetCharacterSpringArm()->ProbeChannel = ECC_Camera;
		GetCharacterSpringArm()->ProbeSize = 3.0f;
		SetTransparentHeadMesh(true);
		//SwapHeadMaterialTransparent(true);
		SwitchToFirstPerson();
		SetSpringArmTargetLength(0.0f);
	}
	else
	{
		EmoteMode = true;
		GetCharacterSpringArm()->bDoCollisionTest = true;
		GetCharacterSpringArm()->ProbeChannel = ECC_Camera;
		GetCharacterSpringArm()->ProbeSize = 3.0f;
		SetTransparentHeadMesh(false);
		//GetCharacter()->SwapHeadMaterialTransparent(false);
		SwitchToThirdPerson();
	}
}

void UCharacterCameraControlComponent::Handle_ViewMode()
{
	if (GetCharacter()->CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	bIsFPSCamera = !bIsFPSCamera;
	SetCameraMode(bIsFPSCamera);
}

void UCharacterCameraControlComponent::ResetCameraLocationToDefault()
{
	SwitchToFirstPerson();
	GetCharacterSpringArm()->bUsePawnControlRotation = true;
	GetCharacter()->bIsAiming = false;
	GetCharacter()->bIsTransitioning = false;
}

void UCharacterCameraControlComponent::SetSpringArmTargetLength(float Distance)
{
	GetCharacterSpringArm()->TargetArmLength = Distance;
}

void UCharacterCameraControlComponent::SetTransparentHeadMesh(bool bIsTransparent)
{
	GetCharacter()->CustomHeadMesh->SetOwnerNoSee(bIsTransparent);
}

void UCharacterCameraControlComponent::StartAiming()
{
	if (!GetCharacter()->bIsAiming)
	{
		GetCharacter()->bIsAiming = true;
		GetCharacter()->bIsTransitioning = true;

		// 스프링암을 RootComponent에 붙여서 자유롭게 움직일 수 있게 함
		//GetCharacterSpringArm()->AttachToComponent(GetCharacter()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

		GetCharacter()->CancelInteraction();
	}
}

void UCharacterCameraControlComponent::StopAiming()
{
	if (GetCharacter()->bIsAiming)
	{
		GetCharacter()->bIsAiming = false;
		GetCharacter()->bIsTransitioning = true;

		// 스프링암을 RootComponent에 붙여서 자유롭게 움직일 수 있게 함
		//GetCharacterSpringArm()->AttachToComponent(GetCharacter()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}
}