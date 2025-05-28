#include "Character/BaseCharacter.h"
#include "BasePlayerController.h"
#include "ALSCamera/Public/AlsCameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/BoxComponent.h"

#include "UI/UIElement/InGameHUD.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "Interface/InteractableInterface.h"

//#include "ALS/Public/Utility/AlsVector.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/Utility/AlsVector.h"
//innclude "ALSCamera/Public/AlsCameraComponent.h"

#include "ALS/Public/AlsCharacterMovementComponent.h"
#include "BasePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/Utility/AlsConstants.h"
#include "GameFramework/SpringArmComponent.h"


ABaseCharacter::ABaseCharacter()
{
	bIsPossessed = false;
	bReplicates = true;
	UseGunBoneforOverlayObjects = true;

	/*Overlay Skeletal/Static Mesh for change animation bluprint and item mesh*/
	OverlayStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OverlayStaticMesh"));
	OverlayStaticMesh->SetupAttachment(GetMesh());

	OverlaySkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("OverlaySkeletalMesh"));
	OverlaySkeletalMesh->SetupAttachment(GetMesh());
	
	RemoteOnlySkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSSkeletalMesh"));
	RemoteOnlySkeletalMesh->SetupAttachment(RootComponent);

	RemoteOnlyOverlayStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteOnlyOverlayStaticMesh"));
	RemoteOnlyOverlayStaticMesh->SetupAttachment(RemoteOnlySkeletalMesh);

	RemoteOnlyOverlaySkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RemoteOnlyOverlaySkeletalMesh"));
	RemoteOnlyOverlaySkeletalMesh->SetupAttachment(RemoteOnlySkeletalMesh);

	
	//Camera Settings
	sCamera = CreateDefaultSubobject<UAlsCameraComponent>(TEXT("Camera"));
	sCamera->SetupAttachment(GetMesh()); // Spring Arm에 카메라를 붙임
	sCamera->SetRelativeRotation_Direct(FRotator::ZeroRotator); 
	
	SetViewMode(AlsViewModeTags::FirstPerson);
	
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, CurrentQuickSlotIndex);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Character BeginPlay - Complete  This is Server."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Character BeginPlay - Complete  This is Client."));
	}
	

	CurrentQuickSlotIndex = 0;

	//애니메이션 오버레이 활성화.
	RefreshOverlayObject(CurrentQuickSlotIndex);

	GetWorld()->GetTimerManager().SetTimer(
		InteractionTraceTimerHandle,
		this,
		&ABaseCharacter::TraceInteractableActor,
		0.1f,
		true 
	);
}



void ABaseCharacter::NotifyControllerChanged()
{
	/*
	UE_LOG(LogTemp, Warning, TEXT("Character NotifyControllerChanged"));

	const auto* PreviousPlayer{ Cast<APlayerController>(PreviousController) };
	if (IsValid(PreviousPlayer))
	{
		auto* InputSubsystem{ ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayer->GetLocalPlayer()) };
		if (IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(InputMappingContext);
		}
	}
	
	*/
	auto* NewPlayer{ Cast<APlayerController>(GetController()) };
	ABasePlayerController* PC = Cast<ABasePlayerController>(NewPlayer);
	if (IsValid(PC))
	{
		
		PC->InputYawScale_DEPRECATED = 1.0f;
		PC->InputPitchScale_DEPRECATED = 1.0f;
		PC->InputRollScale_DEPRECATED = 1.0f;
		
		PC->InitInputComponent();
		auto* InputSubsystem{ ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()) };
		if (IsValid(InputSubsystem))
		{
			FModifyContextOptions Options;
			Options.bNotifyUserSettings = true;
			
			InputSubsystem->AddMappingContext(PC->InputMappingContext, 0, Options);
			
		}
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	
	Super::NotifyControllerChanged();
}

void ABaseCharacter::CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo)
{
	
	if (sCamera->IsActive())
	{
		sCamera->GetViewInfo(ViewInfo);
		return;
	}

	Super::CalcCamera(DeltaTime, ViewInfo);
	
}



void ABaseCharacter::Handle_LookMouse(const FInputActionValue& ActionValue)
{

	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	if (!Controller) return;

	// 현재 컨트롤러 회전
	FRotator CurrentRotation = Controller->GetControlRotation();
	float CurrentPitch = CurrentRotation.GetNormalized().Pitch;

	// 입력값 계산
	const float NewPitchInput = Value.Y * LookUpMouseSensitivity;

	// Pitch 제한 적용
	float NewPitch = FMath::Clamp(CurrentPitch + NewPitchInput, MinPitchAngle, MaxPitchAngle);

	// Yaw는 그대로
	float NewYaw = CurrentRotation.Yaw + Value.X * LookRightMouseSensitivity;

	// 새 회전값 적용
	FRotator NewRotation = FRotator(NewPitch, NewYaw, 0.f);
	Controller->SetControlRotation(NewRotation);
}

