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
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/Utility/AlsVector.h"
#include "ALS/Public/AlsCharacterMovementComponent.h"
#include "ALSCamera/Public/AlsCameraSettings.h"
#include "BasePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/Utility/AlsConstants.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"
#include "Item/ItemBase.h"
#include "Item/ItemSpawnerComponent.h"
#include "Item/ResourceNode.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "UI/Manager/LCUIManager.h"
#include "LastCanary.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ArrowComponent.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsAnimationInstance.h"
#include "Character/BaseCharacterAnimNotify.h"
#include "Components/PostProcessComponent.h"

#include "SaveGame/LCLocalPlayerSaveGame.h"

ABaseCharacter::ABaseCharacter()
{
	bIsPossessed = false;
	bReplicates = true;
	UseGunBoneforOverlayObjects = true;

	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetMesh());
	HeadMesh->SetMasterPoseComponent(GetMesh()); // GetMesh()는 전체 메시

	OverlayStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OverlayStaticMesh"));
	OverlayStaticMesh->SetupAttachment(GetMesh());

	OverlaySkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("OverlaySkeletalMesh"));
	OverlaySkeletalMesh->SetupAttachment(GetMesh());

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh(), TEXT("FirstPersonCamera"));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);  // SpringArm에 카메라 부착

	// 초기 방향 설정S
	Camera->SetRelativeRotation(FRotator::ZeroRotator);  // 필요시만 설정

	ThirdPersonArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FirstPersonArrow"));
	ThirdPersonArrow->SetupAttachment(SpringArm);

	ADSSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ADSSpringArm"));
	ADSSpringArm->SetupAttachment(GetMesh(), TEXT("FirstPersonCamera"));

	ADSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ADSCamera"));
	ADSCamera->SetupAttachment(ADSSpringArm);  // SpringArm에 카메라 부착

	SpectatorSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpectatorSpringArm"));
	SpectatorSpringArm->SetupAttachment(GetMesh(), TEXT("SpectatorCamera"));

	SpectatorCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("SpectatorCamera"));
	SpectatorCamera->SetupAttachment(SpectatorSpringArm);  // SpringArm에 카메라 부착

	CustomPostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	CustomPostProcessComponent->bUnbound = false; // 시야 내에서만 적용
	CustomPostProcessComponent->SetupAttachment(Camera); // 카메라에 붙이기


	// 캐릭터 클래스의 생성자 함수 내부 
	FieldOfView = Camera->FieldOfView;

	ItemSpawner = CreateDefaultSubobject<UItemSpawnerComponent>(TEXT("ItemSpawner"));

	BackpackMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackpackMeshComponent"));
	BackpackMeshComponent->SetupAttachment(GetMesh(), TEXT("backpack1"));
	BackpackMeshComponent->SetVisibility(false);
	BackpackMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ToolbarInventoryComponent = CreateDefaultSubobject<UToolbarInventoryComponent>(TEXT("ToolbarInventoryComponent"));

	BackpackInventoryComponent = CreateDefaultSubobject<UBackpackInventoryComponent>(TEXT("BackpackInventoryComponent"));
	BackpackInventoryComponent->MaxSlots = 0;
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, EquippedTags);
	DOREPLIFETIME(ABaseCharacter, bInventoryOpen);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		// "head"는 스켈레탈 메시의 머리 본에 해당하는 이름

		//GetMesh()->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);
		SwapHeadMaterialTransparent(true);
	}
	//애니메이션 오버레이 활성화.
	RefreshOverlayObject(0);


	GetWorld()->GetTimerManager().SetTimer(
		InteractionTraceTimerHandle,
		this,
		&ABaseCharacter::TraceInteractableActor,
		0.1f,
		true
	);

	if (IsValid(ToolbarInventoryComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory Ready"));
		ToolbarInventoryComponent->OnInventoryUpdated.AddDynamic(this, &ABaseCharacter::HandleInventoryUpdated);
	}

	EnableStencilForAllMeshes(2);


	if (IsLocallyControlled() && CustomPostProcessComponent)
	{
		CustomPostProcessComponent->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
		CustomPostProcessComponent->Settings.bOverride_AutoExposureMinBrightness = true;
		CustomPostProcessComponent->Settings.bOverride_AutoExposureMaxBrightness = true;

		// 노출 범위는 0.5~2.0 사이 정도로 잡는 게 적당
		float baseBrightness = FMath::Lerp(1.0f, 10.0f, GetBrightness()); // 0~1 값을 0.5~2.0 범위로 매핑
		CustomPostProcessComponent->Settings.AutoExposureMinBrightness = baseBrightness;
		CustomPostProcessComponent->Settings.AutoExposureMaxBrightness = baseBrightness + 0.5f;

		CustomPostProcessComponent->Settings.bOverride_AutoExposureBias = true;
		CustomPostProcessComponent->Settings.AutoExposureBias = baseBrightness; // 유저 설정값 반영

		// 블렌드 웨이트 1.0으로 보정 적용 보장
		CustomPostProcessComponent->BlendWeight = 1.0f;
	}
}

float ABaseCharacter::GetBrightness()
{
	ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
	if (!IsValid(PC))
	{
		return 1.0f;
	}
	return PC->BrightnessSetting;
}

void ABaseCharacter::SetBrightness(float Value)
{
	float baseBrightness = FMath::Lerp(1.0f, 10.0f, Value); // 0~1 값을 0.5~2.0 범위로 매핑
	CustomPostProcessComponent->Settings.AutoExposureMinBrightness = baseBrightness;
	CustomPostProcessComponent->Settings.AutoExposureMaxBrightness = baseBrightness + 0.5f;
	CustomPostProcessComponent->Settings.AutoExposureBias = baseBrightness; // 유저 설정값 반영
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


		ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
		if (IsValid(MyPlayerState))
		{
			SetMovementSetting();
			UE_LOG(LogTemp, Warning, TEXT("플레이어 무브먼트 세팅 초기화 성공"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("플레이어 무브먼트 세팅 초기화 실패"));
		}
	}

	Super::NotifyControllerChanged();
}

void ABaseCharacter::CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo)
{
	if (Controller && Controller->IsLocalPlayerController())
	{
		// 내가 조종 중이라면 → 1인칭 시점
		Camera->GetCameraView(DeltaTime, ViewInfo);
	}
	else
	{
		// 관전자가 바라볼 경우 → 3인칭 시점
		SpectatorCamera->GetCameraView(DeltaTime, ViewInfo);
		//GetMesh()->SetOwnerNoSee(false);
		//GetMesh()->UnHideBoneByName(TEXT("head"));
	}
}

void ABaseCharacter::Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity)
{
	if (LocomotionAction == AlsLocomotionActionTags::Mantling)
	{
		return;
	}
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	if (!Controller) return;

	// 현재 컨트롤러 회전
	FRotator CurrentRotation = Controller->GetControlRotation();
	float CurrentPitch = CurrentRotation.GetNormalized().Pitch;

	// 입력값 계산
	const float NewPitchInput = Value.Y * Sensivity;

	// Pitch 제한 적용
	float NewPitch = FMath::Clamp(CurrentPitch + NewPitchInput, MinPitchAngle, MaxPitchAngle);

	// Yaw는 그대로
	float NewYaw = CurrentRotation.Yaw + Value.X * Sensivity;

	// 새 회전값 적용
	FRotator NewRotation = FRotator(NewPitch, NewYaw, 0.f);
	Controller->SetControlRotation(NewRotation);
}

void ABaseCharacter::CameraShake(float Vertical, float Horizontal)
{
	// 새로운 반동량을 기존 값에 누적
	RecoilStepPitch += Vertical / RecoilMaxSteps;
	RecoilStepYaw += FMath::RandRange(-Horizontal, Horizontal) / RecoilMaxSteps;

	// 타이머가 안 돌고 있을 때만 시작
	if (!GetWorld()->GetTimerManager().IsTimerActive(RecoilTimerHandle))
	{
		RecoilStep = 0;
		GetWorld()->GetTimerManager().SetTimer(RecoilTimerHandle, this, &ABaseCharacter::ApplyRecoilStep, 0.02f, true);
	}
}

