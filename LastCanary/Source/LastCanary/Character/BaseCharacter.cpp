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
#include "Item/ItemBase.h"
#include "Item/ItemSpawnerComponent.h"
#include "Item/ResourceNode.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Item/EquipmentItem/BackpackItem.h"
#include "UI/Manager/LCUIManager.h"
#include "LastCanary.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ArrowComponent.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsAnimationInstance.h"
#include "Character/BaseCharacterAnimNotify.h"
#include "Components/PostProcessComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Item/Drone/BaseDrone.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsLinkedAnimationInstance.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"
#include "Components/CapsuleComponent.h"
#include "Framework/GameState/LCGameState.h"
#include "Components/WidgetComponent.h"
#include "UI/UIObject/PlayerNameWidget.h"

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

	// 이름 3D 위젯
	NameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	NameWidgetComponent->SetupAttachment(GetMesh());
	NameWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); 
	NameWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	NameWidgetComponent->SetDrawSize(FVector2D(200, 50));
	NameWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NameWidgetComponent->SetIsReplicated(false); 
	NameWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	NameWidgetComponent->SetTickWhenOffscreen(true);
	NameWidgetComponent->SetTwoSided(true);
	NameWidgetComponent->SetUsingAbsoluteRotation(false);
	NameWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

	// CustomDepth 설정 (벽 판별에 중요)
	NameWidgetComponent->SetRenderCustomDepth(true);
	NameWidgetComponent->SetCustomDepthStencilValue(1); // 머티리얼에서 사용할 값

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
	RefreshOverlayObject();


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
		float baseBrightness = FMath::Lerp(-10.0f, 10.0f, GetBrightness()); // 0~1 값을 0.5~2.0 범위로 매핑
		CustomPostProcessComponent->Settings.AutoExposureMinBrightness = baseBrightness - 0.1f;
		CustomPostProcessComponent->Settings.AutoExposureMaxBrightness = baseBrightness + 0.1f;

		CustomPostProcessComponent->Settings.bOverride_AutoExposureBias = true;
		CustomPostProcessComponent->Settings.AutoExposureBias = baseBrightness	; // 유저 설정값 반영

		// 블렌드 웨이트 1.0으로 보정 적용 보장
		CustomPostProcessComponent->BlendWeight = 1.0f;
	}
	SetMovementSetting();
	if (ABasePlayerController* PC = Cast<ABasePlayerController>(GetController()))
	{
		if (IsLocallyControlled())
		{
			PC->RequestShowInGameHUD();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("IsLocal: %s / IsServer: %s"),
		IsLocallyControlled() ? TEXT("YES") : TEXT("NO"),
		HasAuthority() ? TEXT("YES") : TEXT("NO"));

	if (NameWidgetComponent && IsValid(NameWidgetComponent->GetWidget()))
	{
		UUserWidget* Widget = NameWidgetComponent->GetWidget();
		if (Widget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget Class: %s"), *Widget->GetClass()->GetName());
		}

		if (UPlayerNameWidget* NameWidget = Cast<UPlayerNameWidget>(NameWidgetComponent->GetWidget()))
		{
			// PlayerState에서 이름 가져오기
			APlayerState* PS = GetPlayerState();
			if (IsValid(PS))
			{
				NameWidget->SetPlayerName(PS->GetPlayerName());
			}
		}

		if (IsLocallyControlled())
		{
			NameWidgetComponent->SetVisibility(false, true);
		}
	}
}

float ABaseCharacter::GetBrightness()
{
	if (!IsValid(GetController()))
	{
		return 0.0f;
	}
	ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
	if (!IsValid(PC))
	{
		return 1.0f;
	}
	return PC->BrightnessSetting;
}

void ABaseCharacter::SetBrightness(float Value)
{
	float baseBrightness = FMath::Lerp(-10.0f, 10.0f, Value); // 0~1 값을 0.5~2.0 범위로 매핑
	CustomPostProcessComponent->Settings.AutoExposureMinBrightness = baseBrightness - 0.1f;
	CustomPostProcessComponent->Settings.AutoExposureMaxBrightness = baseBrightness + 0.1f;
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
			//SetMovementSetting();
			LOG_Char_WARNING(TEXT("플레이어 무브먼트 세팅 초기화 성공"));
		}
		else
		{
			LOG_Char_WARNING(TEXT("플레이어 무브먼트 세팅 초기화 실패"));
		}
	}

	Super::NotifyControllerChanged();
}

void ABaseCharacter::CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo)
{
	Super::CalcCamera(DeltaTime, ViewInfo);
	if (!IsLocallyControlled()) return; 
	UpdateGunWallClipOffset(DeltaTime);
	if (bIsMantling)
	{
		bIsAiming = false;
		bIsTransitioning = false;
		SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
		Controller->SetControlRotation(GetActorRotation());
		SpringArm->bUsePawnControlRotation = true;
		FVector TargetLoc = GetActorLocation();
		FRotator TargetRot = GetActorRotation();
		ViewInfo.Rotation = TargetRot;
		SpringArm->SetWorldRotation(TargetRot);
		if (!bIsFPSCamera)
		{
			SpringArm->TargetArmLength = 200.0f;
		}
		
		return;
	}

	// 전환 중일 때만 부드러운 이동 처리
	if (bIsTransitioning)
	{
		FVector CurrentLocation = SpringArm->GetComponentLocation();
		FVector TargetLocation;

		// 목표 위치 결정
		if (bIsAiming && IsValid(CurrentRifleMesh) && !bIsReloading)
		{
			TargetLocation = CurrentRifleMesh->GetSocketLocation(FName("Scope"));
		}
		else
		{
			TargetLocation = GetMesh()->GetSocketLocation(FName("FirstPersonCamera"));
		}

		// 부드럽게 이동
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, CameraTransitionSpeed);
		SpringArm->SetWorldLocation(NewLocation);

		// 목표 지점에 가까워지면 Attach
		float Distance = FVector::Dist(NewLocation, TargetLocation);
		if (Distance < 2.0f) // 2.0f 단위 이내로 가까워지면
		{
			bIsTransitioning = false;
			SpringArm->bEnableCameraLag = false;
			SpringArm->bEnableCameraRotationLag = false;
			if (bIsAiming && IsValid(CurrentRifleMesh) && !bIsReloading)
			{
				// Scope에 붙이기
				AttachCameraToRifle();
				SpringArm->bUsePawnControlRotation = false;
			}
			else
			{
				// FirstPersonCamera에 붙이기
				AttachCameraToCharacter();
				SpringArm->TargetArmLength = bIsFPSCamera ? 0.0f : 200.0f;
				SpringArm->bUsePawnControlRotation = true;
			}
		}
	}
	ViewInfo.Rotation.Roll = 0.0f;
	
}

void ABaseCharacter::ResetCameraLocationToDefault()
{
	AttachCameraToCharacter();
	SpringArm->bUsePawnControlRotation = true;
	bIsAiming = false;
	bIsTransitioning = false;
}