void ABaseCharacter::Handle_Look(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y * LookUpRate);
	AddControllerYawInput(Value.X * LookRightRate);
}

void ABaseCharacter::Handle_Move(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	const auto Value{ UAlsVector::ClampMagnitude012D(ActionValue.Get<FVector2D>()) };

	const auto ForwardDirection{ UAlsVector::AngleToDirectionXY(UE_REAL_TO_FLOAT(GetViewState().Rotation.Yaw)) };
	const auto RightDirection{ UAlsVector::PerpendicularCounterClockwiseXY(ForwardDirection) };
	if (CheckHardLandState())
	{
		return;
	}
	AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}

void ABaseCharacter::Handle_Sprint(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}

	SetDesiredGait(ActionValue.Get<bool>() ? AlsGaitTags::Sprinting : AlsGaitTags::Running);
	if (GetDesiredGait() == AlsGaitTags::Sprinting)
	{
		ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
		if (PC)
		{
			PC->SetSprintingStateToPlayerState(true);
		}
	}
	else
	{
		ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
		if (PC)
		{
			PC->SetSprintingStateToPlayerState(false);
		}
	}
}

void ABaseCharacter::Handle_SprintOnPlayerState(const FInputActionValue& ActionValue, float multiplier)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}

	SetDesiredGait(ActionValue.Get<bool>() ? AlsGaitTags::Sprinting : AlsGaitTags::Running);
	if (GetDesiredGait() == AlsGaitTags::Sprinting)
	{
		ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
		if (PC)
		{
			PC->SetSprintingStateToPlayerState(true);
		}
	}
	else
	{
		ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
		if (PC)
		{
			PC->SetSprintingStateToPlayerState(false);
		}
	}

}

void ABaseCharacter::Handle_Walk(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}
	SetDesiredGait(ActionValue.Get<bool>() ? AlsGaitTags::Walking : AlsGaitTags::Running);
}

void ABaseCharacter::Handle_Crouch()
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}
	if (GetDesiredStance() == AlsStanceTags::Standing)
	{
		SetDesiredStance(AlsStanceTags::Crouching);
	}
	else if (GetDesiredStance() == AlsStanceTags::Crouching)
	{
		SetDesiredStance(AlsStanceTags::Standing);
	}
}

void ABaseCharacter::Handle_Jump(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}
	if (ActionValue.Get<bool>())
	{
		if (StopRagdolling())
		{
			return;
		}
		if (StartMantlingGrounded())
		{
			return;
		}
		if (GetStance() == AlsStanceTags::Crouching)
		{
			SetDesiredStance(AlsStanceTags::Standing);
			return;
		}

		Jump();
	}
	else
	{
		StopJumping();
	}
}

void ABaseCharacter::Handle_Aim(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}
	SetDesiredAiming(ActionValue.Get<bool>());
}

void ABaseCharacter::Handle_ViewMode()
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	SetViewMode(GetViewMode() == AlsViewModeTags::ThirdPerson ? AlsViewModeTags::FirstPerson : AlsViewModeTags::ThirdPerson);
}


void ABaseCharacter::Handle_Strafe(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	const auto Value{ UAlsVector::ClampMagnitude012D(ActionValue.Get<FVector2D>()) };


}

void ABaseCharacter::Handle_Interact()
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}

	if (!CurrentFocusedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: No focused actor."));
		//Test Code
		PlayInteractionMontage(CurrentFocusedActor);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Interacted with: %s"), *CurrentFocusedActor->GetName());

	if (CurrentFocusedActor->Implements<UInteractableInterface>())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			IInteractableInterface::Execute_Interact(CurrentFocusedActor, PC);
			UE_LOG(LogTemp, Log, TEXT("Handle_Interact: Called Interact on %s"), *CurrentFocusedActor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: Controller is nullptr"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: %s does not implement IInteractableInterface"), *CurrentFocusedActor->GetName());
		PlayInteractionMontage(CurrentFocusedActor);
	}
}