void ABaseCharacter::ApplyRecoilStep()
{
	if (!Controller) return;

	FRotator ControlRotation = Controller->GetControlRotation();
	float CurrentPitch = ControlRotation.Pitch;

	// Unreal에서는 Pitch가 0~360도로 표현될 수 있으므로 정규화
	if (CurrentPitch > 180.f)
		CurrentPitch -= 360.f;

	// Pitch 제한 확인
	if (CurrentPitch + RecoilStepPitch < MaxPitchAngle)
	{
		AddControllerPitchInput(RecoilStepPitch);
	}
	else
	{
		// 최대 각도를 초과하지 않도록 필요한 만큼만 보정하여 추가
		float RemainingPitch = MaxPitchAngle - CurrentPitch;
		if (RemainingPitch > 0.0f)
		{
			AddControllerPitchInput(RemainingPitch);
		}
	}

	// Yaw는 제한 없이 계속 적용
	AddControllerYawInput(RecoilStepYaw);

	RecoilStep++;

	if (RecoilStep >= RecoilMaxSteps)
	{
		RecoilStepPitch = 0.0f;
		RecoilStepYaw = 0.0f;
		GetWorld()->GetTimerManager().ClearTimer(RecoilTimerHandle);
	}
}


void ABaseCharacter::Handle_Look(const FInputActionValue& ActionValue)
{
	/*  Deprecated
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (IsValid(CurrentInteractMontage))
	{
		return;
	}
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y);
	AddControllerYawInput(Value.X);
	*/
}

void ABaseCharacter::Handle_Move(const FInputActionValue& ActionValue)
{
	const auto Value{ UAlsVector::ClampMagnitude012D(ActionValue.Get<FVector2D>()) };
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	CancelInteraction();
	FrontInput = Value.Y;
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
	const float Value = ActionValue.Get<float>();
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}

	//만약 지친 상태라면 불가
	if (MyPlayerState->MovementState == ECharacterMovementState::Exhausted)
	{
		return;
	}
	//만약 스테미나가 0이어도 불가 // 위에 조건이랑 같긴 할텐데 혹시 모르니까
	if (!HasStamina())
	{
		return;
	}
	if (MyPlayerState->SprintInputMode == EInputMode::Hold)
	{
		//입력이 떼지는 거면 어차피 뛰는 거 아님..
		if (Value < 0.5f)
		{
			bIsSprinting = false;
			FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
			SetDesiredGait(AlsGaitTags::Running);
			StopStaminaDrain();
			StartStaminaRecoverAfterDelay();
			return;
		}
		FootSoundModifier = MyPlayerState->SprintingFootSoundModifier;

		//달리기 시작하면서 스테미나 소모 시작
		StartStaminaDrain();
		StopStaminaRecovery();
		StopStaminaRecoverAfterDelay();
	}
	else if (MyPlayerState->SprintInputMode == EInputMode::Toggle)
	{
		if (Value > 0.5f)
		{
			if (GetDesiredGait() == AlsGaitTags::Sprinting)
			{
				bIsSprinting = false;
				FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
				SetDesiredGait(AlsGaitTags::Running);
				StopStaminaDrain();
				StartStaminaRecoverAfterDelay();
			}
			else if (GetDesiredGait() == AlsGaitTags::Running)
			{
				//bIsSprinting = true;
				FootSoundModifier = MyPlayerState->SprintingFootSoundModifier;
				//SetDesiredAiming(false);
				Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				Camera->SetRelativeLocation(FVector::ZeroVector);
				Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
				StopStaminaRecovery();
				StopStaminaRecoverAfterDelay();
				StartStaminaDrain();
				//SetDesiredGait(AlsGaitTags::Sprinting);
			}
			else
			{
				//bIsSprinting = true;
				FootSoundModifier = MyPlayerState->SprintingFootSoundModifier;
				//SetDesiredAiming(false);
				Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				Camera->SetRelativeLocation(FVector::ZeroVector);
				Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
				StopStaminaRecovery();
				StopStaminaRecoverAfterDelay();
				StartStaminaDrain();
				//SetDesiredGait(AlsGaitTags::Sprinting);
			}
		}
	}
}

void ABaseCharacter::Handle_Walk(const FInputActionValue& ActionValue)
{
	const float Value = ActionValue.Get<float>();
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}

	if (MyPlayerState->WalkInputMode == EInputMode::Hold)
	{
		if (Value > 0.5f)
		{
			FootSoundModifier = MyPlayerState->WalkingFootSoundModifier;
			SetDesiredGait(AlsGaitTags::Walking);
		}
		else
		{
			FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
			SetDesiredGait(AlsGaitTags::Running);
		}
	}
	else if (MyPlayerState->WalkInputMode == EInputMode::Toggle)
	{
		if (Value > 0.5f)
		{
			if (GetDesiredGait() == AlsGaitTags::Walking)
			{
				FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
				SetDesiredGait(AlsGaitTags::Running);
			}
			else if (GetDesiredGait() == AlsGaitTags::Running)
			{
				FootSoundModifier = MyPlayerState->WalkingFootSoundModifier;
				SetDesiredGait(AlsGaitTags::Walking);
			}
			else
			{
				FootSoundModifier = MyPlayerState->WalkingFootSoundModifier;
				SetDesiredGait(AlsGaitTags::Running);
			}
		}
	}
}

void ABaseCharacter::Handle_Crouch(const FInputActionValue& ActionValue)
{
	CancelInteraction();
	const float Value = ActionValue.Get<float>();
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}

	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (MyPlayerState->CrouchInputMode == EInputMode::Hold)
	{
		if (Value > 0.5f)
		{
			SetDesiredStance(AlsStanceTags::Crouching);
		}
		else
		{
			SetDesiredStance(AlsStanceTags::Standing);
		}
	}
	else if (MyPlayerState->CrouchInputMode == EInputMode::Toggle)
	{
		if (Value > 0.5f)
		{
			if (GetDesiredStance() == AlsStanceTags::Standing)
			{
				SetDesiredStance(AlsStanceTags::Crouching);
			}
			else if (GetDesiredStance() == AlsStanceTags::Crouching)
			{
				SetDesiredStance(AlsStanceTags::Standing);
			}
		}
	}


}

void ABaseCharacter::Handle_Jump(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}
	CancelInteraction();
	if (ActionValue.Get<bool>())
	{
		if (StopRagdolling())
		{
			return;
		}
		if (StartMantlingGrounded())
		{
			SetDesiredAiming(false);
			SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
			return;
		}
		if (GetStance() == AlsStanceTags::Crouching)
		{
			SetDesiredStance(AlsStanceTags::Standing);
			return;
		}
		ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
		if (!IsValid(MyPlayerState))
		{
			return;
		}
		if (MyPlayerState->MovementState == ECharacterMovementState::Exhausted)
		{
			return;
		}
		if (MyPlayerState->GetStamina() >= MyPlayerState->InitialStats.JumpStaminaCost)
		{
			Jump();
			if (!CanJump())
			{
				return;
			}
			if (MyPlayerState->bInfiniteStamina == true)
			{
				return;
			}
			float Stamina = FMath::Clamp(MyPlayerState->GetStamina() - MyPlayerState->InitialStats.JumpStaminaCost, 0.f, MyPlayerState->InitialStats.MaxStamina);
			MyPlayerState->SetStamina(Stamina);
			StopStaminaRecovery();
			StartStaminaRecoverAfterDelayOnJump();
		}
	}
	else
	{
		StopJumping();
	}
}

void ABaseCharacter::Handle_Aim(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (CheckHardLandState())
	{
		return;
	}
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!EquippedItem)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] 현재 장착된 아이템이 없습니다."));
		return;
	}
	if (bIsSprinting)
	{
		return;
	}
	if (bIsReloading)
	{
		return; // 리로드 중이므로 줌 입력 무시
	}
	if (bIsClose)
	{
		return;
	}
	if (bIsMantling)
	{
		return;
	}
	if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem))
	{
		if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
		{
			AGunBase* RifleItem = Cast<AGunBase>(EquippedItem);
			if (RifleItem)
			{
				USkeletalMeshComponent* RifleMesh = RifleItem->GetSkeletalMeshComponent();
				if (!RifleMesh)
				{
					UE_LOG(LogTemp, Warning, TEXT("Handle_Aim: No SkeletalMeshComponent found on rifle"));
					return;
				}

				if (ActionValue.Get<float>() > 0.5f && bIsCloseToWall == false)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Scope on"));
					CancelInteraction();
					// 1. 소켓 위치와 회전 가져오기
					FTransform ScopeTransform = RifleMesh->GetSocketTransform(TEXT("Scope"));
					Camera->AttachToComponent(RifleMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Scope"));
					Camera->SetWorldRotation(RifleMesh->GetSocketRotation(TEXT("Scope")));
					//ToADSCamera(true);
					return;
				}
				else
				{
					//UE_LOG(LogTemp, Warning, TEXT("Scope out"));
					// 예: 조준 해제 시
					Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					Camera->SetRelativeLocation(FVector::ZeroVector);
					Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
					//ToADSCamera(false);
					return;
				}
			}
		}
	}

	SetDesiredAiming(ActionValue.Get<bool>());
}