void ABaseCharacter::AttachCameraToRifle()
{
	if (IsValid(CurrentRifleMesh))
	{
		if (IsLocallyControlled())
		{
			SpringArm->AttachToComponent(CurrentRifleMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Scope"));
		}
	}
}

void ABaseCharacter::AttachCameraToCharacter()
{
	if (IsValid(GetMesh()))
	{
		if (IsLocallyControlled())
		{
			SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
		}
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
		return;
	}
	if (bIsSprinting || bIsReloading || bIsClose || bIsMantling)
	{
		StopAiming();
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
				CurrentRifleMesh = RifleMesh;
				if (!RifleMesh)
				{
					return;
				}

				if (ActionValue.Get<float>() > 0.5f && bIsCloseToWall == false)
				{
					StartAiming();
					return;
				}
				else
				{
					SpringArm->bUsePawnControlRotation = true;
					StopAiming();
					return;
				}
			}
		}
	}

	SetDesiredAiming(ActionValue.Get<bool>());
}

void ABaseCharacter::StartAiming()
{
	if (!bIsAiming)
	{
		bIsAiming = true;
		bIsTransitioning = true;

		// 스프링암을 RootComponent에 붙여서 자유롭게 움직일 수 있게 함
		SpringArm->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

		CancelInteraction();
	}
}

void ABaseCharacter::StopAiming()
{
	if (bIsAiming)
	{
		bIsAiming = false;
		bIsTransitioning = true;

		// 스프링암을 RootComponent에 붙여서 자유롭게 움직일 수 있게 함
		SpringArm->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	}
}

void ABaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (NameWidgetComponent == nullptr)
	{
		return;
	}

	// 누적 시간 계산
	static float TimeAccumulator = 0.f;
	TimeAccumulator += DeltaSeconds;

	// 0.1초마다 갱신
	if (TimeAccumulator > 0.1f)
	{
		TimeAccumulator = 0.f;

		// 카메라 참조
		APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		if (!CamManager) return;

		FVector CameraLocation = CamManager->GetCameraLocation();
		FVector WidgetLocation = NameWidgetComponent->GetComponentLocation();

		// 거리 측정
		const float Distance = FVector::Dist(CameraLocation, WidgetLocation);
		const float MaxVisibleDistance = 2500.f; // 25m 안일 때만 회전 처리

		if (Distance < MaxVisibleDistance)
		{
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(WidgetLocation, CameraLocation);
			NameWidgetComponent->SetWorldRotation(LookAtRotation);
		}
	}
}// 전환이 완료되었는지 확인하는 유틸리티 함수 (선택사항)



void ABaseCharacter::NotifyNoiseToAI(FVector Velocity)
{
	Super::NotifyNoiseToAI(Velocity);
	float XSpeed = Velocity.X;
	float YSpeed = Velocity.Y;

	//플레이어의 앞뒤좌우 움직임의 속도를 가져오는 부분
	FVector2D XYSpeed(XSpeed, YSpeed);
	float SpeedXY = XYSpeed.Size();
	float PowerOnFoot = SpeedXY + Velocity.Z;
	MakeNoiseSoundToAI(PowerOnFoot);
	MakeNoiseSoundToBoss(PowerOnFoot);
}

void ABaseCharacter::NotifyNoiseToAI(float LandVelocity)
{
	Super::NotifyNoiseToAI(LandVelocity);
	float CurrentPlayerSpeed = GetPlayerMovementSpeed();
	float PowerOnFoot = CurrentPlayerSpeed + LandVelocity;

	MakeNoiseSoundToAI(PowerOnFoot);
	MakeNoiseSoundToBoss(PowerOnFoot);
}

void ABaseCharacter::MakeNoiseSoundToAI(float Force)
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (Force < MyPlayerState->WalkSpeed)
	{
		return;
	}
	
	FVector SoundLocation = GetActorLocation();
	
	float SoundLoudness = Force / SoundLoudnessDivider;
	
	AActor* SoundCauser = this;
	
	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		SoundLocation,              // FVector: 소리가 발생한 위치
		SoundLoudness*1.5,                   // float: 소리의 크기 (기본값은 1.0f, 감지 거리 등에 영향을 줌)
		SoundCauser,                 // AActor*: 소리의 주체 (보통 this)
		MaxSoundRange,                   // float: 소리를 들을 수 있는 최대 거리
		AISoundCheckTag                         // FName: 태그로 필터링 가능 (선택사항)
	);

}

void ABaseCharacter::MakeNoiseSoundToBoss(float Force)
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (Force < MyPlayerState->WalkSpeed)
	{
		return;
	}

	FVector SoundLocation = GetActorLocation();

	float SoundLoudness = Force / SoundLoudnessDivider;

	AActor* SoundCauser = this;

	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		SoundLocation,              // FVector: 소리가 발생한 위치
		SoundLoudness*2,                   // float: 소리의 크기 (기본값은 1.0f, 감지 거리 등에 영향을 줌)
		SoundCauser,                 // AActor*: 소리의 주체 (보통 this)
		MaxSoundRange,                   // float: 소리를 들을 수 있는 최대 거리
		FName("Boss")                         // FName: 태그로 필터링 가능 (선택사항)
	);
}

void ABaseCharacter::Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity)
{
	if (bIsPlayingInteractionMontage)
	{
		return;
	}
	if (LocomotionAction == AlsLocomotionActionTags::Mantling)
	{
		return;
	}
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	if (!Controller) 
	{ 
		return;
	}
	ReduceRecoil(0.3f);
	AddControllerYawInput(Value.X * Sensivity);
	AddControllerPitchInput(Value.Y * Sensivity);
}


void ABaseCharacter::StartTrackingDrone()
{
	GetWorld()->GetTimerManager().SetTimer(DroneTrackingTimerHandle, this, &ABaseCharacter::UpdateRotationToDrone, 0.02f, true);
}

void ABaseCharacter::StopTrackingDrone()
{
	GetWorld()->GetTimerManager().ClearTimer(DroneTrackingTimerHandle);
}

void ABaseCharacter::UpdateRotationToDrone()
{
	if (!IsValid(ControlledDrone))
	{
		return;
	}
	if (!IsValid(Controller))
	{
		return;
	}
	FVector ToDrone = ControlledDrone->GetActorLocation() - GetActorLocation();
	ToDrone.Z = 0; // Pitch는 무시해서 Yaw 회전만

	if (!ToDrone.IsNearlyZero())
	{
		FRotator LookAtRotation = ToDrone.Rotation();

		// 부드러운 회전 (선택)
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), 5.0f);

		SetActorRotation(NewRotation);
		SpringArm->SetWorldRotation(NewRotation);
		Controller->SetControlRotation(NewRotation); // 컨트롤러 회전도 고정
	}
}

