#include "Character/BaseCharacter.h"
#include "BasePlayerController.h"
#include "ALSCamera/Public/AlsCameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/BoxComponent.h"

#include "Interface/InteractableInterface.h"

//#include "ALS/Public/Utility/AlsVector.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/Utility/AlsVector.h"
//innclude "ALSCamera/Public/AlsCameraComponent.h"

#include "ALS/Public/AlsCharacterMovementComponent.h"
#include "BasePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/Utility/AlsConstants.h"

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

	//Camera Settings
	Camera = CreateDefaultSubobject<UAlsCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetMesh());
	Camera->SetRelativeRotation_Direct({ 0.0f, 90.0f, 0.0f });

	//OverlapBox for Interact Settings
	InteractDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractDetectionBox"));
	InteractDetectionBox->SetupAttachment(RootComponent);
	InteractDetectionBox->SetBoxExtent(FVector(50.f, 100.f, 50.f)); // 얇고 길게
	InteractDetectionBox->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	InteractDetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractDetectionBox->SetCollisionObjectType(ECC_WorldDynamic);
	InteractDetectionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractDetectionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractDetectionBox->SetGenerateOverlapEvents(true);
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
	if (InteractDetectionBox && GetMesh())
	{
		InteractDetectionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("head"));
		InteractDetectionBox->SetRelativeLocation(FVector(0.f, 100.f, 0.f));  // 
		InteractDetectionBox->SetRelativeRotation(FRotator(0.f, -10.f, 0.f));  // 
	}

	InteractDetectionBox->OnComponentBeginOverlap.AddDynamic(this, &ABaseCharacter::OnInteractBoxBeginOverlap);
	InteractDetectionBox->OnComponentEndOverlap.AddDynamic(this, &ABaseCharacter::OnInteractBoxEndOverlap);

	CurrentQuickSlotIndex = 0;

	//애니메이션 오버레이 활성화.
	RefreshOverlayObject(CurrentQuickSlotIndex);
}


void ABaseCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
}


void ABaseCharacter::CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo)
{
	
	if (Camera->IsActive())
	{
		Camera->GetViewInfo(ViewInfo);
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

void ABaseCharacter::Handle_Interact(AActor* HitActor)
{
	if (CheckPlayerCurrentState() == EPlayerState::Dead)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Interacted!!!!"));
	if (!HitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: HitActor is nullptr"));
		return;
	}

	if (HitActor->Implements<UInteractableInterface>())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			IInteractableInterface::Execute_Interact(HitActor, PC);
			UE_LOG(LogTemp, Log, TEXT("Handle_Interact: Called Interact on %s"), *HitActor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: Controller is nullptr"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: HitActor %s does not implement IInteractableInterface"), *HitActor->GetName());
		PlayInteractionMontage(HitActor);
	}
	//TO DO...
	/*
	if(Hit.Type == ItemClass)
		PickupItem(Hit);
	*/	
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

void ABaseCharacter::OnInteractBoxBeginOverlap(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
		return;

	// Optional: 인터페이스 확인
	/*
	if (!OtherActor->Implements<UInteractable>())
		return;
	*/


	// 타이머가 이미 돌아가고 있으면 다시 시작하지 않음
	if (!GetWorld()->GetTimerManager().IsTimerActive(OverlapCheckTimerHandle))
	{
		// 0.1초마다 반복 실행 (주기는 필요에 따라 조절)
		GetWorld()->GetTimerManager().SetTimer(OverlapCheckTimerHandle, this, &ABaseCharacter::OverlapCheckFunction, 0.1f, true);
	}

}


void ABaseCharacter::OnInteractBoxEndOverlap(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == CurrentFocusedActor)
	{
		//TODO: HideInteractUI();
		CurrentFocusedActor = nullptr;
	}

	GetWorld()->GetTimerManager().ClearTimer(OverlapCheckTimerHandle);
}


void ABaseCharacter::OverlapCheckFunction()
{
	// 오버랩 중일 때 해야 할 반복 작업 수행
	if (!bIsPossessed)
	{
		return;
	}
	// 라인트레이스로 시야 안에 정확히 들어왔는지 확인
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + (Camera->GetForwardVector() * 200.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_GameTraceChannel1, Params);

	if (bHit)
	{
		// 감지 성공 → UI 표시
		/*
			//TODO: UI띄우기
			//GetGameInstance->GetUIManager->적당한 UI 띄우는 함수...();
		*/
	}
	// 필요하면 조건에 따라 타이머를 멈출 수도 있음
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

float ABaseCharacter::GetFallDamage(float Amount)
{
	Super::GetFallDamage(Amount);
	if (bIsGetFallDownDamage == false)
	{
		return 0;
	}
	UE_LOG(LogTemp, Log, TEXT("player Take Fall Damage : %f"), Amount);
	ABasePlayerController* PC = Cast<ABasePlayerController>(GetController());
	if (PC)
	{
		UE_LOG(LogTemp, Log, TEXT("Controller Existed"));
		ABasePlayerState* MyPlayerState = PC->GetPlayerState<ABasePlayerState>();
		if (MyPlayerState)
		{
			UE_LOG(LogTemp, Log, TEXT("State Existed"));
			MyPlayerState->ApplyDamage(Amount);
		}
	}
	return Amount;
}

void ABaseCharacter::HandlePlayerDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Character is dead"));
	Camera->SetRelativeRotation_Direct({ 0.0f, 90.0f, 0.0f });
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



/// <summary>
/// 물체 별 인터렉션애니메이션 몽타주 재생 함수
/// </summary>
/// <param name="Target"></param>

void ABaseCharacter::PlayInteractionMontage(AActor* Target)
{
	if (!Target || !GetMesh() || !GetMesh()->GetAnimInstance())
		return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

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
		AnimInstance->Montage_Play(MontageToPlay);
	}
	
}