void ABaseCharacter::StartStaminaDrain()
{
	TickStaminaDrain();
	if (!GetWorldTimerManager().IsTimerActive(StaminaDrainHandle))
	{
		GetWorldTimerManager().SetTimer(
			StaminaDrainHandle,
			this,
			&ABaseCharacter::TickStaminaDrain,
			0.1f,
			true);
	}
}

void ABaseCharacter::TickStaminaDrain()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (IsValid(MyPlayerState))
	{
		//플레이어 스테이트에 있는 스테미나를 계속 까기.
		ConsumeStamina();
	}
}

void ABaseCharacter::StopStaminaDrain()
{
	//스테미나 소모 중지
	GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
}

void ABaseCharacter::StartStaminaRecovery()
{
	if (!GetWorldTimerManager().IsTimerActive(StaminaRecoveryHandle))
	{
		GetWorldTimerManager().SetTimer(
			StaminaRecoveryHandle,
			this,
			&ABaseCharacter::TickStaminaRecovery,
			0.1f,
			true);
	}
}

void ABaseCharacter::TickStaminaRecovery()
{
	//스테미나가 가득 차 있으면 중지
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (MyPlayerState->GetStamina() >= MyPlayerState->InitialStats.ExhaustionRecoveryThreshold)
	{
		MyPlayerState->SetPlayerMovementState(ECharacterMovementState::Walking);
	}
	if (IsStaminaFull())
	{
		MyPlayerState->SetPlayerMovementState(ECharacterMovementState::Walking);
		StopStaminaRecovery();
		return;
	}
	float Stamina = FMath::Clamp(MyPlayerState->CurrentStamina + MyPlayerState->InitialStats.StaminaRecoveryRate * 0.1f, 0.f, MyPlayerState->InitialStats.MaxStamina);
	MyPlayerState->SetStamina(Stamina);
}

void ABaseCharacter::StopStaminaRecovery()
{
	//스태미나 회복 중지
	GetWorldTimerManager().ClearTimer(StaminaRecoveryHandle);
}

void ABaseCharacter::StartStaminaRecoverAfterDelay()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (GetWorldTimerManager().IsTimerActive(StaminaRecoveryDelayHandle)) //이미 발동되었으면 넘기기
	{
		return;
	}

	//몇초 뒤에 실행할 건지
	GetWorldTimerManager().SetTimer(StaminaRecoveryDelayHandle, this, &ABaseCharacter::StartStaminaRecovery, MyPlayerState->InitialStats.RecoverDelayTime, false);
}

void ABaseCharacter::StartStaminaRecoverAfterDelayOnJump()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}

	//몇초 뒤에 실행할 건지
	GetWorldTimerManager().SetTimer(StaminaRecoveryDelayHandle, this, &ABaseCharacter::StartStaminaRecovery, MyPlayerState->InitialStats.RecoverDelayTime, false);
}

void ABaseCharacter::StopStaminaRecoverAfterDelay()
{
	//몇초 뒤에 실행할 건지
	GetWorldTimerManager().ClearTimer(StaminaRecoveryDelayHandle);
}


void ABaseCharacter::ConsumeStamina()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (MyPlayerState->bInfiniteStamina == true)
	{
		return;
	}
	float CurrentPlayerSpeed = GetPlayerMovementSpeed();
	if (FrontInput < 0.1f)
	{
		bIsSprinting = false;
		SetDesiredGait(AlsGaitTags::Running);
		//일단 회복 시키기는 해
		StartStaminaRecoverAfterDelay();
		return;
	}
	bIsSprinting = true;
	SetDesiredAiming(false);
	SetDesiredGait(AlsGaitTags::Sprinting);
	Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Camera->SetRelativeLocation(FVector::ZeroVector);
	Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
	StopStaminaRecovery();
	StopStaminaRecoverAfterDelay();
	StartStaminaRecoverAfterDelayOnJump();
	float Amount = MyPlayerState->InitialStats.StaminaDrainRate * 0.1f;
	float Stamina = FMath::Clamp(MyPlayerState->CurrentStamina - Amount, 0.f, MyPlayerState->InitialStats.MaxStamina);
	MyPlayerState->SetStamina(Stamina);

	if (MyPlayerState->CurrentStamina <= 0.f)
	{
		MyPlayerState->SetPlayerMovementState(ECharacterMovementState::Exhausted);
		bIsSprinting = false;
		SetDesiredAiming(true);
		SetDesiredGait(AlsGaitTags::Running);
		StopStaminaDrain();
		StartStaminaRecoverAfterDelay();
	}
}

bool ABaseCharacter::HasStamina() const
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return false;
	}
	return MyPlayerState->CurrentStamina > 0.f;
}

bool ABaseCharacter::IsStaminaFull() const
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		//혹시 몰라서 일단 타이머는 끌 수 있게
		return true;
	}
	return MyPlayerState->GetStamina() >= MyPlayerState->InitialStats.MaxStamina;
}

float ABaseCharacter::GetPlayerMovementSpeed()
{
	float XSpeed = GetLocomotionStateMovementSpeed().X;
	float YSpeed = GetLocomotionStateMovementSpeed().Y;

	//플레이어의 앞뒤좌우 움직임의 속도를 가져오는 부분
	FVector2D XYSpeed(XSpeed, YSpeed);
	float SpeedXY = XYSpeed.Size();
	return SpeedXY;
}


void ABaseCharacter::ToADSCamera(bool bToADS)
{
	SmoothCameraCurrentTime = 0.f;
	if (bToADS)
	{
		bDesiredADS = true;
		//ADSCamera->SetActive(true);
		//Camera->SetActive(false);
		// 카메라를 FirstPerson에서 ADS로 변경
		//SetViewMode(AlsViewModeTags::FirstPerson);

		if (IsValid(OverlaySkeletalMesh))
		{
			OverlaySkeletalMesh->SetVisibility(false);
		}
		if (GetMesh())
		{
			GetMesh()->SetVisibility(false);
		}
		/*
		if (IsValid(ADSSkeletalMesh))
		{
			ADSSkeletalMesh->SetVisibility(true);
		}
		*/
	}
	else
	{
		// 카메라를 ADS에서 ThirdPerson으로 변경
		bDesiredADS = false;
		//ADSCamera->SetActive(false);
		//Camera->SetActive(true);
		//SetViewMode(AlsViewModeTags::ThirdPerson);

		if (IsValid(OverlaySkeletalMesh))
		{
			OverlaySkeletalMesh->SetVisibility(true);
		}
		if (GetMesh())
		{
			GetMesh()->SetVisibility(true);
		}
		/*
		if (IsValid(ADSSkeletalMesh))
		{
			ADSSkeletalMesh->SetVisibility(false);
		}
		*/
	}
}

void ABaseCharacter::SmoothADSCamera(float DeltaTime)
{
	if (bDesiredADS && !bADS)
	{
		SmoothCameraCurrentTime += DeltaTime;
		// 카메라를 ThirdPerson에서 ADS로 변경
		if (IsValid(Camera) && IsValid(ADSCamera))
		{
			FVector NewLocation = UKismetMathLibrary::VInterpTo(Camera->GetComponentLocation(), ADSCamera->GetComponentLocation(), DeltaTime, SmoothCameraSpeed);
			Camera->SetWorldLocation(NewLocation);

			float NewFOV = UKismetMathLibrary::FInterpTo(Camera->FieldOfView, ADSCamera->FieldOfView, DeltaTime, SmoothCameraSpeed);
			Camera->SetFieldOfView(NewFOV);

			if (UKismetMathLibrary::VSize(Camera->GetComponentLocation() - ADSCamera->GetComponentLocation()) < 0.5f ||
				SmoothCameraCurrentTime >= SmoothCameraTimeThreshold)
			{
				// 변경 완료
				SmoothCameraCurrentTime = 0.f;
				bADS = true;

				Camera->SetActive(false);
				ADSCamera->SetActive(true);
				/*
				if (IsValid(ADSSkeletalMesh))
				{
					ADSSkeletalMesh->SetVisibility(true);
				}
				*/
			}
		}
	}
	else if (!bDesiredADS && bADS)
	{
		SmoothCameraCurrentTime += DeltaTime;
		if (IsValid(Camera) && IsValid(ADSCamera))
		{
			// 카메라를 ADS에서 ThirdPerson으로 변경
			FVector NewLocation = UKismetMathLibrary::VInterpTo(Camera->GetComponentLocation(), ThirdPersonArrow->GetComponentLocation(), DeltaTime, SmoothCameraSpeed);
			Camera->SetWorldLocation(NewLocation);

			float NewFOV = UKismetMathLibrary::FInterpTo(Camera->FieldOfView, FieldOfView, DeltaTime, SmoothCameraSpeed);
			Camera->SetFieldOfView(NewFOV);

			if (UKismetMathLibrary::VSize(Camera->GetComponentLocation() - ThirdPersonArrow->GetComponentLocation()) < 0.5f ||
				SmoothCameraCurrentTime >= SmoothCameraTimeThreshold)
			{
				// 변경 완료
				Camera->SetFieldOfView(FieldOfView);
				SmoothCameraCurrentTime = 0.f;
				bADS = false;
			}
		}
	}
}