// 방법 1: 즉시 반동 + 점진적 복구 (일반적인 방식)
void ABaseCharacter::ApplyRecoil(float Vertical, float Horizontal)
{
	if (!Controller) return;

	// 연사 배수 계산
	float ShotMultiplier = FMath::Min(1.0f + (CurrentShotCount * 0.15f), 2.5f);

	// 목표 반동량 설정
	TargetRecoil.X += Vertical * ShotMultiplier;
	TargetRecoil.Y += FMath::RandRange(-Horizontal, Horizontal) * ShotMultiplier;

	CurrentShotCount++;

	// 반동 적용 타이머 시작
	if (!GetWorld()->GetTimerManager().IsTimerActive(RecoilRecoveryTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(RecoilRecoveryTimer, this,
			&ABaseCharacter::UpdateRecoil, 0.016f, true);
	}

	// 연사 리셋 타이머
	GetWorld()->GetTimerManager().ClearTimer(ShotResetTimer);
	GetWorld()->GetTimerManager().SetTimer(ShotResetTimer, this,
		&ABaseCharacter::ResetShotCounter, 0.5f, false);
}

void ABaseCharacter::RecoverFromRecoil()
{
	if (!Controller || AccumulatedRecoil.IsNearlyZero(0.01f))
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float RecoveryAmount = RecoilRecoverySpeed * DeltaTime;

	// 점진적으로 원래 위치로 복구
	FVector2D RecoveryVector = AccumulatedRecoil;
	RecoveryVector.Normalize();
	RecoveryVector *= RecoveryAmount;

	if (RecoveryVector.Size() >= AccumulatedRecoil.Size())
	{
		// 완전 복구
		AddControllerPitchInput(AccumulatedRecoil.X);
		AddControllerYawInput(-AccumulatedRecoil.Y);
		AccumulatedRecoil = FVector2D::ZeroVector;
	}
	else
	{
		// 부분 복구
		AddControllerPitchInput(RecoveryVector.X);
		AddControllerYawInput(-RecoveryVector.Y);
		AccumulatedRecoil -= RecoveryVector;
	}
}

// 스무스하게 반동주기
void ABaseCharacter::ApplySmoothRecoil(float Vertical, float Horizontal)
{
	if (!Controller) return;

	// 목표 반동량 설정
	float ShotMultiplier = FMath::Min(1.0f + (CurrentShotCount * 0.15f), 2.5f);
	TargetRecoil.X += Vertical * ShotMultiplier;
	TargetRecoil.Y += FMath::RandRange(-Horizontal, Horizontal) * ShotMultiplier;

	CurrentShotCount++;

	// 스무딩된 반동 적용 시작
	if (!GetWorld()->GetTimerManager().IsTimerActive(RecoilRecoveryTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(RecoilRecoveryTimer, this,
			&ABaseCharacter::ApplySmoothRecoilStep, 0.016f, true);
	}

	// 연사 리셋 타이머
	GetWorld()->GetTimerManager().ClearTimer(ShotResetTimer);
	GetWorld()->GetTimerManager().SetTimer(ShotResetTimer, this,
		&ABaseCharacter::ResetShotCounter, 0.3f, false);
}

void ABaseCharacter::ApplySmoothRecoilStep()
{
	if (!Controller)
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 목표 반동량으로 보간
	FVector2D RecoilDelta = (TargetRecoil - AccumulatedRecoil) * (5.0f * DeltaTime);

	if (!RecoilDelta.IsNearlyZero(0.01f))
	{
		AddControllerPitchInput(RecoilDelta.X);
		AddControllerYawInput(RecoilDelta.Y);
		AccumulatedRecoil += RecoilDelta;
	}

	// 자동 복구 (사격이 멈춘 후)
	if (GetWorld()->GetTimerManager().GetTimerRemaining(ShotResetTimer) <= 0.0f)
	{
		FVector2D RecoveryDelta = AccumulatedRecoil * (RecoilRecoverySpeed * DeltaTime);

		if (RecoveryDelta.Size() >= AccumulatedRecoil.Size())
		{
			// 완전 복구
			AddControllerPitchInput(-AccumulatedRecoil.X);
			AddControllerYawInput(-AccumulatedRecoil.Y);
			AccumulatedRecoil = FVector2D::ZeroVector;
			TargetRecoil = FVector2D::ZeroVector;
			GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
		}
		else
		{
			AddControllerPitchInput(-RecoveryDelta.X);
			AddControllerYawInput(-RecoveryDelta.Y);
			AccumulatedRecoil -= RecoveryDelta;
			TargetRecoil -= RecoveryDelta;
		}
	}
}

void ABaseCharacter::UpdateRecoil()
{
	if (!Controller)
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 1. 목표 반동량으로 이동 (반동 적용 단계)
	FVector2D RecoilDelta = (TargetRecoil - AccumulatedRecoil) * (5.0f * DeltaTime);

	if (!RecoilDelta.IsNearlyZero(0.01f))
	{
		AddControllerPitchInput(-RecoilDelta.X);
		AddControllerYawInput(RecoilDelta.Y);
		AccumulatedRecoil += RecoilDelta;
	}

	// 2. 사격이 멈춘 후 자동 복구
	if (GetWorld()->GetTimerManager().GetTimerRemaining(ShotResetTimer) <= 0.0f)
	{
		if (AccumulatedRecoil.IsNearlyZero(0.01f))
		{
			// 완전히 복구됨 - 타이머 정리
			AccumulatedRecoil = FVector2D::ZeroVector;
			TargetRecoil = FVector2D::ZeroVector;
			GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
			return;
		}

		FVector2D RecoveryDelta = AccumulatedRecoil * (RecoilRecoverySpeed * DeltaTime);

		if (RecoveryDelta.Size() >= AccumulatedRecoil.Size())
		{
			// 완전 복구
			AddControllerPitchInput(AccumulatedRecoil.X);
			AddControllerYawInput(-AccumulatedRecoil.Y);
			AccumulatedRecoil = FVector2D::ZeroVector;
			TargetRecoil = FVector2D::ZeroVector;
			GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
		}
		else
		{
			// 점진적 복구
			AddControllerPitchInput(RecoveryDelta.X);
			AddControllerYawInput(-RecoveryDelta.Y);
			AccumulatedRecoil -= RecoveryDelta;
			TargetRecoil -= RecoveryDelta;
		}
	}
}

void ABaseCharacter::ResetShotCounter()
{
	CurrentShotCount = 0;
}

// ===== 이 부분이 핵심! 직접 반동 초기화 함수들 =====

// 반동 완전 리셋
void ABaseCharacter::ResetRecoil()
{
	AccumulatedRecoil = FVector2D::ZeroVector;
	TargetRecoil = FVector2D::ZeroVector;
	GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
}

// 반동 부분 리셋 (특정 축만)
void ABaseCharacter::ResetRecoilPitch()
{
	AccumulatedRecoil.X = 0.0f;
	TargetRecoil.X = 0.0f;
}

void ABaseCharacter::ResetRecoilYaw()
{
	AccumulatedRecoil.Y = 0.0f;
	TargetRecoil.Y = 0.0f;
}

// 반동 감소 (완전 리셋이 아닌 부분 감소)
void ABaseCharacter::ReduceRecoil(float ReductionFactor)
{
	AccumulatedRecoil *= (1.0f - FMath::Clamp(ReductionFactor, 0.0f, 1.0f));
	TargetRecoil *= (1.0f - FMath::Clamp(ReductionFactor, 0.0f, 1.0f));

	// 거의 0에 가까우면 완전 리셋
	if (AccumulatedRecoil.Size() < 0.1f)
	{
		ResetRecoil();
	}
}

// 현재 반동 상태 확인용
FVector2D ABaseCharacter::GetCurrentRecoil() const
{
	return AccumulatedRecoil;
}

bool ABaseCharacter::HasActiveRecoil() const
{
	return !AccumulatedRecoil.IsNearlyZero(0.01f);
}

void ABaseCharacter::Handle_Look(const FInputActionValue& ActionValue)
{

}

void ABaseCharacter::Handle_VoiceChatting(const FInputActionValue& ActionValue)
{
	const float Value = ActionValue.Get<float>();

	if (Value > 0.5f)
	{
		StartVoiceChat();
	}
	else
	{
		CancelVoiceChat();
	}
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

	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	if (CheckHardLandState())
	{
		bIsSprinting = false;
		FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
		SetDesiredGait(AlsGaitTags::Running);
		StopStaminaDrain();
		StartStaminaRecoverAfterDelay();
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
			FootSoundModifier = MyPlayerState->CrouchingFootSoundModifier;
			SetDesiredStance(AlsStanceTags::Crouching);
		}
		else
		{
			FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
			SetDesiredStance(AlsStanceTags::Standing);
		}
	}
	else if (MyPlayerState->CrouchInputMode == EInputMode::Toggle)
	{
		if (Value > 0.5f)
		{
			if (GetDesiredStance() == AlsStanceTags::Standing)
			{
				FootSoundModifier = MyPlayerState->CrouchingFootSoundModifier;
				SetDesiredStance(AlsStanceTags::Crouching);
			}
			else if (GetDesiredStance() == AlsStanceTags::Crouching)
			{
				FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
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

float ABaseCharacter::GetPlayerMovementSpeed() const
{
	float XSpeed = GetLocomotionStateMovementSpeed().X;
	float YSpeed = GetLocomotionStateMovementSpeed().Y;

	//플레이어의 앞뒤좌우 움직임의 속도를 가져오는 부분
	FVector2D XYSpeed(XSpeed, YSpeed);
	float SpeedXY = XYSpeed.Size();
	return SpeedXY;
}

void ABaseCharacter::Handle_Reload()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (bIsReloading)
	{
		return;
	}
	if (bIsUsingItem)
	{
		return;
	}
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!IsValid(EquippedItem))
	{
		return;
	}
	AGunBase* Gun = Cast<AGunBase>(EquippedItem);
	if (!IsValid(Gun))
	{
		return;
	}
	RequestReload(Gun);
}

void ABaseCharacter::RequestReload(AGunBase* Gun)
{
	if (!IsValid(Gun))
	{
		return;
	}
	Gun->CheckReloadCondition();
}

void ABaseCharacter::StartReload()
{
	CancelInteraction();
	bIsReloading = true;
	Server_PlayReload();
}


void ABaseCharacter::Server_PlayReload_Implementation()
{
	Multicast_PlayReload();
}

void ABaseCharacter::Multicast_PlayReload_Implementation()
{
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!IsValid(EquippedItem))
	{
		return;
	}
	AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem);
	if (!IsValid(EquipmentItem))
	{
		return;
	}
	AGunBase* Gun = Cast<AGunBase>(EquippedItem);
	if (!IsValid(Gun))
	{
		return;
	}
	UAnimMontage* MontageToPlay = ReloadMontage;
	if (!(MontageToPlay && Gun && GetMesh()))
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}
	AnimInstance->Montage_Play(MontageToPlay);
	Gun->Multicast_PlayReloadAnimation_Implementation();
	Gun->Multicast_PlayReloadSound_Implementation();
}