void ABaseCharacter::PickupItem()
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	//TO DO...
	//Play Animation Montage
	//when Animation Montage ended, call NotifyFunc() to getItem
	//add to quickslot
	/*
	AItemBase* HitItem = Cast<AItemBase>(Hit.GetActor());
	if (!HitItem) return;
		아이템 저장 함수
		AddItem(HitItem);
	*/
}


void ABaseCharacter::TraceInteractableActor()
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	if (Controller)
	{
		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		return;
	}

	FVector Start = ViewLocation;
	FVector End = Start + (ViewRotation.Vector() * TraceDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_GameTraceChannel1, Params);

	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f);

	if (bHit && Hit.GetActor() && Hit.GetActor()->Implements<UInteractableInterface>())
	{
		if (CurrentFocusedActor != Hit.GetActor())
		{
			CurrentFocusedActor = Hit.GetActor();

			FString Message = IInteractableInterface::Execute_GetInteractMessage(CurrentFocusedActor);

			if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				if (ULCUIManager* UIManager = Subsystem->GetUIManager())
				{
					if (UInGameHUD* HUD = Cast<UInGameHUD>(UIManager->GetInGameHUD()))
					{
						UE_LOG(LogTemp, Warning, TEXT("SetInteractMessage to %s"), *Message);
						HUD->SetInteractMessage(Message);
						HUD->SetInteractMessageVisible(true);
					}
				}
			}
		}
	}
	else
	{
		if (CurrentFocusedActor)
		{
			CurrentFocusedActor = nullptr;

			if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				if (ULCUIManager* UIManager = Subsystem->GetUIManager())
				{
					if (UInGameHUD* HUD = Cast<UInGameHUD>(UIManager->GetInGameHUD()))
					{
						HUD->SetInteractMessageVisible(false);
					}
				}
			}
		}
	}
}



void ABaseCharacter::SetPossess(bool IsPossessed)
{
	bIsPossessed = IsPossessed;
}


void ABaseCharacter::GetHeldItem()
{
	//TODO: 아이템 반환
	return;
}

int32 ABaseCharacter::GetCurrentQuickSlotIndex()
{
	return CurrentQuickSlotIndex;
}
void ABaseCharacter::SetCurrentQuickSlotIndex(int32 NewIndex)
{
	Server_SetQuickSlotIndex(NewIndex);
}

void ABaseCharacter::Server_SetQuickSlotIndex_Implementation(int32 NewIndex)
{
	int32 AdjustedIndex = NewIndex;
	if (AdjustedIndex > MaxQuickSlotIndex)
	{
		AdjustedIndex = 0;
	}
	else if (AdjustedIndex < 0)
	{
		AdjustedIndex = MaxQuickSlotIndex;
	}

	CurrentQuickSlotIndex = AdjustedIndex;

	// 동기화된 장착 요청
	Multicast_EquipItemFromQuickSlot(AdjustedIndex);
}

void ABaseCharacter::Multicast_EquipItemFromQuickSlot_Implementation(int32 Index)
{
	EquipItemFromCurrentQuickSlot(Index);
}

void ABaseCharacter::EquipItemFromCurrentQuickSlot(int QuickSlotIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Change Equip Item"));

	TestEquipFunction(QuickSlotIndex);
	//TODO: 아이템 교체 로직 추가
	/* // 지금은 아이템 없어서 임시 비활성화.
	if (!QuickSlots.IsValidIndex(QuickSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid quick slot index: %d"), QuickSlotIndex);
		return;
	}

	if (QuickSlotIndex >= QuickSlots.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid quick slot index : %d (Out of Range)"), QuickSlotIndex);
		return;
	}
	UObject* Item = QuickSlots[QuickSlotIndex];
	if (Item)
	{
		EquipItem(Item); // 실제 장착 로직 호출
	}
	else
	{
		UnequipCurrentItem(); // 슬롯이 비어있으면 장착 해제
	}
	*/
}

void ABaseCharacter::EquipItem(UObject* Item)
{
	HeldItem = Item;
	//TODO: 메시 부착, 무기 생성, 이펙트 적용 등 추가 로직 필요
	UE_LOG(LogTemp, Log, TEXT("Equipped item: %s"), *GetNameSafe(Item));
	//TODO: ALS의 attach Component 함수를 참고 혹은 불러오기... 혹은 Overlay모드 변화
}

void ABaseCharacter::UnequipCurrentItem()
{
	HeldItem = nullptr;
	//TODO: 손에서 제거, 메시 해제, 이펙트 제거 등 처리
	UE_LOG(LogTemp, Log, TEXT("Unequipped current item"));
}