void ABaseCharacter::StartSmoothMove(const FVector& Start, const FVector& Destination)
{
	StartLocation = Start;
	TargetLocation = Destination;

	// 0.01초 간격으로 반복 (100fps 수준)
	GetWorldTimerManager().SetTimer(MoveTimerHandle, this, &ABaseCharacter::SmoothMoveStep, 0.01f, true);
}

void ABaseCharacter::SmoothMoveStep()
{
	FVector Current = Camera->GetComponentLocation();
	FVector New = FMath::VInterpTo(Current, TargetLocation, 0.01f, InterpSpeed); // 고정된 DeltaTime 사용

	if (FVector::Dist(New, TargetLocation) < SnapTolerance)
	{
		Camera->SetWorldLocation(TargetLocation);
		GetWorldTimerManager().ClearTimer(MoveTimerHandle); // 타이머 중지
		SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocketName);
		return;
	}

	Camera->SetWorldLocation(New);
}

void ABaseCharacter::CalcCameraLocation()
{

}

void ABaseCharacter::Handle_Reload()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (bIsUsingItem)
	{
		return;
	}
	CancelInteraction();
	Server_PlayReload();
}

void ABaseCharacter::Server_PlayReload_Implementation()
{
	Multicast_PlayReload();
}

void ABaseCharacter::Multicast_PlayReload_Implementation()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance on server: %s"), *GetNameSafe(GetMesh()->GetAnimInstance()));
	}
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!EquippedItem)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] 현재 장착된 아이템이 없습니다."));
		return;
	}
	AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem);
	if (!IsValid(EquipmentItem))
	{
		return;
	}
	if (EquipmentItem->ItemData.ItemType != FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
	{
		return;
	}
	AGunBase* RifleItem = Cast<AGunBase>(EquippedItem);
	UAnimMontage* MontageToPlay = ReloadMontage;

	if (!(MontageToPlay && RifleItem && GetMesh()))
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}


	float Duration = AnimInstance->Montage_Play(MontageToPlay, 1.0f);
	if (Duration > 0.f)
	{
		RifleItem->Multicast_PlayReloadAnimation_Implementation();
		SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
		bIsReloading = true;
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ABaseCharacter::OnGunReloadAnimComplete);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
	}
}

void ABaseCharacter::OnGunReloadAnimComplete(UAnimMontage* CompletedMontage, bool bInterrupted)
{
	bIsReloading = false; // <- 리로드 끝났으므로 플래그 해제
	if (bInterrupted)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::OnGunReloadAnimComplete] 애니메이션이 중단되었습니다."));
		return;
	}

	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (AGunBase* Gun = Cast<AGunBase>(EquippedItem))
	{
		Gun->Reload();
	}
}


void ABaseCharacter::Handle_ViewMode()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	bIsFPSCamera = !bIsFPSCamera;
	SetCameraMode(bIsFPSCamera);
}


void ABaseCharacter::SetCameraMode(bool bIsFirstPersonView)
{
	if (bIsFirstPersonView)
	{
		//GetMesh()->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);
		SwapHeadMaterialTransparent(true);
		SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
		SpringArm->TargetArmLength = 0.0f;
	}
	else
	{
		//GetMesh()->UnHideBoneByName(TEXT("head"));
		SwapHeadMaterialTransparent(false);
		SpringArm->TargetArmLength = 200.0f;
	}
}

void ABaseCharacter::SwapHeadMaterialTransparent(bool bUseTransparent)
{
	if (!GetMesh()) return;

	if (bUseTransparent && TransparentHeadMaterial)
	{
		GetMesh()->SetMaterial(7, TransparentHeadMaterial);
		GetMesh()->SetMaterial(9, TransparentHeadMaterial);
		GetMesh()->SetMaterial(10, TransparentHeadMaterial);
		GetMesh()->SetMaterial(11, TransparentHeadMaterial);
		GetMesh()->SetMaterial(12, TransparentHeadMaterial);
		GetMesh()->SetMaterial(13, TransparentHeadMaterial);
		GetMesh()->SetMaterial(14, TransparentHeadMaterial);
		GetMesh()->SetMaterial(15, TransparentHeadMaterial);
	}
	else
	{
		GetMesh()->SetMaterial(7, DefaultHeadMaterial_HelmBoots);
		GetMesh()->SetMaterial(9, DefaultHeadMaterial_HelmBoots_Glassess);
		GetMesh()->SetMaterial(11, DefaultHeadMaterial_Teeth);
		GetMesh()->SetMaterial(12, DefaultHeadMaterial_Body);
		GetMesh()->SetMaterial(13, DefaultHeadMaterial_Eyelash);
		GetMesh()->SetMaterial(14, DefaultHeadMaterial_CORNEA);
		GetMesh()->SetMaterial(15, DefaultHeadMaterial_EYEBALL);
	}
	// 7 10 11  13  14 15 
}

void ABaseCharacter::Handle_Strafe(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
}

void ABaseCharacter::Handle_Interact(const FInputActionValue& ActionValue)
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}

	if (!CurrentFocusedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: No focused actor."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Interacted with: %s"), *CurrentFocusedActor->GetName());

	if (CurrentFocusedActor->Implements<UInteractableInterface>())
	{
		AActor* actor = CurrentFocusedActor;
		if (!IsValid(actor))
		{
			return;
		}
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			CancelInteraction();
			//IInteractableInterface::Execute_Interact(CurrentFocusedActor, PC);
			UE_LOG(LogTemp, Log, TEXT("Handle_Interact: Called Interact on %s"), *actor->GetName());
			UE_LOG(LogTemp, Log, TEXT("Equipped item on slot"));
			InteractAfterPlayMontage(actor);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: Controller is nullptr"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: %s does not implement IInteractableInterface"), *CurrentFocusedActor->GetName());
	}

	UE_LOG(LogTemp, Log, TEXT("Interact Ended"));
}

void ABaseCharacter::InteractAfterPlayMontage(AActor* TargetActor)
{
	UAnimMontage* MontageToPlay = nullptr;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}
	InteractTargetActor = TargetActor;
	if (InteractTargetActor->Tags.Contains("Roll"))
	{
		MontageToPlay = OpeningValveMontage;
	}
	else if (InteractTargetActor->Tags.Contains("Kick"))
	{
		MontageToPlay = KickMontage;
	}
	else if (InteractTargetActor->Tags.Contains("Press"))
	{
		MontageToPlay = PressButtonMontage;
	}
	else
	{
		//당장 태그 없는 거 빠르게 테스트 하기 위해서 넣어놨습니다.
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			if (!IsValid(InteractTargetActor))
			{
				return;
			}
			UE_LOG(LogTemp, Warning, TEXT("excute interact"));
			IInteractableInterface::Execute_Interact(InteractTargetActor, PC);
		}
	}
	if (!IsValid(MontageToPlay))
	{
		UE_LOG(LogTemp, Warning, TEXT("Anim Montage does not exist."));
		return;
	}
	CurrentInteractMontage = MontageToPlay;
	Server_PlayMontage(MontageToPlay);
}

void ABaseCharacter::CancelInteraction()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CurrentInteractMontage)
	{
		AnimInstance->Montage_Stop(0.2f, CurrentInteractMontage); // 부드럽게 블렌드 아웃
		Server_CancelInteraction();
	}
}

void ABaseCharacter::Server_CancelInteraction_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CurrentInteractMontage)
	{
		AnimInstance->Montage_Stop(0.2f, CurrentInteractMontage); // 부드럽게 블렌드 아웃
	}
}

void ABaseCharacter::OnNotified()
{
	//재생 후 notify로
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!IsValid(PC))
	{
		return;
	}
	if (!IsValid(InteractTargetActor))
	{
		return;
	}
	IInteractableInterface::Execute_Interact(InteractTargetActor, PC);
	//		InteractTargetActor = nullptr;
}


void ABaseCharacter::PlayInteractionMontage(AActor* Target)
{
	UAnimMontage* MontageToPlay;
	//TODO: 게임 플레이 태그 비교
	//if(Target->GetGamePlayTag)



	/*
	if(Target->Tags.Contains("Gimmick"))
	{

	}
	*/
	MontageToPlay = InteractMontageOnUnderObject;
	if (!IsValid(MontageToPlay))
	{
		UE_LOG(LogTemp, Warning, TEXT("Anim Montage does not exist."));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Anim Montage"));
	Server_PlayMontage(MontageToPlay);
}

void ABaseCharacter::Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay)
{
	Multicast_PlayMontage(MontageToPlay);
}

void ABaseCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(MontageToPlay);
	CurrentInteractMontage = MontageToPlay;
}

void ABaseCharacter::PickupItem()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
}


void ABaseCharacter::TraceInteractableActor()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (!bIsSprinting)
	{
		SetDesiredAiming(true);
	}
	
	SetRotationMode(AlsRotationModeTags::Aiming);
	if (!IsLocallyControlled())
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
	FVector Start;
	FVector End;
	if (bIsFPSCamera)
	{
		Start = ViewLocation;
		End = Start + (ViewRotation.Vector() * TraceDistance);
	}
	else
	{
		Start = ViewLocation;
		End = Start + (ViewRotation.Vector() * TraceDistance * 3);
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		float DistanceToHit = Hit.Distance;
		if (Hit.Distance < 100.0f)
		{
			bIsCloseToWall = true;
		}
		else
		{
			bIsCloseToWall = false;
		}
	}
	else
	{
		bIsCloseToWall = false;
	}


	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f);
	//여기가 로그가 안찍힘 수정해야됨

	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (EquippedItem)
	{
		AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem);
		if (IsValid(EquipmentItem))
		{
			if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
			{
				AGunBase* RifleItem = Cast<AGunBase>(EquippedItem);

				if (bHit)  // 레이가 맞으면서 맞은 대상이 벽인 경우를 추가하거나, 캐릭터의 캡슐 콜라이더가 닿았을 때로 조건을 변경하는 것도...
				{
				}
			}
		}
	}



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

void ABaseCharacter::UpdateGunWallClipOffset(float DeltaTime)
{
	// 1. 총을 들고 있는 상태인지 확인 (OverlayState or 커스텀 상태)

	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!IsValid(EquippedItem))
	{
		WallClipAimOffsetPitch = 0.0f;
		return;
	}

	AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem);
	if (!IsValid(EquipmentItem))
	{
		return;
	}
	if (EquipmentItem->ItemData.ItemType != FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
	{
		return;
	}
	AGunBase* RifleItem = Cast<AGunBase>(EquippedItem);
	if (!IsValid(RifleItem))
	{
		return;
	}

	USkeletalMeshComponent* RifleMesh = RifleItem->GetSkeletalMeshComponent();
	if (!IsValid(RifleMesh))
	{
		return;
	}

	FVector MuzzleLoc = RifleMesh->GetSocketLocation("Muzzle");
	FTransform MuzzleTransform = RifleMesh->GetSocketTransform("Muzzle", RTS_World);

	// 1. 머즐의 앞 방향과 Pitch 각도 얻기
	FVector MuzzleForward = MuzzleTransform.GetUnitAxis(EAxis::Z); // 머즐의 "앞" 방향
	FRotator MuzzleRot = MuzzleForward.Rotation();
	float MuzzlePitch = MuzzleRot.Pitch;  // 상하 방향 판별용

	// 2. 라인 트레이스
	static constexpr float GunWallTraceDistance = 50.0f; // 50에서 100으로 더 여유있게
	FVector TraceEnd = MuzzleLoc + MuzzleForward * GunWallTraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, MuzzleLoc, TraceEnd, ECC_Visibility, Params);

	DrawDebugLine(GetWorld(), MuzzleLoc, TraceEnd, FColor::Red, false, 0.1f);
	// 3. 벽과의 거리 비율 계산
	//float WallRatio = 0.0f;
	//if (bHit)
	//{
	//	float Dist = (Hit.Location - MuzzleLoc).Size();
	//	WallRatio = 1.0f - (Dist / 30.0f); // 30cm 안으로 들어가면 1.0
	//	WallRatio = FMath::Clamp(WallRatio, 0.0f, 1.0f);
	//}

	static constexpr float WallClipTriggerDistance = 60.0f;
	float TargetWallRatio = 0.0f;

	if (bHit)
	{
		float Dist = (Hit.Location - MuzzleLoc).Size();
		if (Dist < WallClipTriggerDistance)
		{
			TargetWallRatio = 1.0f - (Dist / WallClipTriggerDistance);
			TargetWallRatio = FMath::Clamp(TargetWallRatio, 0.0f, 1.0f);
		}
	}

	// WallRatio 보간 (떨림 방지 핵심)
	static float SmoothedWallRatio = 0.0f; // 내부 상태 유지
	SmoothedWallRatio = FMath::FInterpTo(SmoothedWallRatio, TargetWallRatio, DeltaTime, 3.0f);


	// 4. Pitch 보정값 계산 (상하 방향에 따라 부호 바꿈)
	float DirectionSign = MuzzlePitch >= 0 ? 1.0f : -1.0f;  // 위를 보면 +, 아래를 보면 -
	float TargetOffset = FMath::Lerp(0.0f, MaxWallClipPitch, SmoothedWallRatio) * DirectionSign;

	// 5. 부드러운 보간
	WallClipAimOffsetPitch = FMath::FInterpTo(WallClipAimOffsetPitch, TargetOffset, DeltaTime, 10.0f);

	// 6. 애님 인스턴스에 전달
	if (UAlsAnimationInstance* AlsAnim = Cast<UAlsAnimationInstance>(GetMesh()->GetAnimInstance()))
	{
		AlsAnim->WallClipAimOffsetPitch = WallClipAimOffsetPitch;
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

void ABaseCharacter::SetCurrentQuickSlotIndex(int32 NewIndex)
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Request Server to change QuickSlotindex"));
	Server_SetQuickSlotIndex(NewIndex);
}

void ABaseCharacter::Server_SetQuickSlotIndex_Implementation(int32 NewIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("change QuickSlotindex on Server"));
	if (!IsValid(ToolbarInventoryComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("툴바 없음."));
		return;
	}
	int32 AdjustedIndex = NewIndex;
	if (AdjustedIndex > 3) // 툴바의 최대 슬롯 개수 가져오는 함수는?
	{
		AdjustedIndex = 0;
	}
	else if (AdjustedIndex < 0)
	{
		AdjustedIndex = 3;
	}

	//if(ToolbarInventoryComponent->GetCurrentEquippedSlotIndex() == AdjustedIndex )  //  인덱스에 변화가 없으면 할 필요가 X
	// {
	//	 retturn;
	// }
	//ToolbarInventoryComponent->SetCurrentEquippedSlotIndex(AdjustedIndex);
	// 동기화된 장착 요청
	Multicast_EquipItemFromQuickSlot(AdjustedIndex);
}

void ABaseCharacter::Multicast_EquipItemFromQuickSlot_Implementation(int32 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("refresh animation on Client"));
	EquipItemFromCurrentQuickSlot(Index);
}

void ABaseCharacter::EquipItemFromCurrentQuickSlot(int32 QuickSlotIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Change Equip Item"));

	//카메라 초기화(총 줌 쓰고 있다가 바뀔 가능성 대비)
	SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));

	//Mesh의 애니메이션 인스턴스 가져오기
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		//만약 재생중인 몽타주가 있으면(예시: 장전모션) 강제로 해제
		AnimInstance->Montage_Stop(0.25f); // 페이드 아웃 시간: 0.25초 //AnimInstance->Montage_Stop(0.25f, ReloadMontage);이런 것도 가능
	}

	if (!ToolbarInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::SwitchToSlot] 툴바 컴포넌트가 없습니다."));
		RefreshOverlayObject(0);
		return;
	}

	if (QuickSlotIndex < 0 || QuickSlotIndex >= ToolbarInventoryComponent->GetMaxSlots())
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::SwitchToSlot] 유효하지 않은 슬롯 인덱스: %d"), QuickSlotIndex);
		return;
	}

	// 클라이언트에서 호출된 경우 서버에 요청
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_EquipItemFromCurrentQuickSlot(QuickSlotIndex); // 클라이언트에서 호출 무시되는 중...
		return;
	}

	//  똑같으면 무시하라는 거 같은데...
	int32 CurrentSlot = ToolbarInventoryComponent->GetCurrentEquippedSlotIndex();
	if (CurrentSlot == QuickSlotIndex)
	{
		return;
	}

	//TODO: 툴바 index로 읽어서 있는 아이템이 뭔지 받아오고, 그걸 토대로 장착 및 애니메이션 변경하지
	ToolbarInventoryComponent->EquipItemAtSlot(QuickSlotIndex);

	RefreshOverlayObject(QuickSlotIndex);
}


void ABaseCharacter::Server_EquipItemFromCurrentQuickSlot_Implementation(int32 QuickSlotIndex)
{
	EquipItemFromCurrentQuickSlot(QuickSlotIndex);
}