void ABaseCharacter::GunReloadAnimationNotified()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!IsValid(PC))
	{
		return;
	}
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!IsValid(EquippedItem))
	{
		return;
	}
	AGunBase* Gun = Cast<AGunBase>(EquippedItem);
	if (!IsValid(Gun))
	{
		return;
	}
	Gun->Reload();
	bIsReloading = false;
}

void ABaseCharacter::StopReload()
{
	Server_StopReload();
}

void ABaseCharacter::Server_StopReload_Implementation()
{
	Multicast_StopReload();
}

void ABaseCharacter::Multicast_StopReload_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		bIsReloading = false;
		AnimInstance->Montage_Stop(0.2f, ReloadMontage); // 부드럽게 블렌드 아웃
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
		SwapHeadMaterialTransparent(true);
		SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
		SpringArm->TargetArmLength = 0.0f;
	}
	else
	{
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
		LOG_Char_WARNING(TEXT("Handle_Interact: No focused actor."));
		return;
	}

	LOG_Char_WARNING(TEXT("Interacted with: %s"), *CurrentFocusedActor->GetName());

	if (bIsPlayingInteractionMontage)
	{
		return;
	}

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
			//CancelInteraction();
			//IInteractableInterface::Execute_Interact(CurrentFocusedActor, PC);
			LOG_Char_WARNING(TEXT("Handle_Interact: Called Interact on %s"), *actor->GetName());
			LOG_Char_WARNING(TEXT("Equipped item on slot"));
			InteractAfterPlayMontage(actor);
		}
		else
		{
			LOG_Char_WARNING(TEXT("Handle_Interact: Controller is nullptr"));
		}
	}
	else
	{
		LOG_Char_WARNING(TEXT("Handle_Interact: %s does not implement IInteractableInterface"), *CurrentFocusedActor->GetName());
	}
	LOG_Char_WARNING(TEXT("Interact Ended"));
}

void ABaseCharacter::InteractAfterPlayMontage(AActor* TargetActor)
{
	UAnimMontage* MontageToPlay = nullptr;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	InteractTargetActor = TargetActor;
	if (!IsValid(AnimInstance))
	{
		return;
	}
	if (InteractTargetActor->IsA<AItemBase>())
	{
		AItemBase* Item = Cast<AItemBase>(InteractTargetActor);

		if (!IsValid(Item))
		{
			return;
		}
		MontageToPlay = InteractMontageOnUnderObject;
	}
	else
	{
		if (InteractTargetActor->Tags.Contains("Roll"))
		{
			LOG_Char_WARNING(TEXT("태그는 Roll"));
			MontageToPlay = OpeningValveMontage;
		}
		else if (InteractTargetActor->Tags.Contains("Kick"))
		{
			LOG_Char_WARNING(TEXT("태그는 Kick"));
			MontageToPlay = KickMontage;
		}
		else if (InteractTargetActor->Tags.Contains("Press"))
		{
			LOG_Char_WARNING(TEXT("태그는 Press"));
			MontageToPlay = PressButtonMontage;
		}
		else if (InteractTargetActor->Tags.Contains("Test"))
		{
			MontageToPlay = InteractMontageOnUnderObject;
			//당장 태그 없는 거 빠르게 테스트 하기 위해서 넣어놨습니다.
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (!IsValid(PC))
			{
				return;
			}
			if (!IsValid(InteractTargetActor))
			{
				return;
			}
			LOG_Char_WARNING(TEXT("excute interact For Test"));
			IInteractableInterface::Execute_Interact(InteractTargetActor, PC);
			return;
		}
		else
		{
			return;
		}
	}
	
	if (!IsValid(MontageToPlay))
	{
		return;
	}
	CurrentInteractMontage = MontageToPlay;
	bIsPlayingInteractionMontage = true;
	Server_PlayMontage(MontageToPlay, EAnimationType::Interaction);
}

void ABaseCharacter::CancelInteraction()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}
	if (!IsValid(CurrentInteractMontage))
	{
		return;
	}
	bIsPlayingInteractionMontage = false;
	AnimInstance->Montage_Stop(0.2f, CurrentInteractMontage); // 부드럽게 블렌드 아웃
	Server_CancelInteraction();
}

void ABaseCharacter::Server_CancelInteraction_Implementation()
{
	Multicast_CancelInteraction();
}

void ABaseCharacter::Multicast_CancelInteraction_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}
	if (!IsValid(CurrentInteractMontage))
	{
		return;
	}
	bIsPlayingInteractionMontage = false;
	AnimInstance->Montage_Stop(0.2f, CurrentInteractMontage); // 부드럽게 블렌드 아웃
}


void ABaseCharacter::OnInteractAnimationNotified()
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
	bIsPlayingInteractionMontage = false;
	IInteractableInterface::Execute_Interact(InteractTargetActor, PC);
}

