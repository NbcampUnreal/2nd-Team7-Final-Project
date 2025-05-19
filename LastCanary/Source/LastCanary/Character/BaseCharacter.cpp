#include "Character/BaseCharacter.h"
#include "BasePlayerController.h"
#include "ALSCamera/Public/AlsCameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/BoxComponent.h"

//#include "ALS/Public/Utility/AlsVector.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/Utility/AlsVector.h"
//innclude "ALSCamera/Public/AlsCameraComponent.h"

#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"
#include "Item/ItemBase.h"

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


/////////////////////////////

void ABaseCharacter::Handle_LookMouse(const FInputActionValue& ActionValue)
{
	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	AddControllerPitchInput(Value.Y * LookUpMouseSensitivity);
	AddControllerYawInput(Value.X * LookRightMouseSensitivity);
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

	AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}

void ABaseCharacter::Handle_Sprint(const FInputActionValue& ActionValue)
{
	SetDesiredGait(ActionValue.Get<bool>() ? AlsGaitTags::Sprinting : AlsGaitTags::Running);
}

void ABaseCharacter::Handle_Walk()
{
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
	SetDesiredAiming(ActionValue.Get<bool>());
}

void ABaseCharacter::Handle_Ragdoll()
{
	if (!StopRagdolling())
	{
		StartRagdolling();
	}
}

void ABaseCharacter::Handle_Roll()
{
	static constexpr auto PlayRate{ 1.3f };

	StartRolling(PlayRate);
}

void ABaseCharacter::Handle_RotationMode()
{
	SetDesiredRotationMode(GetDesiredRotationMode() == AlsRotationModeTags::VelocityDirection
		? AlsRotationModeTags::ViewDirection
		: AlsRotationModeTags::VelocityDirection);
}

void ABaseCharacter::Handle_ViewMode()
{
	SetViewMode(GetViewMode() == AlsViewModeTags::ThirdPerson ? AlsViewModeTags::FirstPerson : AlsViewModeTags::ThirdPerson);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABaseCharacter::Handle_SwitchShoulder()
{
	Camera->SetRightShoulder(!Camera->IsRightShoulder());
}

void ABaseCharacter::Handle_Interact(AActor* HitActor)
{
	UE_LOG(LogTemp, Log, TEXT("Interacted!!!!"));
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
	UE_LOG(LogTemp, Log, TEXT("Overlap 상태"));
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
		UE_LOG(LogTemp, Warning, TEXT("There is Interactive Object"));
	}
	// 필요하면 조건에 따라 타이머를 멈출 수도 있음
}

void ABaseCharacter::SetPossess(bool IsPossessed)
{
	bIsPossessed = IsPossessed;
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
	static FGameplayTag EquippedTag = FGameplayTag::RequestGameplayTag(TEXT("Charater.Player.Equipped"));
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
	if (!HitItem) return false;

	UE_LOG(LogTemp, Log, TEXT("TryPickupItem 실행"));

	// 1. 가방 인벤토리 컴포넌트가 있고, 아이템이 수집품이면 우선 가방에 시도
	if (BackpackInventoryComponent)
	{
		if (BackpackInventoryComponent->TryAddItem(HitItem))
		{
			UE_LOG(LogTemp, Log, TEXT("가방에 아이템 추가 성공!"));
			return true;
		}
	}

	// 2. 툴바 인벤토리 컴포넌트로 시도
	if (ToolbarInventoryComponent)
	{
		if (ToolbarInventoryComponent->TryAddItem(HitItem))
		{
			UE_LOG(LogTemp, Log, TEXT("툴바에 아이템 추가 성공!"));
			return true;
		}
	}

	// 3. 모두 실패
	UE_LOG(LogTemp, Warning, TEXT("아이템 습득 실패!"));
	return false;
}