int32 ABaseCharacter::GetCurrentQuickSlotIndex()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return 0;
	}
	if (!IsValid(ToolbarInventoryComponent))
	{
		return 0;
	}
	return ToolbarInventoryComponent->GetCurrentEquippedSlotIndex();
}

void ABaseCharacter::StopCurrentPlayingMontage()
{
	UE_LOG(LogTemp, Warning, TEXT("애님 몽타주 강종"));
	//Mesh의 애니메이션 인스턴스 가져오기
	UAnimInstance* FPSAnimInstance = GetMesh()->GetAnimInstance();
	if (FPSAnimInstance && FPSAnimInstance->IsAnyMontagePlaying())
	{
		//만약 재생중인 몽타주가 있으면(예시: 장전모션) 강제로 해제
		FPSAnimInstance->Montage_Stop(0.25f); // 페이드 아웃 시간: 0.25초 //AnimInstance->Montage_Stop(0.25f, ReloadMontage);이런 것도 가능
	}
}

void ABaseCharacter::HandleInventoryUpdated()
{
	UE_LOG(LogTemp, Log, TEXT("Inventory updated!"));
	//ToolbarInventoryComponent->GetCurrentEquippedSlotIndex();

	RefreshOverlayObject(0);
	// 여기서 UI 갱신 등 원하는 작업 수행
}

void ABaseCharacter::UnequipCurrentItem()
{
	HeldItem = nullptr;
	//TODO: 손에서 제거, 메시 해제, 이펙트 제거 등 처리
	UE_LOG(LogTemp, Log, TEXT("Unequipped current item"));

	if (!IsEquipped())
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] 현재 장비 상태가 아닙니다."));
		return;
	}

	if (!ToolbarInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] 툴바 컴포넌트가 없습니다."));
		return;
	}

	// 클라이언트에서 호출된 경우 서버에 요청
	if (GetLocalRole() < ROLE_Authority)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] 클라이언트에서 서버로 요청"));
		Server_UnequipCurrentItem();
		return;
	}

	// 서버에서 실제 처리
	LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] 서버에서 장비 해제 처리"));

	// 현재 장착된 아이템 정보 가져오기 (로그용)
	AItemBase* CurrentEquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	FString ItemName = CurrentEquippedItem ? CurrentEquippedItem->ItemRowName.ToString() : TEXT("Unknown");

	// 툴바 컴포넌트에서 실제 해제 처리
	ToolbarInventoryComponent->UnequipCurrentItem();

	// 장비 해제 후 상태 확인
	if (!IsEquipped())
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] %s 아이템 해제 성공"), *ItemName);
	}
	else
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] 아이템 해제 실패 - 여전히 장비 상태임"));
		return;
	}
}


float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Log, TEXT("Character Take Damage"));
	if (!HasAuthority())
	{
		return 0;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return 0;
	}
	if (MyPlayerState->bInfiniteHP == true)
	{
		return 0;
	}
	float FinalDamage = CalculateTakeDamage(DamageAmount);
	float CurrentHP = MyPlayerState->GetHP();
	float MaxHP = MyPlayerState->MaxHP;
	float CalCulatedHP = FMath::Clamp(CurrentHP - FinalDamage, 0.0f, MaxHP);
	MyPlayerState->SetHP(CalCulatedHP);
	UE_LOG(LogTemp, Warning, TEXT("Current HP : %f"), CalCulatedHP);
	if (CalCulatedHP <= 0.f)
	{
		HandlePlayerDeath(); // 사망 처리
	}
	return DamageAmount;
}

void ABaseCharacter::GetFallDamage(float Velocity)
{
	Super::GetFallDamage(Velocity);
	if (!HasAuthority())
	{
		return;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (MyPlayerState->bInfiniteHP == true)
	{
		return;
	}

	float FinalDamage = CalculateFallDamage(Velocity);
	float CurrentHP = MyPlayerState->GetHP();
	float MaxHP = MyPlayerState->MaxHP;
	float CalCulatedHP = FMath::Clamp(CurrentHP - FinalDamage, 0.0f, MaxHP);
	UE_LOG(LogTemp, Warning, TEXT("Current HP : %f"), CalCulatedHP);
	MyPlayerState->SetHP(CalCulatedHP);
	if (CalCulatedHP <= 0.f)
	{
		HandlePlayerDeath(); // 사망 처리
	}
}

void ABaseCharacter::HandlePlayerDeath()
{
	UE_LOG(LogTemp, Log, TEXT("Character Died"));
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
	if (!IsValid(PC))
	{
		return;
	}	
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	MyPlayerState->CurrentState = EPlayerState::Dead;
	MyPlayerState->SetInGameStatus(EPlayerInGameStatus::Spectating);
	PC->OnPlayerExitActivePlay();
	Multicast_SetPlayerInGameStateOnDie();
	UnequipCurrentItem();
	StartRagdolling();
}

void ABaseCharacter::Multicast_SetPlayerInGameStateOnDie_Implementation()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	MyPlayerState->CurrentState = EPlayerState::Dead;
	MyPlayerState->SetInGameStatus(EPlayerInGameStatus::Spectating);
	SwapHeadMaterialTransparent(false);
}

float ABaseCharacter::CalculateTakeDamage(float DamageAmount)
{
	//TODO: 여기에다가 추가로 뭔가 장비나 방어력이 추가 되면 여기서 계산하고 넘겨도 됨.
	return DamageAmount;
}

float ABaseCharacter::CalculateFallDamage(float Velocity)
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return 0.0f;
	}
	if (-Velocity < MyPlayerState->FallDamageThreshold)
	{
		return 0.0f;
	}
	float AppliedFallDamage = -Velocity - MyPlayerState->FallDamageThreshold;
	float AdditionalCalculatedDamage = AppliedFallDamage / 10;
	//TODO: 여기에다가 추가로 뭔가 장비나 방어력이 추가 되면 여기서 계산하고 넘겨도 됨.
	return AdditionalCalculatedDamage;
}