void ABaseCharacter::Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationType Animtype)
{
	Multicast_PlayMontage(MontageToPlay, Animtype);
}

void ABaseCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationType Animtype)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}
	AnimInstance->Montage_Play(MontageToPlay);
	CurrentInteractMontage = MontageToPlay;
	
	switch (Animtype)
	{
	case EAnimationType::Interaction:
	{
		bIsPlayingInteractionMontage = true;
		break;
	}
	case EAnimationType::UseItem:
	{
		bIsPlayingUseItemMontage = true;
		break;
	}
	default:
	{
		break;
	}
	}
}

void ABaseCharacter::UseItemAfterPlayMontage(AItemBase* EquippedItem)
{
	UAnimMontage* MontageToPlay;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}
	CurrentUsingItem = EquippedItem;
	if (CurrentUsingItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Consumable")))
	{
		MontageToPlay = UsingBandageMontage;
	}
	else if (CurrentUsingItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
	{
		MontageToPlay = PickAxeMontage;
	}
	else
	{
		LOG_Char_WARNING(TEXT("태그가 없음"));
		//태그가 없으면 바로 실행
		EquippedItem->UseItem();
		return;
	}
	if (!IsValid(MontageToPlay))
	{
		LOG_Char_WARNING(TEXT("Anim Montage does not exist."));
		return;
	}
	CurrentUseItemMontage = MontageToPlay;	
	bIsPlayingUseItemMontage = true;
	LOG_Char_WARNING(TEXT("플레이 애니메이션."));
	Server_PlayMontage(MontageToPlay, EAnimationType::UseItem);
}

void ABaseCharacter::UseItemAnimationNotified()
{
	LOG_Char_WARNING(TEXT("애니메이션 재생 후 아이템 사용됨"));
	//재생 후 notify로
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!IsValid(PC))
	{
		return;
	}
	if (!IsValid(CurrentUsingItem))
	{
		return;
	}
	bIsPlayingUseItemMontage = false;
	LOG_Char_WARNING(TEXT("애니메이션 재생 후 아이템 사용됨"));

	CurrentUsingItem->UseItem();
}

void ABaseCharacter::CancelUseItem()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}
	if (!IsValid(CurrentUseItemMontage))
	{
		return;
	}
	AnimInstance->Montage_Stop(0.2f, CurrentUseItemMontage); // 부드럽게 블렌드 아웃
	bIsPlayingUseItemMontage = false;
	Server_CancelUseItem();
}

void ABaseCharacter::Server_CancelUseItem_Implementation()
{
	Multicast_CancelUseItem();
}

void ABaseCharacter::Multicast_CancelUseItem_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance))
	{
		return;
	}
	if (!IsValid(CurrentUseItemMontage))
	{
		return;
	}
	bIsPlayingUseItemMontage = false;
	AnimInstance->Montage_Stop(0.2f, CurrentUseItemMontage); // 부드럽게 블렌드 아웃
}

void ABaseCharacter::TraceInteractableActor()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}
	if (bIsSpawnDrone)
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

#if WITH_EDITOR
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f);
#endif
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

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// 2. 라인 트레이스
	static constexpr float GunWallTraceDistance = 70.0f; // 50에서 100으로 더 여유있게
	static constexpr float TraceStartOffset = 50.0f; // 뒤로 10cm 정도
	FVector TraceStart = MuzzleLoc - MuzzleForward * TraceStartOffset;
	// 끝 지점은 그대로 앞쪽 방향으로 트레이스 거리만큼
	FVector TraceEnd = TraceStart + MuzzleForward * GunWallTraceDistance;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

	// 디버그 라인도 수정된 시작점 기준으로
#if WITH_EDITOR
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f);
#endif
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
	SmoothedWallRatio = FMath::FInterpTo(SmoothedWallRatio, TargetWallRatio, DeltaTime, 10.0f);


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

void ABaseCharacter::SetCurrentQuickSlotIndex(int32 NewIndex)
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}

	StopReload();
	LOG_Char_WARNING(TEXT("Request Server to change QuickSlotindex"));
	Server_SetQuickSlotIndex(NewIndex);
}

void ABaseCharacter::Server_SetQuickSlotIndex_Implementation(int32 NewIndex)
{
	//change QuickSlotindex on Server
	if (!IsValid(ToolbarInventoryComponent))
	{
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
	EquipItem(AdjustedIndex); //서버에서 처리
}

void ABaseCharacter::EquipItem(int32 Index)
{
	if (Index == ToolbarInventoryComponent->GetCurrentEquippedSlotIndex())
	{
		return;
	}
	ToolbarInventoryComponent->EquipItemAtSlot(Index);
	// 동기화된 장착 요청
	Multicast_ResetAnimationAndCamera(Index);
}

void ABaseCharacter::Multicast_ResetAnimationAndCamera_Implementation(int32 Index)
{
	LOG_Char_WARNING(TEXT("Change Equip Item"));
	//카메라 초기화(총 줌 쓰고 있다가 바뀔 가능성 대비)
	ResetCameraLocationToDefault();
	StopCurrentPlayingMontage();
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
	LOG_Char_WARNING(TEXT("애님 몽타주 강종"));
	//Mesh의 애니메이션 인스턴스 가져오기
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		//만약 재생중인 몽타주가 있으면(예시: 장전모션) 강제로 해제
		AnimInstance->Montage_Stop(0.25f); // 페이드 아웃 시간: 0.25초 //AnimInstance->Montage_Stop(0.25f, ReloadMontage);이런 것도 가능
	}
}

void ABaseCharacter::HandleInventoryUpdated()
{
	LOG_Char_WARNING(TEXT("Inventory updated!"));
	RefreshOverlayObject();
}

void ABaseCharacter::UnequipCurrentItem()
{
	LOG_Char_WARNING(TEXT("Unequipped current item"));

	if (!IsEquipped() || !ToolbarInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("현재 장비 상태가 아니거나 툴바가 없습니다."));
		return;
	}
	// 클라이언트에서 호출된 경우 서버에 요청
	if (GetLocalRole() < ROLE_Authority)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] 클라이언트에서 서버로 요청"));
		Server_UnequipCurrentItem();
		return;
	}
#if WITH_EDITOR
	LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] 서버에서 장비 해제 처리")); // 서버에서 실제 처리
#endif
	// 현재 장착된 아이템 정보 가져오기 (로그용)
	AItemBase* CurrentEquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	FString ItemName = CurrentEquippedItem ? CurrentEquippedItem->ItemRowName.ToString() : TEXT("Unknown");

	// 툴바 컴포넌트에서 실제 해제 처리
	ToolbarInventoryComponent->UnequipCurrentItem();

#if WITH_EDITOR
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
#endif
}

void ABaseCharacter::Server_UnequipCurrentItem_Implementation()
{
	UnequipCurrentItem();
}


float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	LOG_Char_WARNING(TEXT("Character Take Damage"));
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
	LOG_Char_WARNING(TEXT("Current HP : %f"), CalCulatedHP);
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
	LOG_Char_WARNING(TEXT("Current HP : %f"), CalCulatedHP);
	MyPlayerState->SetHP(CalCulatedHP);
	if (CalCulatedHP <= 0.f)
	{
		HandlePlayerDeath(); // 사망 처리
	}
}