float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	UE_LOG(LogTemp, Log, TEXT("Character Take Damage"));

	ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
	if (PC)
	{
		UE_LOG(LogTemp, Log, TEXT("Controller Existed"));
		ABasePlayerState* MyPlayerState = PC->GetPlayerState<ABasePlayerState>();
		if (MyPlayerState)
		{
			UE_LOG(LogTemp, Log, TEXT("State Existed"));
			MyPlayerState->ApplyDamage(DamageAmount);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Character Take Damage End"));
	return DamageAmount;
}

float ABaseCharacter::GetFallDamage(float Velocity)
{
	Super::GetFallDamage(Velocity);
	if (bIsGetFallDownDamage == false)
	{
		return 0;
	}

	if (-Velocity < FallDamageThreshold)
	{
		return 0;
	}

	float FallDamage = (-Velocity - FallDamageThreshold) / 10.0f;
	UE_LOG(LogTemp, Log, TEXT("player Take Fall Damage : %f"), Velocity);
	ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
	if (PC)
	{
		UE_LOG(LogTemp, Log, TEXT("Controller Existed"));
		ABasePlayerState* MyPlayerState = PC->GetPlayerState<ABasePlayerState>();
		if (MyPlayerState)
		{
			UE_LOG(LogTemp, Log, TEXT("State Existed"));
			MyPlayerState->ApplyDamage(FallDamage);
		}
	}
	return FallDamage;
}

void ABaseCharacter::HandlePlayerDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Character is dead"));
	//Camera->SetRelativeRotation_Direct({ 0.0f, 90.0f, 0.0f });
	StartRagdolling();
}

bool ABaseCharacter::CheckHardLandState()
{
	if (bIsInHardLandingState)
	{
		ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
		if (PC)
		{
			PC->SetHardLandStateToPlayerState(true);
		}
		return true;
	}
	else
	{
		ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
		if (PC)
		{
			PC->SetHardLandStateToPlayerState(false);
		}
		return false;
	}
}


EPlayerState ABaseCharacter::CheckPlayerCurrentState()
{
	ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
	if (PC)
	{
		ABasePlayerState* MyPlayerState = PC->GetPlayerState<ABasePlayerState>();
		if (MyPlayerState)
		{
			return MyPlayerState->CurrentState;

		}
	}
	return EPlayerState::None;
}


void ABaseCharacter::SetMovementSetting(float _WalkForwardSpeed, float _WalkBackwardSpeed, float _RunForwardSpeed, float _RunBackwardSpeed, float _SprintSpeed)
{
	AlsCharacterMovement->SetGaitSettings(_WalkForwardSpeed, _WalkBackwardSpeed, _RunForwardSpeed, _RunBackwardSpeed, _SprintSpeed);
}

void ABaseCharacter::ResetMovementSetting()
{
	AlsCharacterMovement->ResetGaitSettings();
}

void ABaseCharacter::TestEquipFunction(int32 NewIndex)
{
	RefreshOverlayObject(NewIndex);
}

void ABaseCharacter::RefreshOverlayObject(int index)
{
	if (index == 0)
	{
		SetDesiredGait(AlsOverlayModeTags::Rifle);
		SetOverlayMode(AlsOverlayModeTags::Rifle);
		RefreshOverlayLinkedAnimationLayer(index);
		AttachOverlayObject(NULL, SKM_Rifle, NULL, "Rifle", false);
	}
	else if (index == 1)
	{
		SetDesiredGait(AlsOverlayModeTags::PistolOneHanded);
		SetOverlayMode(AlsOverlayModeTags::PistolOneHanded);
		RefreshOverlayLinkedAnimationLayer(index);
		AttachOverlayObject(NULL, SKM_Pistol, NULL, "Pistol", false);
	}
	else if (index == 2)
	{
		SetDesiredGait(AlsOverlayModeTags::Bow);
		SetOverlayMode(AlsOverlayModeTags::Bow);
		RefreshOverlayLinkedAnimationLayer(index);
		AttachOverlayObject(SM_Torch, NULL, NULL, "Torch", true);
	}
	else if (index == 3)
	{
		SetDesiredGait(AlsOverlayModeTags::Default);
		RefreshOverlayLinkedAnimationLayer(index);
		OverlayStaticMesh->SetStaticMesh(NULL);
		OverlaySkeletalMesh->SetSkinnedAssetAndUpdate(NULL, true);
		OverlaySkeletalMesh->SetAnimInstanceClass(NULL);
	}

}