void ABaseCharacter::EscapeThroughGate()
{
	ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
	if (!IsValid(PC))
	{
		return;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	MyPlayerState->CurrentState = EPlayerState::Escape;
	MyPlayerState->SetInGameStatus(EPlayerInGameStatus::Spectating);	
	PC->OnPlayerExitActivePlay();
	Multicast_SetPlayerInGameStateOnEscapeGate();
}

void ABaseCharacter::Multicast_SetPlayerInGameStateOnEscapeGate_Implementation()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerState Isn`t Valid"));
		return;
	}
	MyPlayerState->CurrentState = EPlayerState::Escape;
	MyPlayerState->InGameState = EPlayerInGameStatus::Spectating; // 관전 상태 돌입
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


EPlayerInGameStatus ABaseCharacter::CheckPlayerCurrentState()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return EPlayerInGameStatus::None;
	}
	return MyPlayerState->InGameState;
}


void ABaseCharacter::Client_SetMovementSetting_Implementation()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}

	TArray<float> CalculatedSpeedArray = CalculateMovementSpeedWithWeigth();
	if (CalculatedSpeedArray.Num() < 5)
	{
		return;
	}


	//스테이트에 바뀐 값 저장
	MyPlayerState->WalkSpeed = CalculatedSpeedArray[0];
	MyPlayerState->RunSpeed = CalculatedSpeedArray[1];
	MyPlayerState->SprintSpeed = CalculatedSpeedArray[2];
	MyPlayerState->CrouchSpeed = CalculatedSpeedArray[3];
	MyPlayerState->JumpZVelocity = CalculatedSpeedArray[4];

	AlsCharacterMovement->SetGaitSettings(CalculatedSpeedArray[0], CalculatedSpeedArray[0], CalculatedSpeedArray[1], CalculatedSpeedArray[1], CalculatedSpeedArray[2], CalculatedSpeedArray[3]);
	AlsCharacterMovement->JumpZVelocity = CalculatedSpeedArray[4];
}

void ABaseCharacter::SetMovementSetting()
{
	if (HasAuthority())
	{
		Client_SetMovementSetting();
	}

	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}

	TArray<float> CalculatedSpeedArray = CalculateMovementSpeedWithWeigth();
	if (CalculatedSpeedArray.Num() < 5)
	{
		return;
	}

	//스테이트에 바뀐 값 저장
	MyPlayerState->WalkSpeed = CalculatedSpeedArray[0];
	MyPlayerState->RunSpeed = CalculatedSpeedArray[1];
	MyPlayerState->SprintSpeed = CalculatedSpeedArray[2];
	MyPlayerState->CrouchSpeed = CalculatedSpeedArray[3];
	MyPlayerState->JumpZVelocity = CalculatedSpeedArray[4];

	AlsCharacterMovement->SetGaitSettings(CalculatedSpeedArray[0], CalculatedSpeedArray[0], CalculatedSpeedArray[1], CalculatedSpeedArray[1], CalculatedSpeedArray[2], CalculatedSpeedArray[3]);
	AlsCharacterMovement->JumpZVelocity = CalculatedSpeedArray[4];

}

TArray<float> ABaseCharacter::CalculateMovementSpeedWithWeigth()
{
	TArray<float> Calculated;
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return Calculated;
	}
	float MyWeight = GetTotalCarryingWeight() * MyPlayerState->WeightSlowdownMultiplier;
	float CalculatedWalkSpeed = MyPlayerState->DefaultWalkSpeed - MyWeight;
	float CalculatedRunSpeed = MyPlayerState->DefaultRunSpeed - MyWeight;
	float CalculatedSprintSpeed = MyPlayerState->DefaultSprintSpeed - MyWeight;
	float CalculatedCrouchSpeed = MyPlayerState->DefaultCrouchSpeed - MyWeight / 2;
	float CalculatedJumpZVelocity = MyPlayerState->DefaultJumpZVelocity - MyWeight / 5;

	Calculated.Add(CalculatedWalkSpeed);
	Calculated.Add(CalculatedRunSpeed);
	Calculated.Add(CalculatedSprintSpeed);
	Calculated.Add(CalculatedCrouchSpeed);
	Calculated.Add(CalculatedJumpZVelocity);
	return Calculated;
}

void ABaseCharacter::ResetMovementSetting()
{
	AlsCharacterMovement->ResetGaitSettings();
}

void ABaseCharacter::RefreshOverlayObject(int index)
{
	UE_LOG(LogTemp, Warning, TEXT("Refresh Overlay Objects"));
	AItemBase* CurrentItem = GetToolbarInventoryComponent()->GetCurrentEquippedItem();
	//static FGameplayTag CurrentItemTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Player.Equipped"));  // 참고용
	if (!IsValid(CurrentItem))
	{
		//아이템이 없으면, 기본 애니메이션 지정 및 소켓에 달려있는 거 삭제
		SetDesiredGait(AlsOverlayModeTags::Default);
		SetOverlayMode(AlsOverlayModeTags::Default);
		RefreshOverlayLinkedAnimationLayer(3);
		//AttachOverlayObject(NULL, NULL, NULL, "Torch", true);

		UE_LOG(LogTemp, Warning, TEXT("Character Equipped None"));

		return;
	}
	//아이템이 있을 때	
	FGameplayTag ItemTag = CurrentItem->ItemData.ItemType;
	UE_LOG(LogTemp, Warning, TEXT("ItemTag: %s"), *ItemTag.ToString());

	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))  // 또는 HasTag 등 비교 방식에 따라
	{
		SetDesiredGait(AlsOverlayModeTags::Rifle);
		SetOverlayMode(AlsOverlayModeTags::Rifle);
		RefreshOverlayLinkedAnimationLayer(0);
		SetDesiredAiming(true);
		//AttachOverlayObject(NULL, SKM_Rifle, NULL, "Rifle", false);
		return;
	}

	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.FlashLight")))
	{
		UStaticMesh* FlashLightMesh = CurrentItem->ItemData.StaticMesh;
		SetDesiredGait(AlsOverlayModeTags::Torch);
		SetOverlayMode(AlsOverlayModeTags::Torch);
		RefreshOverlayLinkedAnimationLayer(2);
		//AttachOverlayObject(FlashLightMesh, NULL, NULL, "Torch", true);
		return;
	}

	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Spawnable.Drone")))
	{
		SetDesiredGait(AlsOverlayModeTags::PistolOneHanded);
		SetOverlayMode(AlsOverlayModeTags::PistolOneHanded);
		RefreshOverlayLinkedAnimationLayer(1);
		//AttachOverlayObject(FlashLightMesh, NULL, NULL, "Torch", true);
		return;
	}
	//아이템은 있는데 매치가 아무것도 안되면
	UE_LOG(LogTemp, Warning, TEXT("Equipped Item is Valid but doesn`t match any tag"));
	SetDesiredGait(AlsOverlayModeTags::Default);
	SetOverlayMode(AlsOverlayModeTags::Default);
	RefreshOverlayLinkedAnimationLayer(3);
	//AttachOverlayObject(SM_Torch, NULL, NULL, "Torch", true);
	UE_LOG(LogTemp, Warning, TEXT("Character Equipped Unknown Item"));
	return;

	/*
	예시 코드.. 참고할 것!
	if (index == 0)
	{
		SetDesiredGait(AlsOverlayModeTags::Rifle);
		SetOverlayMode(AlsOverlayModeTags::Rifle);
		RefreshOverlayLinkedAnimationLayer(index);
		AttachOverlayObject(NULL, SKM_Rifle, NULL, "Rifle", false);
	}*/
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

	//EquippedItemComponent->SetMesh()
	//EquippedItemComponent->AttachToComponent(GetMesh(), AttachRules, ResultSocketName);
	//EquippedItemComponent->SetAnimInstanceClass(NewAnimationClass);

	//RemoteOnlyEquippedItemComponent->SetMesh()
	//RemoteOnlyEquippedItemComponent->AttachToComponent(RemoteOnlySkeletalMesh, AttachRules, ResultSocketName);
	//RemoteOnlyOverlayStaticMesh->SetAnimInstanceClass(NewAnimationClass);
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
	}
	else
	{
		GetMesh()->LinkAnimClassLayers(DefaultAnimationClass);
	}
}


UToolbarInventoryComponent* ABaseCharacter::GetToolbarInventoryComponent() const
{
	return ToolbarInventoryComponent;
}

UBackpackInventoryComponent* ABaseCharacter::GetBackpackInventoryComponent() const
{
	return BackpackInventoryComponent;
}

bool ABaseCharacter::IsEquipped() const
{
	static FGameplayTag EquippedTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Player.Equipped"));
	return EquippedTags.HasTag(EquippedTag);
}

void ABaseCharacter::SetEquipped(bool bEquip)
{
	static FGameplayTag EquippedTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Player.Equipped"));
	if (bEquip)
	{
		EquippedTags.AddTag(EquippedTag);
	}
	else
	{
		EquippedTags.RemoveTag(EquippedTag);
	}
}

bool ABaseCharacter::TryPickupItem(AItemBase* HitItem)
{
	if (!HitItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABaseCharacter::TryPickupItem] ItemActor가 NULL"));
		return false;
	}
	//서버
	if (HasAuthority())
	{
		return TryPickupItem_Internal(HitItem);
	}
	else
	{
		//클라이언트
		Server_TryPickupItem(HitItem);
		return true;
	}
}

void ABaseCharacter::Server_TryPickupItem_Implementation(AItemBase* HitItem)
{
	if (!HitItem)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::Server_TryPickupItem] ItemActor가 nullptr입니다."));
		return;
	}

	TryPickupItem_Internal(HitItem);
}

bool ABaseCharacter::TryPickupItem_Internal(AItemBase* ItemActor)
{
	if (!HasAuthority())
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::TryPickupItem_Internal] Authority가 없습니다."));
		return false;
	}

	if (!ItemActor)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::TryPickupItem_Internal] ItemActor가 nullptr입니다."));
		return false;
	}

	ULCGameInstanceSubsystem* GameSubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	if (GameSubsystem)
	{
		if (const FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(ItemActor->ItemRowName))
		{
			static const FGameplayTag CollectibleTag = FGameplayTag::RequestGameplayTag(TEXT("ItemType.Collectible"));
			bool bIsCollectible = ItemData->ItemType.MatchesTag(CollectibleTag);
			if (bIsCollectible && BackpackInventoryComponent)
			{
				if (BackpackInventoryComponent->TryAddItem(ItemActor))
				{
					return true;
				}
			}
		}
	}

	//툴바가 있으면
	if (ToolbarInventoryComponent)
	{
		//툴바에 집어넣기
		if (ToolbarInventoryComponent->TryAddItem(ItemActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("툴바에 집어넣는중"));
			return true;
		}
	}


	UE_LOG(LogTemp, Warning, TEXT("[ABaseCharacter::TryPickupItem_Internal] 모든 인벤토리가 가득참: %s"),
		*ItemActor->ItemRowName.ToString());
	return false;
}

void ABaseCharacter::Server_UnequipCurrentItem_Implementation()
{
	UnequipCurrentItem();
}