void ABaseCharacter::HandlePlayerDeath()
{
	LOG_Char_WARNING(TEXT("Character Died"));
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

	//if 캐릭터가 죽으면
	//장착 아이템 제거
	//캐릭터 래그돌
	//보이스 채팅 관련 로직 -> 블루프린트에서 마저 진행
	//캐릭터 상태 변경 -> 멀티캐스트까지
	//관전 상태로 넘어갈 준비
	//게이트 액터 or 게임 스테이트 or 게임 모드에 어떻게든 상태 알리기

	//NotifyPlayerDeathToGameState();
	DropAllItemsOnDeath();
	UnequipCurrentItem();
	StartRagdolling();

	// State 변경
	MyPlayerState->CurrentState = EPlayerState::Dead;
	MyPlayerState->SetInGameStatus(EPlayerInGameStatus::Spectating);
	Client_HandlePlayerVoiceChattingState();
	Multicast_SetPlayerInGameStateOnDie();
	PC->PlayerExitActivePlayOnDeath();
}

void ABaseCharacter::NotifyPlayerDeathToGameState()
{
	AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();
	if (!GameState)
	{
		LOG_Char_WARNING(TEXT("게임스테이트가 유효하지 않음"));
		return;
	}
	ALCGameState* LCGameState = Cast<ALCGameState>(GameState);
	if (!LCGameState)
	{
		LOG_Char_WARNING(TEXT("LCGameState가 유효하지 않음"));
		return;
	}

	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		LOG_Char_WARNING(TEXT("PlayerState가 유효하지 않음"));
		return;
	}

	//여기서부터는 게임스테이트의 코드가 바뀔 것은 알지만 테스트를 위해서 임의로 넣은 코드입니다.
	//LCGameState->MarkPlayerAsEscaped(MyPlayerState);
}

void ABaseCharacter::Client_HandlePlayerVoiceChattingState_Implementation()
{
	//블루프린트에서 마저 구현
	UpdateVoiceChannelBySoectateState();
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
	//if 캐릭터가 게이트에 상호작용을 하면
	//보이스 채팅 관련 로직 -> 블루프린트에서 마저 진행
	//캐릭터 상태 변경 -> 멀티캐스트까지
	//관전 상태로 넘어갈 준비
	Client_HandlePlayerVoiceChattingState();
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
	Multicast_SetPlayerInGameStateOnEscapeGate();
	PC->PlayerExitActivePlayOnEscapeGate();
}

void ABaseCharacter::Multicast_SetPlayerInGameStateOnEscapeGate_Implementation()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		LOG_Char_WARNING(TEXT("PlayerState Isn`t Valid"));
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
	LOG_Char_WARNING(TEXT("SetMovementSetting 클라이언트에서"));
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		LOG_Char_WARNING(TEXT("SetMovementSetting 클라이언트에서 스테이트 없음"));
		return;
	}
	SpeedMultiplier = CalculateMovementSpeedMultiplier();

	//스테이트에 바뀐 값 저장
	float CruochSpeed = MyPlayerState->DefaultCrouchSpeed * SpeedMultiplier;
	float WalkSpeed = MyPlayerState->DefaultWalkSpeed * SpeedMultiplier;
	float RunSpeed = MyPlayerState->DefaultRunSpeed * SpeedMultiplier;
	float SprintSpeed = MyPlayerState->DefaultSprintSpeed * SpeedMultiplier;
	float JumpZVelocity = MyPlayerState->DefaultJumpZVelocity * SpeedMultiplier;

	MyPlayerState->CrouchSpeed = CruochSpeed;
	MyPlayerState->WalkSpeed = WalkSpeed;
	MyPlayerState->RunSpeed = RunSpeed;
	MyPlayerState->SprintSpeed = SprintSpeed;
	MyPlayerState->JumpZVelocity = JumpZVelocity;

	AlsCharacterMovement->SetPlayerMovementSpeed(CruochSpeed, WalkSpeed, RunSpeed, SprintSpeed);
	AlsCharacterMovement->JumpZVelocity = JumpZVelocity;
	LOG_Char_WARNING(TEXT("SetMovementSetting 클라이언트에서 설정 완료"));
}

void ABaseCharacter::SetMovementSetting()
{
	LOG_Char_WARNING(TEXT("SetMovementSetting()"));
	if (HasAuthority())
	{
		LOG_Char_WARNING(TEXT("무브먼트 세팅 서버임()"));
		Client_SetMovementSetting();
		//return;
	}
	LOG_Char_WARNING(TEXT("SetMovementSetting() On Server"));
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		LOG_Char_WARNING(TEXT("서버에서 플레이어 스테이트 못찾음"));
		return;
	}

	SpeedMultiplier = CalculateMovementSpeedMultiplier();

	//스테이트에 바뀐 값 저장
	float CruochSpeed = MyPlayerState->DefaultCrouchSpeed * SpeedMultiplier;
	float WalkSpeed = MyPlayerState->DefaultWalkSpeed * SpeedMultiplier;
	float RunSpeed = MyPlayerState->DefaultRunSpeed * SpeedMultiplier;
	float SprintSpeed = MyPlayerState->DefaultSprintSpeed * SpeedMultiplier;
	float JumpZVelocity = MyPlayerState->DefaultJumpZVelocity * SpeedMultiplier;

	MyPlayerState->CrouchSpeed = CruochSpeed;
	MyPlayerState->WalkSpeed = WalkSpeed;
	MyPlayerState->RunSpeed = RunSpeed;
	MyPlayerState->SprintSpeed = SprintSpeed;
	MyPlayerState->JumpZVelocity = JumpZVelocity;

	AlsCharacterMovement->SetPlayerMovementSpeed(CruochSpeed, WalkSpeed, RunSpeed, SprintSpeed);
	AlsCharacterMovement->JumpZVelocity = JumpZVelocity;
	
	LOG_Char_WARNING(TEXT("SetMovementSetting 완료"));
}

float ABaseCharacter::CalculateMovementSpeedMultiplier()
{
	LOG_Char_WARNING(TEXT("스피드 연산 중..."));
	float Calculated = 1.0f;
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return Calculated;
	}
	float MyWeight = GetTotalCarryingWeight() * MyPlayerState->WeightSlowdownMultiplier;
	float WeightFactor = FMath::Clamp(1 - MyWeight / MaxWeight, 0.0f, 1.0f);
	float MyDebuff = CalculateDebuffMultiplier();
	float DebuffFactor = FMath::Clamp(MyDebuff, 0.0f, 1.0f);
	Calculated = 1.0f * WeightFactor * DebuffFactor;
	LOG_Char_WARNING(TEXT("계산한 속도 계수 리턴 : %f"), Calculated);
	return Calculated;
}

void ABaseCharacter::ResetMovementSetting()
{
	AlsCharacterMovement->ResetGaitSettings();
}

float ABaseCharacter::CalculateDebuffMultiplier()
{
	//TODO: 디버프 계산식
	return 1.0f;
}

void ABaseCharacter::Multicast_RefreshOverlayObject_Implementation()
{
	LOG_Char_WARNING(TEXT("멀티캐스트 Overlay Objects"));
	bIsSpawnDrone = true;
	RefreshOverlayObject();
}

void ABaseCharacter::Server_UnPossessDrone_Implementation()
{
	bIsSpawnDrone = false;
	NetMulticast_UnPossessDrone();
}


void ABaseCharacter::NetMulticast_UnPossessDrone_Implementation()
{
	bIsSpawnDrone = false;
	RefreshOverlayObject();
}