void ABaseCharacter::AttachOverlayObject(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh, TSubclassOf<UAnimInstance> NewAnimationClass, FName SocketName, bool bUseLeftGunBone)
{
	FName ResultSocketName;
	FName VirtualBoneName;
	if (bUseLeftGunBone)
	{
		VirtualBoneName = TEXT("VB hand_l_to_ik_hand_gun");
	}
	else
	{
		VirtualBoneName = TEXT("VB hand_r_to_ik_hand_gun");
	}

	if (UseGunBoneforOverlayObjects)
	{
		ResultSocketName = VirtualBoneName;
	}
	else
	{
		ResultSocketName = SocketName;
	}
	FAttachmentTransformRules AttachRules(
		EAttachmentRule::SnapToTarget,  // Location
		EAttachmentRule::SnapToTarget,  // Rotation
		EAttachmentRule::SnapToTarget,  // Scale
		true                            // bWeldSimulatedBodies
	);
	OverlayStaticMesh->SetStaticMesh(NewStaticMesh);
	OverlayStaticMesh->AttachToComponent(GetMesh(), AttachRules, ResultSocketName);

	OverlaySkeletalMesh->SetSkinnedAssetAndUpdate(NewSkeletalMesh, true);
	OverlaySkeletalMesh->SetAnimInstanceClass(NewAnimationClass);
	OverlaySkeletalMesh->AttachToComponent(GetMesh(), AttachRules, ResultSocketName);

	RemoteOnlyOverlayStaticMesh->SetStaticMesh(NewStaticMesh);
	RemoteOnlyOverlayStaticMesh->AttachToComponent(RemoteOnlySkeletalMesh, AttachRules, ResultSocketName);

	RemoteOnlyOverlaySkeletalMesh->SetSkinnedAssetAndUpdate(NewSkeletalMesh, true);
	RemoteOnlyOverlaySkeletalMesh->SetAnimInstanceClass(NewAnimationClass);
	RemoteOnlyOverlaySkeletalMesh->AttachToComponent(RemoteOnlySkeletalMesh, AttachRules, ResultSocketName);
}

void ABaseCharacter::RefreshOverlayLinkedAnimationLayer(int index)
{
	TSubclassOf<UAnimInstance> OverlayAnimationInstanceClass;

	if (index == 0)
	{
		OverlayAnimationInstanceClass = RifleAnimationClass;
	}
	else if (index == 1)
	{
		OverlayAnimationInstanceClass = PistolAnimationClass;
	}
	else if (index == 2)
	{
		OverlayAnimationInstanceClass = TorchAnimationClass;
	}
	else if (index == 3)
	{
		OverlayAnimationInstanceClass = DefaultAnimationClass;;
	}
	else
	{
		OverlayAnimationInstanceClass = DefaultAnimationClass;
	}


	if (IsValid(OverlayAnimationInstanceClass))
	{
		GetMesh()->LinkAnimClassLayers(OverlayAnimationInstanceClass);
		RemoteOnlySkeletalMesh->LinkAnimClassLayers(OverlayAnimationInstanceClass);
	}
	else
	{
		GetMesh()->LinkAnimClassLayers(DefaultAnimationClass);
		RemoteOnlySkeletalMesh->LinkAnimClassLayers(DefaultAnimationClass);
	}
}



/// <summary>
/// 물체 별 인터렉션애니메이션 몽타주 재생 함수
/// </summary>
/// <param name="Target"></param>

void ABaseCharacter::PlayInteractionMontage(AActor* Target)
{
	/*
	if (!Target || !GetMesh() || !GetMesh()->GetAnimInstance())
		return;
		*/

	UAnimMontage* MontageToPlay = InteractMontage;
	// 1. 대상 클래스별로 분기
	/*
	if (Target->IsA(ADoorActor::StaticClass()))
	{
		MontageToPlay = OpenDoorMontage;
	}
	else if (Target->IsA(AChestActor::StaticClass()))
	{
		MontageToPlay = OpenChestMontage;
	}
	else if (Target->ActorHasTag("Tree"))
	{
		MontageToPlay = ChopTreeMontage;
	}
	*/
	// 2. 몽타주 재생
	if (MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("Anim Montage"));
		Server_PlayMontage(MontageToPlay);
		
	}
	
}

void ABaseCharacter::Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay)
{
	Multicast_PlayMontage(MontageToPlay);
}

void ABaseCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(MontageToPlay);
	
	AnimInstance = RemoteOnlySkeletalMesh->GetAnimInstance();
	AnimInstance->Montage_Play(MontageToPlay);
}