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

#include "BasePlayerState.h"

#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"
#include "Item/ItemBase.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ABaseCharacter::ABaseCharacter()
{
	bIsPossessed = false;

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

	ToolbarInventoryComponent = CreateDefaultSubobject<UToolbarInventoryComponent>(TEXT("ToolbarInventoryComponent"));
	BackpackInventoryComponent = nullptr;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (InteractDetectionBox && GetMesh())
	{
		InteractDetectionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("head"));
		InteractDetectionBox->SetRelativeLocation(FVector(0.f, 100.f, 0.f));  // 
		InteractDetectionBox->SetRelativeRotation(FRotator(0.f, -10.f, 0.f));  // 
	}

	InteractDetectionBox->OnComponentBeginOverlap.AddDynamic(this, &ABaseCharacter::OnInteractBoxBeginOverlap);
	InteractDetectionBox->OnComponentEndOverlap.AddDynamic(this, &ABaseCharacter::OnInteractBoxEndOverlap);
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
	/*
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y * LookUpMouseSensitivity);
	AddControllerYawInput(Value.X * LookRightMouseSensitivity);
	*/

	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	if (!Controller) return;

	// 현재 컨트롤러 회전
	FRotator CurrentRotation = Controller->GetControlRotation();
	float CurrentPitch = CurrentRotation.GetNormalized().Pitch;

	// 입력값 계산
	const float NewPitchInput = Value.Y * LookUpMouseSensitivity;

	// Pitch 제한 적용
	float NewPitch = FMath::Clamp(CurrentPitch + NewPitchInput, -50.f, 50.f);

	// Yaw는 그대로
	float NewYaw = CurrentRotation.Yaw + Value.X * LookRightMouseSensitivity;

	// 새 회전값 적용
	FRotator NewRotation = FRotator(NewPitch, NewYaw, 0.f);
	Controller->SetControlRotation(NewRotation);


}

void ABaseCharacter::Handle_Look(const FInputActionValue& ActionValue)
{
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y * LookUpRate);
	AddControllerYawInput(Value.X * LookRightRate);
}

void ABaseCharacter::Handle_Move(const FInputActionValue& ActionValue)
{
	const auto Value{ UAlsVector::ClampMagnitude012D(ActionValue.Get<FVector2D>()) };

	const auto ForwardDirection{ UAlsVector::AngleToDirectionXY(UE_REAL_TO_FLOAT(GetViewState().Rotation.Yaw)) };
	const auto RightDirection{ UAlsVector::PerpendicularCounterClockwiseXY(ForwardDirection) };
	if (bIsInHardLandingState)
	{
		// 착지 중에는 이동 금지
		return;
	}
	AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}

void ABaseCharacter::Handle_Sprint(const FInputActionValue& ActionValue)
{
	if (bIsInHardLandingState)
	{
		// 착지 중에는 조작 금지
		return;
	}
	SetDesiredGait(ActionValue.Get<bool>() ? AlsGaitTags::Sprinting : AlsGaitTags::Running);
}

void ABaseCharacter::Handle_Walk()
{
	if (bIsInHardLandingState)
	{
		// 착지 중에는 조작 금지
		return;
	}
	if (GetDesiredGait() == AlsGaitTags::Walking)
	{
		SetDesiredGait(AlsGaitTags::Running);
	}
	else if (GetDesiredGait() == AlsGaitTags::Running)
	{
		SetDesiredGait(AlsGaitTags::Walking);
	}
}