void ABaseCharacter::RefreshOverlayObject()
{
	LOG_Char_WARNING(TEXT("Refresh Overlay Objects"));
	AItemBase* CurrentItem = GetToolbarInventoryComponent()->GetCurrentEquippedItem();
	//static FGameplayTag CurrentItemTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Player.Equipped"));  // 참고용
	if (bIsSpawnDrone == true)
	{
		LOG_Char_WARNING(TEXT("Drone Controller"));
		SetDesiredGait(AlsOverlayModeTags::Binoculars);
		SetOverlayMode(AlsOverlayModeTags::Binoculars);
		RefreshOverlayLinkedAnimationLayer(4);
		SetDesiredAiming(false);
		AttachOverlayObject(RCController, NULL, NULL, "DroneController", true);
		return;
	}

	if (!IsValid(CurrentItem))
	{
		//아이템이 없으면, 기본 애니메이션 지정 및 소켓에 달려있는 거 삭제
		SetDesiredGait(AlsOverlayModeTags::Default);
		SetOverlayMode(AlsOverlayModeTags::Default);
		RefreshOverlayLinkedAnimationLayer(3);
		AttachOverlayObject(NULL, NULL, NULL, "Torch", true);
		LOG_Char_WARNING(TEXT("Character Equipped None"));
		return;
	}
	//아이템이 있을 때	
	FGameplayTag ItemTag = CurrentItem->ItemData.ItemType;
	LOG_Char_WARNING(TEXT("ItemTag: %s"), *ItemTag.ToString());

	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))  // 또는 HasTag 등 비교 방식에 따라
	{
		if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(CurrentItem))
		{
			AGunBase* RifleItem = Cast<AGunBase>(EquipmentItem);
			USkeletalMeshComponent* RifleMesh = RifleItem->GetSkeletalMeshComponent();
			CurrentRifleMesh = RifleMesh;
		}
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
	LOG_Char_WARNING(TEXT("Equipped Item is Valid but doesn`t match any tag"));
	SetDesiredGait(AlsOverlayModeTags::Default);
	SetOverlayMode(AlsOverlayModeTags::Default);
	RefreshOverlayLinkedAnimationLayer(3);
	AttachOverlayObject(NULL, NULL, NULL, "Torch", true);
	LOG_Char_WARNING(TEXT("Character Equipped Unknown Item"));
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
	OverlayStaticMesh->SetStaticMesh(NewStaticMesh);
	OverlayStaticMesh->AttachToComponent(GetMesh(), AttachRules, ResultSocketName);
	OverlaySkeletalMesh->SetSkinnedAssetAndUpdate(NewSkeletalMesh, true);
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
	else if (index == 4)
	{
		OverlayAnimationInstanceClass = BinocularsAnimationClass;
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
		LOG_Item_WARNING(TEXT("[ABaseCharacter::TryPickupItem] ItemActor가 NULL"));
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

	//툴바가 있으면
	if (ToolbarInventoryComponent)
	{
		//툴바에 집어넣기
		if (ToolbarInventoryComponent->TryAddItem(ItemActor))
		{
			return true;
		}
	}


	LOG_Item_WARNING(TEXT("[ABaseCharacter::TryPickupItem_Internal] 모든 인벤토리가 가득참: %s"), *ItemActor->ItemRowName.ToString());
	return false;
}

void ABaseCharacter::UseEquippedItem(float ActionValue)
{
	if (ActionValue >= 0.5f)
	{
		bIsUsingItem = true;
	}
	else
	{
		bIsUsingItem = false;
	}
	if (bIsMantling || bIsReloading || !IsEquipped() || !ToolbarInventoryComponent)
	{
		return;
	}
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_UseEquippedItem(ActionValue);
		return;
	}
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!IsValid(EquippedItem))
	{
		return;
	}
	if (ActionValue >= 0.5f)
	{
		UseItem(EquippedItem);
	}
	else
	{
		CancelUseItem(EquippedItem);
	}
}

void ABaseCharacter::Server_UseEquippedItem_Implementation(float ActionValue)
{
	UseEquippedItem(ActionValue);
}

void ABaseCharacter::UseItem(AItemBase* Item)
{
	FGameplayTag ItemGameplayTag = Item->ItemData.ItemType;

	if (ItemGameplayTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Spawnable.Drone")))
	{
		Item->UseItem();
		ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
		if (PC)
		{
			PC->SpawnDrone();
			//현재 들고 있는 인벤토리에서 제거하기

			//*  이거 삭제하고 싶은데, 제거하는 함수가 있나? *//
			ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].ItemRowName = "Default";
			ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].Quantity = 1;
			ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].Durability = 100;
			ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].bIsEquipped = false;
			ToolbarInventoryComponent->ItemSlots[ToolbarInventoryComponent->GetCurrentEquippedSlotIndex()].bIsValid = true;

			//UnequipCurrentItem(); << 없어도 되는 거 같은데, 그래도 혹시 몰라서..
			ToolbarInventoryComponent->EquippedItemComponent->DestroyChildActor();
			bIsSpawnDrone = true;
			RefreshOverlayObject();
			Multicast_RefreshOverlayObject();
			return;
		}
	}

	if (ItemGameplayTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
	{
		if (bIsSprinting)
		{
			return;
		}
		if (IsDesiredAiming() == false)
		{
			return;
		}
	}

	UseItemAfterPlayMontage(Item);
}