bool ABaseCharacter::UseEquippedItem(float ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("아이템 사용 : %f"), ActionValue);

	if (bIsReloading)
	{
		return true;
	}
	if (!IsEquipped())
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] 장비 상태가 아닙니다."));
		return false;
	}

	if (!ToolbarInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] 툴바 컴포넌트가 없습니다."));
		return false;
	}

	if (GetLocalRole() < ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버가 아님"));
		Server_UseEquippedItem(ActionValue);
		return true;
	}



	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!EquippedItem)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] 현재 장착된 아이템이 없습니다."));
		return false;
	}

	if (ActionValue >= 0.5f)
	{
		if (EquippedItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Spawnable.Drone")))
		{
			ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
			if (PC)
			{
				PC->SpawnDrone();
				//현재 들고 있는 인벤토리에서 제거하기

				ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].ItemRowName = "Default";
				ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].Quantity = 1;
				ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].Durability = 100;
				ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].bIsEquipped = false;
				ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].bIsValid = true;
				UnequipCurrentItem();
				ToolbarInventoryComponent->EquippedItemComponent->DestroyChildActor();
				RefreshOverlayObject(0);
				return true;
			}
		}

		if (EquippedItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
		{
			if (bIsSprinting)
			{
				return true;
			}
			if (IsDesiredAiming() == false)
			{
				UE_LOG(LogTemp, Warning, TEXT("조준하세요"));
				return true;
			}

		}
		UE_LOG(LogTemp, Warning, TEXT("UseItem"));
		EquippedItem->UseItem();
		bIsUsingItem = true;
		return true;
	}
	else
	{
		bIsUsingItem = false;
		AGunBase* Rifle = Cast<AGunBase>(EquippedItem);
		if (!IsValid(Rifle))
		{
			return false;
		}
		if (Rifle->CurrentFireMode == EFireMode::FullAuto)
		{
			Rifle->StopAutoFire();
		}
		return true;
	}
	
}

void ABaseCharacter::Server_UseEquippedItem_Implementation(float ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("서버 RPC"));
	UseEquippedItem(ActionValue);
}

void ABaseCharacter::ToggleInventory()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			UIManager->ToggleInventory();
		}
	}
}

bool ABaseCharacter::IsInventoryOpen() const
{
	return bInventoryOpen;
}

void ABaseCharacter::DropCurrentItem()
{
	if (!ToolbarInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[DropCurrentItem] ToolbarInventoryComponent is null"));
		return;
	}

	int32 CurrentSlotIndex = ToolbarInventoryComponent->GetCurrentEquippedSlotIndex();
	if (CurrentSlotIndex < 0)
	{
		LOG_Item_WARNING(TEXT("[DropCurrentItem] 현재 장착된 아이템이 없습니다"));
		return;
	}

	// ⭐ DropItemAtSlot 재사용 (Authority 처리 포함)
	DropItemAtSlot(CurrentSlotIndex, 1);

	// 장착 해제 및 UI 새로고침
	RefreshOverlayObject(0);
}

void ABaseCharacter::DropItemAtSlot(int32 SlotIndex, int32 Quantity)
{
	if (!ToolbarInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::DropItemAtSlot] ToolbarInventoryComponent is null"));
		return;
	}

	bool bSuccess = ToolbarInventoryComponent->TryDropItemAtSlot(SlotIndex, Quantity);

	if (bSuccess)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::DropItemAtSlot] 드롭 요청 성공"));
		// UI 새로고침은 OnInventoryUpdated 델리게이트에서 자동 처리
	}
	else
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::DropItemAtSlot] 드롭 요청 실패"));
	}
}

bool ABaseCharacter::EquipBackpack(FName BackpackItemRowName, const TArray<FBaseItemSlotData>& BackpackData, int32 MaxSlots)
{
	if (BackpackItemRowName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABaseCharacter::EquipBackpack] 잘못된 매개변수"));
		return false;
	}

	if (BackpackInventoryComponent)
	{
		BackpackInventoryComponent->MaxSlots = MaxSlots;
		BackpackInventoryComponent->ItemSlots = BackpackData;

		if (ULCGameInstanceSubsystem* GameSubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			BackpackInventoryComponent->ItemDataTable = GameSubsystem->ItemDataTable;
		}

		if (BackpackInventoryComponent->ItemSlots.Num() == 0)
		{
			BackpackInventoryComponent->InitializeSlots();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABaseCharacter::EquipBackpack] BackpackInventoryComponent가 없습니다"));
		return false;
	}

	if (ULCGameInstanceSubsystem* GameSubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (const FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(BackpackItemRowName))
		{
			SetBackpackMesh(ItemData->StaticMesh);
		}
	}

	return true;
}

TArray<FBaseItemSlotData> ABaseCharacter::UnequipBackpack()
{
	TArray<FBaseItemSlotData> BackpackData;

	if (BackpackInventoryComponent)
	{
		BackpackData = BackpackInventoryComponent->ItemSlots;

		BackpackInventoryComponent->ItemSlots.Empty();
		BackpackInventoryComponent->MaxSlots = 0;
	}

	SetBackpackMesh(nullptr);

	return BackpackData;
}

bool ABaseCharacter::HasBackpackEquipped() const
{
	return BackpackInventoryComponent && BackpackInventoryComponent->MaxSlots > 0;
}

void ABaseCharacter::SetBackpackMesh(UStaticMesh* BackpackMesh)
{
	if (!BackpackMeshComponent)
	{
		return;
	}

	if (BackpackMesh)
	{
		BackpackMeshComponent->SetStaticMesh(BackpackMesh);
		BackpackMeshComponent->SetVisibility(true);
		UE_LOG(LogTemp, Warning, TEXT("[SetBackpackMesh] 가방 메시 표시"));
	}
	else
	{
		BackpackMeshComponent->SetStaticMesh(nullptr);
		BackpackMeshComponent->SetVisibility(false);
		UE_LOG(LogTemp, Warning, TEXT("[SetBackpackMesh] 가방 메시 숨김"));
	}
}

void ABaseCharacter::OnInventoryWeightChanged(float WeightDifference)
{
	float NewTotalWeight = 0.0f;

	if (ToolbarInventoryComponent)
	{
		NewTotalWeight += ToolbarInventoryComponent->GetTotalWeight();
	}

	if (BackpackInventoryComponent)
	{
		NewTotalWeight += BackpackInventoryComponent->GetTotalWeight();
	}

	float OldWeight = CurrentTotalWeight;
	CurrentTotalWeight = NewTotalWeight;

	LOG_Item_WARNING(TEXT("[OnInventoryWeightChanged] 총 무게: %.2f -> %.2f"),
		OldWeight, NewTotalWeight);

	//// 블루프린트에서 UI 업데이트나 이동속도 조절 처리
	//OnWeightChanged(OldWeight, NewTotalWeight);
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (IsValid(MyPlayerState))
	{
		SetMovementSetting();
		UE_LOG(LogTemp, Warning, TEXT("플레이어 무브먼트 세팅 초기화 성공"));
	}
}

float ABaseCharacter::GetTotalCarryingWeight() const
{
	return CurrentTotalWeight;
}

void ABaseCharacter::ToggleFireMode()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}

	if (!IsValid(ToolbarInventoryComponent))
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::ToggleFireMode] 툴바 컴포넌트가 없습니다."));
		return;
	}

	// 현재 장착된 아이템 가져오기
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!EquippedItem)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::ToggleFireMode] 현재 장착된 아이템이 없습니다."));
		return;
	}

	// 장착된 아이템이 총기인지 확인
	AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem);
	if (!IsValid(EquipmentItem))
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::ToggleFireMode] 장착된 아이템이 장비 아이템이 아닙니다."));
		return;
	}

	if (EquipmentItem->ItemData.ItemType != FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::ToggleFireMode] 장착된 아이템이 총기가 아닙니다."));
		return;
	}

	// 총기로 캐스팅하여 발사 모드 전환
	AGunBase* Gun = Cast<AGunBase>(EquippedItem);
	if (IsValid(Gun))
	{
		Gun->ToggleFireMode();
		LOG_Item_WARNING(TEXT("[ABaseCharacter::ToggleFireMode] 총기 발사 모드를 전환했습니다."));
	}
	else
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::ToggleFireMode] 총기로 캐스팅에 실패했습니다."));
	}
}

void ABaseCharacter::EnableStencilForAllMeshes(int32 StencilValue)
{
	TArray<UMeshComponent*> MeshComponents;
	GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComp : MeshComponents)
	{
		MeshComp->SetRenderCustomDepth(true);
		MeshComp->SetCustomDepthStencilValue(StencilValue);
	}
}

void ABaseCharacter::Server_InteractWithResourceNode_Implementation(AResourceNode* TargetNode)
{
	if (!TargetNode)
	{
		return;
	}

	AItemBase* EquippedItem = GetToolbarInventoryComponent()->GetCurrentEquippedItem();
	if (!EquippedItem || !EquippedItem->ItemData.ItemType.MatchesTag(TargetNode->RequiredToolTag))
	{
		LOG_Item_WARNING(TEXT("올바른 도구를 장착하지 않았습니다."));
		return;
	}

	TargetNode->HarvestResource(GetController<APlayerController>());
}