void ABaseCharacter::Handle_Crouch()
{
	if (bIsInHardLandingState)
	{
		// 착지 중에는 조작 금지
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
	// 임시로 넣은 부분 꼭 삭제할것!
	UseEquippedItem();
}

void ABaseCharacter::Handle_Jump(const FInputActionValue& ActionValue)
{
	if (bIsInHardLandingState)
	{
		// 착지 중에는 조작 금지
		return;
	}
	if (ActionValue.Get<bool>())
	{
		if (StopRagdolling())
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
	if (bIsInHardLandingState)
	{
		// 착지 중에는 조작 금지
		return;
	}
	SetDesiredAiming(ActionValue.Get<bool>());
	// 임시로 넣은 부분 꼭 삭제할것!
	UseEquippedItem();
}

void ABaseCharacter::Handle_ViewMode()
{
	SetViewMode(GetViewMode() == AlsViewModeTags::ThirdPerson ? AlsViewModeTags::FirstPerson : AlsViewModeTags::ThirdPerson);
}


void ABaseCharacter::Handle_Strafe(const FInputActionValue& ActionValue)
{
	const auto Value{ UAlsVector::ClampMagnitude012D(ActionValue.Get<FVector2D>()) };

	// 임시로 넣은 부분 꼭 삭제할것!
	UseEquippedItem();
}

void ABaseCharacter::Handle_Interact(AActor* HitActor)
{
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
	}
	//TO DO...
	/*
	if(Hit.Type == ItemClass)
		PickupItem(Hit);
	*/	

	// 임시 추가 코드
	AItemBase* HitItem = Cast<AItemBase>(HitActor);
	if (!HitItem)
	{
		return;
	}

	const FItemDataRow* ItemData = HitItem->ItemDataTable->FindRow<FItemDataRow>(HitItem->ItemRowName, TEXT("Handle_Interact"));
	if (!ItemData)
	{
		return;
	}

	const FGameplayTag ItemTypeTag = FGameplayTag::RequestGameplayTag(FName("ItemType"));
	if (!ItemData->ItemType.MatchesTag(ItemTypeTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_Interact: HitItem에 ItemType 태그가 없음!"));
		return;
	}

	TryPickupItem(HitItem);
}

void ABaseCharacter::PickupItem()
{
	//TO DO...
	//interact
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
		//HideInteractUI();
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
			//TO DO...
			//GetGameInstance->GetUIManager->적당한 UI 띄우는 함수...();
		*/
	}
	// 필요하면 조건에 따라 타이머를 멈출 수도 있음
}

void ABaseCharacter::SetPossess(bool IsPossessed)
{
	bIsPossessed = IsPossessed;
}



void ABaseCharacter::EquipItemFromCurrentQuickSlot(int QuickSlotIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Change Equip Item"));

	TestEquipFunction(QuickSlotIndex);
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
	//메시 부착, 무기 생성, 이펙트 적용 등 추가 로직 필요
	UE_LOG(LogTemp, Log, TEXT("Equipped item: %s"), *GetNameSafe(Item));
	//ALS의 attach Component 함수를 참고 혹은 불러오기... 혹은 Overlay모드 변화
}

void ABaseCharacter::UnequipCurrentItem()
{
	HeldItem = nullptr;
	// 손에서 제거, 메시 해제, 이펙트 제거 등 처리
	UE_LOG(LogTemp, Log, TEXT("Unequipped current item"));
}


float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (MyPlayerState)
	{
		MyPlayerState->ApplyDamage(DamageAmount);
	}

	return DamageAmount;
}

void ABaseCharacter::HandlePlayerDeath()
{
	//플레이어 사망 처리 로직 적용 후 관전 상태 진입
	UE_LOG(LogTemp, Warning, TEXT("Character is dead"));

	// 입력 막기 등 처리
	AController* MyController = GetController();
	if (MyController)
	{
		MyController->UnPossess();
	}
}





void ABaseCharacter::SetBackpackInventoryComponent(UBackpackInventoryComponent* BackpackInvenComp, bool bEquip)
{
	if (bEquip)
	{
		BackpackInventoryComponent = BackpackInvenComp;
	}
	else
	{
		BackpackInventoryComponent = nullptr;
	}
}

// 게임 플레이 태그 관련 함수
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
		LOG_Item_WARNING(TEXT("[ABaseCharacter::SetEquipped] 태그 추가"));
		EquippedTags.AddTag(EquippedTag);
	}
	else
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::SetEquipped] 태그 제거"));
		EquippedTags.RemoveTag(EquippedTag);
	}
}

bool ABaseCharacter::TryPickupItem(AItemBase* HitItem)
{
	if (!HitItem) return false;

	UE_LOG(LogTemp, Log, TEXT("TryPickupItem 실행"));

	// 클라이언트에서 호출된 경우 서버에 요청
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_TryPickupItem(HitItem);
		return true; // 요청 성공 (실제 결과는 서버에서 결정)
	}

	if (BackpackInventoryComponent)
	{
		if (BackpackInventoryComponent->TryAddItem(HitItem))
		{
			UE_LOG(LogTemp, Log, TEXT("가방에 아이템 추가 성공!"));
			return true;
		}
	}

	if (ToolbarInventoryComponent)
	{
		if (ToolbarInventoryComponent->TryAddItem(HitItem))
		{
			UE_LOG(LogTemp, Log, TEXT("툴바에 아이템 추가 성공!"));
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("아이템 습득 실패!"));
	return false;
}

void ABaseCharacter::Server_TryPickupItem_Implementation(AItemBase* HitItem)
{
	if (!HitItem)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::ServerTryPickupItem] HitItem이 nullptr입니다."));
		return;
	}
	TryPickupItem(HitItem);
}

bool ABaseCharacter::UseEquippedItem()
{
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
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] 클라이언트에서 실행"));
		Server_UseEquippedItem();
		return true;
	}

	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!EquippedItem)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] 현재 장착된 아이템이 없습니다."));
		return false;
	}

	if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem))
	{
		EquipmentItem->UseItem();
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] %s 아이템 사용 성공"),
			*EquippedItem->ItemRowName.ToString());
		return true;
	}
	else
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UseEquippedItem] 아이템이 장비 아이템이 아닙니다."));
		return false;
	}
}

void ABaseCharacter::Server_UseEquippedItem_Implementation()
{
	UseEquippedItem();
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseCharacter, EquippedTags);
}