void ABaseCharacter::CancelUseItem(AItemBase* Item)
{
	FGameplayTag ItemGameplayTag = Item->ItemData.ItemType;
	if (ItemGameplayTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.WalkieTalkie")))
	{
		Item->UseItem();
	}
	AGunBase* Rifle = Cast<AGunBase>(Item);
	if (!IsValid(Rifle))
	{
		return;
	}
	if (Rifle->CurrentFireMode == EFireMode::FullAuto)
	{
		Rifle->StopAutoFire();
	}
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
	StopAiming();
	StopReload();
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
	RefreshOverlayObject();
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

void ABaseCharacter::DropAllItemsOnDeath()
{
	if (!ToolbarInventoryComponent) return;

	int32 NumSlots = ToolbarInventoryComponent->ItemSlots.Num();

	for (int32 i = 0; i < NumSlots; ++i)
	{
		// 슬롯이 유효하고, Default가 아니고, 수량이 1 이상인 아이템만 드랍
		FBaseItemSlotData* SlotData = ToolbarInventoryComponent->GetItemDataAtSlot(i);
		if (SlotData && SlotData->bIsValid && !ToolbarInventoryComponent->IsDefaultItem(SlotData->ItemRowName) && SlotData->Quantity > 0)
		{
			ToolbarInventoryComponent->TryDropItemAtSlot(i, SlotData->Quantity);
		}
	}
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

	float OldWeight = CurrentTotalWeight;
	CurrentTotalWeight = NewTotalWeight;
	LOG_Item_WARNING(TEXT("[OnInventoryWeightChanged] 총 무게: %.2f -> %.2f"),
		OldWeight, NewTotalWeight);
	Client_OnInventoryWeightChanged(CurrentTotalWeight);

	//// 블루프린트에서 UI 업데이트나 이동속도 조절 처리
	//OnWeightChanged(OldWeight, NewTotalWeight);
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (IsValid(MyPlayerState))
	{
		SetMovementSetting();
		UE_LOG(LogTemp, Warning, TEXT("플레이어 무브먼트 세팅 초기화 성공"));
	}
}

void ABaseCharacter::Client_OnInventoryWeightChanged_Implementation(float NewWeight)
{
	CurrentTotalWeight = NewWeight;
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

void ABaseCharacter::StartHealing(float TotalHealAmount, float Duration)
{
	if (!HasAuthority())
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(HealingTimerHandle))
	{
		return;
	}

	const float Interval = 1.0f;
	HealingTicksRemaining = FMath::CeilToInt(Duration / Interval);
	HealingPerTick = TotalHealAmount / HealingTicksRemaining;

	GetWorldTimerManager().SetTimer(HealingTimerHandle, this, &ABaseCharacter::HealStep, Interval, true);
}

void ABaseCharacter::HealStep()
{
	if (!HasAuthority())
	{
		return;
	}
	ABasePlayerState* PS = GetPlayerState<ABasePlayerState>();
	if (!IsValid(PS))
	{
		return;
	}
	const float NewHP = FMath::Clamp(PS->GetHP() + HealingPerTick, 0.0f, PS->MaxHP);
	PS->SetHP(NewHP);
	HealingTicksRemaining--;

	if (HealingTicksRemaining <= 0)
	{
		StopHealing();
	}
}

void ABaseCharacter::StopHealing()
{
	GetWorldTimerManager().ClearTimer(HealingTimerHandle);
	HealingTicksRemaining = 0;
}

void ABaseCharacter::ApplyMovementDebuff_Implementation(float SlowRate, float Duration)
{
	LOG_Art(Log, TEXT("[BaseCharacter] ApplyMovementDebuff called → SlowRate: %.2f"), SlowRate);

	if (bIsMovementDebuffed || !AlsCharacterMovement)
	{
		LOG_Art_ERROR(TEXT("AlsCharacterMovement is null"));

		return;
	}

	bIsMovementDebuffed = true;
	DebuffSlowRate = SlowRate;

	// 현재 속도값을 백업
	OriginalWalkSpeed = AlsCharacterMovement->WalkForwardSpeed;
	OriginalRunSpeed = AlsCharacterMovement->RunForwardSpeed;
	OriginalSprintSpeed = AlsCharacterMovement->SprintSpeed;


	const float NewRunSpeed = AlsCharacterMovement->RunForwardSpeed * SlowRate;
	LOG_Art(Log, TEXT("[BaseCharacter] NewRunSpeed: %.2f"), NewRunSpeed)

	// 느려진 값으로 설정
	AlsCharacterMovement->SetGaitSettings(
		OriginalWalkSpeed * SlowRate,
		OriginalWalkSpeed * SlowRate,
		OriginalRunSpeed * SlowRate,
		OriginalRunSpeed * SlowRate,
		OriginalSprintSpeed * SlowRate,
		OriginalRunSpeed * SlowRate / 2
	);
}

void ABaseCharacter::RemoveMovementDebuff_Implementation()
{
	if (!bIsMovementDebuffed || !AlsCharacterMovement)
	{
		return;
	}

	bIsMovementDebuffed = false;
	DebuffSlowRate = 1.f;

	// 원래 속도로 복원
	AlsCharacterMovement->SetGaitSettings(
		OriginalWalkSpeed,
		OriginalWalkSpeed,
		OriginalRunSpeed,
		OriginalRunSpeed,
		OriginalSprintSpeed,
		OriginalRunSpeed / 2
	);
}

void ABaseCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = OwnedTags;
}
void ABaseCharacter::SetWalkieTalkieChannelStatus(bool bActive)
{
	bool bPreviousStatus = bHasWalkieTalkieChannel;
	bHasWalkieTalkieChannel = bActive;

	// 상태가 변경된 경우에만 블루프린트 이벤트 호출
	if (bPreviousStatus != bActive)
	{
		if (bActive)
		{
			LOG_Item_WARNING(TEXT("[SetWalkieTalkieChannelStatus] 워키토키 채널 추가"));
			AddWalkieTalkieChannel();
		}
		else
		{
			LOG_Item_WARNING(TEXT("[SetWalkieTalkieChannelStatus] 워키토키 채널 제거"));
			RemoveWalkieTalkieChannel();
		}
	}
}

void ABaseCharacter::Client_SetWalkieTalkieChannelStatus_Implementation(bool bActive)
{
	bool bPreviousStatus = bHasWalkieTalkieChannel;
	bHasWalkieTalkieChannel = bActive;

	// 상태가 변경된 경우에만 블루프린트 이벤트 호출
	if (bPreviousStatus != bActive)
	{
		if (bActive)
		{
			LOG_Item_WARNING(TEXT("[Client_SetWalkieTalkieChannelStatus] 클라이언트에서 워키토키 채널 추가"));
			AddWalkieTalkieChannel();
		}
		else
		{
			LOG_Item_WARNING(TEXT("[Client_SetWalkieTalkieChannelStatus] 클라이언트에서 워키토키 채널 제거"));
			RemoveWalkieTalkieChannel();
		}
	}
}

void ABaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	LOG_Char_WARNING(TEXT("[OnRep_PlayerState] for %s"), *GetName());

	UpdateNameWidget(); // PlayerState가 복제될 때 UI 갱신
	
	if (IsLocallyControlled() && NameWidgetComponent)
	{
		NameWidgetComponent->SetVisibility(false, true);
	}
}

void ABaseCharacter::UpdateNameWidget()
{
	LOG_Char_WARNING(TEXT("[UpdateNameWidget] Called on %s"), *GetName());

	if (!NameWidgetComponent)
	{
		LOG_Char_WARNING(TEXT("[UpdateNameWidget] NameWidgetComponent is NULL"));
		return;
	}

	UUserWidget* Widget = NameWidgetComponent->GetWidget();
	if (!Widget)
	{
		LOG_Char_WARNING(TEXT("[UpdateNameWidget] Widget is NULL"));
		return;
	}

	if (UPlayerNameWidget* NameWidget = Cast<UPlayerNameWidget>(Widget))
	{
		APlayerState* PS = GetPlayerState();
		if (!PS)
		{
			LOG_Char_WARNING(TEXT("[UpdateNameWidget] PlayerState is NULL"));
			return;
		}

		const FString Name = PS->GetPlayerName();
		LOG_Char_WARNING(TEXT("[UpdateNameWidget] PlayerState name = %s"), *Name);

		NameWidget->SetPlayerName(Name);
	}
	Server_UpdateNameWidget();

}

void ABaseCharacter::Server_UpdateNameWidget_Implementation()
{
	LOG_Char_WARNING(TEXT("[UpdateNameWidget] Called on Server:: %s"), *GetName());

	if (!NameWidgetComponent)
	{
		LOG_Char_WARNING(TEXT("[UpdateNameWidget] NameWidgetComponent is NULL"));
		return;
	}

	UUserWidget* Widget = NameWidgetComponent->GetWidget();
	if (!Widget)
	{
		LOG_Char_WARNING(TEXT("[UpdateNameWidget] Widget is NULL"));
		return;
	}

	if (UPlayerNameWidget* NameWidget = Cast<UPlayerNameWidget>(Widget))
	{
		APlayerState* PS = GetPlayerState();
		if (!PS)
		{
			LOG_Char_WARNING(TEXT("[UpdateNameWidget] PlayerState is NULL"));
			return;
		}

		const FString Name = PS->GetPlayerName();
		LOG_Char_WARNING(TEXT("[UpdateNameWidget] PlayerState name = %s"), *Name);

		NameWidget->SetPlayerName(Name);
	}
}