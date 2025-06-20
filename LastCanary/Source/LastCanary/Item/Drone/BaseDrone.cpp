#include "Item/Drone/BaseDrone.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Character/BasePlayerController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Net/UnrealNetwork.h"
#include "Item/ItemSpawnerComponent.h"
#include "UI/UIElement/DroneHUD.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/UIElement/InGameHUD.h"

ABaseDrone::ABaseDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	// 스태틱 메시 생성
	DroneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	RootComponent = DroneMesh; // 이 줄 추가!

	// SpringArm과 Camera 설정
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(DroneMesh);
	SpringArm->TargetArmLength = 0.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// 기타 설정
	SetReplicatingMovement(true);
	bReplicates = true;

	SpringArm->bUsePawnControlRotation = false;
	Camera->bUsePawnControlRotation = false;
	

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = DroneMesh; // RootComponent 또는 DroneMesh로 설정

	ItemSpawner = CreateDefaultSubobject<UItemSpawnerComponent>(TEXT("ItemSpawner"));

	// 아이템 부착용 메시 컴포넌트 생성
	CarriedItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarriedItemMesh"));
	CarriedItemMesh->SetupAttachment(DroneMesh, TEXT("ItemSocket")); // ItemSocket 소켓에 부착
	CarriedItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CarriedItemMesh->SetVisibility(false);

	CarriedItem = nullptr;
}

void ABaseDrone::BeginPlay()
{
	Super::BeginPlay();

	DroneMesh->SetNotifyRigidBodyCollision(true); // 충돌 이벤트 사용하려면 true여야 함
	DroneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DroneMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// 충돌 이벤트 바인딩 (OnComponentHit)
	DroneMesh->OnComponentHit.AddDynamic(this, &ABaseDrone::OnDroneHit);

	MoveDirection = FVector::ForwardVector;

	TargetDroneRotation.Yaw = GetActorRotation().Yaw;
	if (const UWorld* World = GetWorld())
	{
		if (ULCGameInstanceSubsystem* GIS = World->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			if (ULCUIManager* UIManager = GIS->GetUIManager())
			{
				CachedDroneHUD = UIManager->GetDroneHUD();
			}
		}
	}

	GetWorldTimerManager().SetTimer(
		DroneDistanceTimerHandle,
		this,
		&ABaseDrone::UpdateDistanceCheck,
		0.2f,
		true
	);

	GetWorld()->GetTimerManager().SetTimer(
		InteractionTraceTimerHandle,
		this,
		&ABaseDrone::TraceInteractableActor,
		0.1f,
		true
	);
}

void ABaseDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// --- 수평 이동 ---
	if (!CurrentVelocity.IsNearlyZero())
	{
		CurrentVelocity = FMath::VInterpConstantTo(CurrentVelocity, FVector::ZeroVector, DeltaTime, HorizontalDeceleration);
	}
	const FVector HorizontalOffset = CurrentVelocity * DeltaTime;

	// --- 수직 이동 ---
	if (FMath::Abs(VerticalInputAxis) < 0.5f)
	{
		// 입력이 없으면 감속
		VerticalVelocity = FMath::FInterpTo(VerticalVelocity, 0.f, DeltaTime, VerticalDeceleration);
	}
	else
	{
		// 입력 방향으로 가속
		VerticalVelocity += VerticalInputAxis * VerticalAcceleration * DeltaTime;
		VerticalVelocity = FMath::Clamp(VerticalVelocity, -VerticalMaxSpeed, VerticalMaxSpeed);
	}
	const FVector VerticalOffset = FVector::UpVector * VerticalVelocity * DeltaTime;

	// 최종 이동
	AddActorWorldOffset(HorizontalOffset + VerticalOffset, true);

	////check Dist to PlayCharacter
	//FVector MyLocation = GetActorLocation();
	//float Distance = FVector::Dist(MyLocation, CharacterLocation);

	////UE_LOG(LogTemp, Log, TEXT("Distance to CharacterLocation: %.2f"), Distance);
	//if (Distance > MaxDistanceToPlayer)
	//{
	//	Multicast_ReturnToPlayer();
	//}
}

void ABaseDrone::UpdateDistanceCheck()
{
	const float DistSquared = FVector::DistSquared(GetActorLocation(), CharacterLocation);
	const float MaxDistSquared = MaxDistanceToPlayer * MaxDistanceToPlayer;

	if (DistSquared > MaxDistSquared)
	{
		Multicast_ReturnToPlayer();
		GetWorldTimerManager().ClearTimer(DroneDistanceTimerHandle);
		return;
	}

	if (CachedDroneHUD)
	{
		float CurrentDistance = FMath::Sqrt(DistSquared);
		if (!FMath::IsNearlyEqual(CurrentDistance, LastDistanceForHUD, 10.0f))
		{
			CachedDroneHUD->UpdateDistanceDisplay(CurrentDistance, MaxDistanceToPlayer);
			LastDistanceForHUD = CurrentDistance;
		}
	}
}

void ABaseDrone::Input_Move(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();
	Server_Move(InputVector);
}

void ABaseDrone::Server_Move_Implementation(FVector2D InputVector)
{
	FInputActionValue WrappedValue(InputVector);
	Move(WrappedValue);
}

void ABaseDrone::Move(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();
	// 단위 방향 (이동 방향을 나타냄)	
	const FVector Forward = GetActorForwardVector();
	const FVector Right = GetActorRightVector();

	const FVector DesiredDir = (Forward * InputVector.Y + Right * InputVector.X).GetSafeNormal();
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 현재 속도에 가속도 추가
	CurrentVelocity += DesiredDir * HorizontalAcceleration * DeltaTime;

	// 최대 속도 제한
	if (CurrentVelocity.Size() > HorizontalMaxSpeed)
	{
		CurrentVelocity = CurrentVelocity.GetClampedToMaxSize(HorizontalMaxSpeed);
	}
}

void ABaseDrone::Input_MoveUp(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	Server_MoveUp(Input);
	// Space 누를 때는 +1, 뗄 때는 0
	//VerticalInputAxis = FMath::Clamp(Value.Get<float>(), 0.f, 1.f);
}

void ABaseDrone::Server_MoveUp_Implementation(float Value)
{
	// Space 누를 때는 +1, 뗄 때는 0
	VerticalInputAxis = FMath::Clamp(Value, 0.f, 1.f);
}

void ABaseDrone::MoveUp(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();

	// Space 누를 때는 +1, 뗄 때는 0
	VerticalInputAxis = FMath::Clamp(Value.Get<float>(), 0.f, 1.f);
}

void ABaseDrone::Input_MoveDown(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	// Ctrl 누를 때는 -1, 뗄 때는 0
	Server_MoveDown(Input);
	//VerticalInputAxis = FMath::Clamp(-Value.Get<float>(), -1.f, 0.f);
}

void ABaseDrone::Server_MoveDown_Implementation(float Value)
{
	VerticalInputAxis = FMath::Clamp(-Value, -1.f, 0.f);
}

void ABaseDrone::MoveDown(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	// Ctrl 누를 때는 -1, 뗄 때는 0
	VerticalInputAxis = FMath::Clamp(-Value.Get<float>(), -1.f, 0.f);
}

void ABaseDrone::Input_Look(const FInputActionValue& Value, float Sensivity)
{
	const FVector2D LookInput = Value.Get<FVector2D>();
	Server_Look(LookInput * Sensivity);
}

void ABaseDrone::Server_Look_Implementation(FVector2D InputVector)
{
	FInputActionValue WrappedValue(InputVector);
	Look(WrappedValue);
}

void ABaseDrone::Look(const FInputActionValue& Value)
{
	const FVector2D LookInput = Value.Get<FVector2D>();

	// 드론 좌우 회전 (Yaw)
	TargetDroneRotation.Yaw += LookInput.X * LookSensitivity * GetWorld()->GetDeltaSeconds();

	// 카메라 위아래 회전 (Pitch) - Clamp로 제한
	CameraPitch = FMath::Clamp(CameraPitch + LookInput.Y * LookSensitivity * GetWorld()->GetDeltaSeconds(), -60.f, 60.f);

	// 드론 회전 적용
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw = TargetDroneRotation.Yaw;
	SetActorRotation(NewRotation);

	// 카메라가 드론에 붙어있으면, 카메라 컴포넌트의 Pitch만 업데이트
	if (Camera)
	{
		FRotator CameraRot = Camera->GetRelativeRotation();
		CameraRot.Pitch = CameraPitch;
		Camera->SetRelativeRotation(CameraRot);
	}
}	

void ABaseDrone::Interact(const FInputActionValue& Value, APlayerController* CallingController)
{
	if (!CurrentFocusedActor)
	{
		LOG_Char_WARNING(TEXT("Handle_Interact: No focused actor."));
		return;
	}

	LOG_Char_WARNING(TEXT("Interacted with: %s"), *CurrentFocusedActor->GetName());

	if (CurrentFocusedActor->Implements<UInteractableInterface>())
	{
		AActor* actor = CurrentFocusedActor;
		if (!IsValid(actor))
		{
			return;
		}
		if(IsValid(CallingController))
		
		if (CallingController)
		{
			IInteractableInterface::Execute_Interact(CurrentFocusedActor, CallingController); /// 여기는 이거 말고 드론에 붙이는 함수가 필요할 것 같습니다...
			LOG_Char_WARNING(TEXT("Handle_Interact: Called Interact on %s"), *actor->GetName());
			LOG_Char_WARNING(TEXT("Equipped item on slot"));
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

void ABaseDrone::TraceInteractableActor()
{// 카메라 컴포넌트가 Drone에 붙어있다고 가정
	if (!IsValid(Camera))
	{
		return;
	}
	FVector ViewLocation = Camera->GetComponentLocation();
	FRotator ViewRotation = Camera->GetComponentRotation();

	FVector Start = ViewLocation;
	FVector End = Start + (ViewRotation.Vector() * TraceDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

#if WITH_EDITOR
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f);
#endif

	if (bHit && Hit.GetActor() && Hit.GetActor()->Implements<UInteractableInterface>())
	{

		LOG_Char_WARNING(TEXT("Interactable object"));
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

		LOG_Char_WARNING(TEXT("no focus actor"));
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

void ABaseDrone::OnRep_CameraPitch()
{
	if (Camera)
	{
		FRotator CameraRot = Camera->GetRelativeRotation();
		CameraRot.Pitch = CameraPitch;
		Camera->SetRelativeRotation(CameraRot);
	}
}

void ABaseDrone::Server_ReturnAsItem_Implementation()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Server_ReturnAsItem] Authority가 없습니다."));
		return;
	}

	// 먼저 소지한 아이템이 있으면 드랍
	if (CarriedItem)
	{
		Server_DropItem();
	}

	// ⭐ 드론 아이템 스폰
	SpawnDroneItemAtCurrentLocation();
	Destroy();
}

void ABaseDrone::ReturnAsItem()
{
	if (HasAuthority())
	{
		// 서버에서 직접 실행
		Server_ReturnAsItem_Implementation();
	}
	else
	{
		// 클라이언트에서는 서버 RPC 호출
		Server_ReturnAsItem();
	}

	UE_LOG(LogTemp, Warning, TEXT("Drone return to Item"));
}

void ABaseDrone::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseDrone, CameraPitch);
	DOREPLIFETIME(ABaseDrone, CarriedItem);
}


void ABaseDrone::SetCharacterLocation(FVector Location)
{
	CharacterLocation = Location;
}

void ABaseDrone::Multicast_ReturnToPlayer_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (ABasePlayerController* MyPC = Cast<ABasePlayerController>(PC))
	{
		MyPC->Input_DroneExit(0);
	}
}

// CPP 구현
void ABaseDrone::OnDroneHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	FVector Normal = Hit.ImpactNormal;
	FVector CurrVelocity = CurrentVelocity;
	
	// 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("Drone hit %s"), *OtherActor->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Impact Normal: X=%.3f Y=%.3f Z=%.3f"), Normal.X, Normal.Y, Normal.Z);
	UE_LOG(LogTemp, Warning, TEXT("Current Velocity Before Reflection: X=%.3f Y=%.3f Z=%.3f"), CurrVelocity.X, CurrVelocity.Y, CurrVelocity.Z);
	// 기존 벡터 크기 저장
	float Speed = CurrVelocity.Size();

	// 반사 벡터 계산 (방향만)
	FVector ReflectedDir = CurrVelocity.MirrorByVector(Normal).GetSafeNormal();

	// 크기 유지
	FVector Reflected = ReflectedDir * Speed * 0.5f; // 반감 크기

	CurrentVelocity = Reflected;
	if (Normal.Z > 0.0f)
	{
		VerticalVelocity = -VerticalVelocity * Normal.Z * 0.5f;
	}
	else
	{
		VerticalVelocity = VerticalVelocity * Normal.Z * 0.5f;
	}
	// 반사 후 속도 로그
	UE_LOG(LogTemp, Warning, TEXT("Reflected Velocity: X=%.3f Y=%.3f Z=%.3f (Speed=%.3f)"), Reflected.X, Reflected.Y, Reflected.Z, Reflected.Size());


	// 디버그 선으로 노말 벡터 시각화 (초록색, 1초간 표시)
	const float DebugLineLength = 100.0f;
	FVector Start = Hit.ImpactPoint;
	FVector End = Start + Normal * DebugLineLength;
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 2.0f);
}

void ABaseDrone::SpawnDroneItemAtCurrentLocation()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDroneItemAtCurrentLocation] Authority가 없습니다."));
		return;
	}

	if (!ItemSpawner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDroneItemAtCurrentLocation] ItemSpawner가 없습니다."));
		return;
	}

	FVector SpawnLocation = GetActorLocation();
	AItemBase* DroneItem = ItemSpawner->CreateItemAtLocation(DroneItemRowName, SpawnLocation);
	if (DroneItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDroneItemAtCurrentLocation] 드론 아이템 스폰 성공: %s"),
			*DroneItem->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDroneItemAtCurrentLocation] 드론 아이템 스폰 실패"));
	}
}

void ABaseDrone::TogglePostProcessEffect()
{
	if (PostProcessMaterials.Num() == 0 || !Camera)
	{
		return;
	}

	int32 NextIndex = (CurrentPPIndex + 1) % (PostProcessMaterials.Num() + 1); // +1은 '없음' 상태 포함
	Server_TogglePostProcessEffect(); // 서버에 요청
}

void ABaseDrone::Server_TogglePostProcessEffect_Implementation()
{
	int32 NextIndex = (CurrentPPIndex + 1) % (PostProcessMaterials.Num() + 1);
	ApplyPostProcessMaterial(NextIndex);
}

void ABaseDrone::ApplyPostProcessMaterial(int32 NewIndex)
{
	// 이전 효과 제거
	Camera->PostProcessSettings.WeightedBlendables.Array.Empty();
	CurrentPPIndex = NewIndex;

	if (NewIndex < PostProcessMaterials.Num() && PostProcessMaterials[NewIndex])
	{
		if (DynamicPPInstances.Num() <= NewIndex || !DynamicPPInstances[NewIndex])
		{
			// 동적 인스턴스가 없다면 생성
			UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(PostProcessMaterials[NewIndex], this);
			if (DynamicPPInstances.Num() <= NewIndex)
			{
				DynamicPPInstances.SetNum(NewIndex + 1);
			}
			DynamicPPInstances[NewIndex] = Dyn;
		}

		FWeightedBlendable Blendable;
		Blendable.Object = DynamicPPInstances[NewIndex];
		Blendable.Weight = 1.0f;
		Camera->PostProcessSettings.WeightedBlendables.Array.Add(Blendable);
	}
}

void ABaseDrone::Server_PickupItem_Implementation(AItemBase* Item)
{
	if (!Item || CarriedItem != nullptr)
	{
		return;
	}

	CarriedItem = Item;
	Item->SetActorEnableCollision(false);

	// 모든 클라이언트에 동기화
	Multicast_AttachItem(Item);
}

void ABaseDrone::Server_DropItem_Implementation()
{
	if (!CarriedItem)
	{
		return;
	}

	CarriedItem->SetActorEnableCollision(true);

	// 모든 클라이언트에 동기화
	Multicast_DetachItem();

	CarriedItem = nullptr;
}

void ABaseDrone::Multicast_AttachItem_Implementation(AItemBase* Item)
{
	if (!Item)
	{
		return;
	}

	AttachItemToSocket(Item);
}

void ABaseDrone::Multicast_DetachItem_Implementation()
{
	DetachItemFromSocket();
}

void ABaseDrone::AttachItemToSocket(AItemBase* Item)
{
	if (!Item || !DroneMesh)
	{
		return;
	}

	if (UPrimitiveComponent* ActiveMesh = Item->GetActiveMeshComponent())
	{
		ActiveMesh->SetSimulatePhysics(false);
	}

	// 스켈레탈 메시를 사용하는 경우 직접 부착
	if (Item->bUsingSkeletalMesh && Item->SkeletalMeshComponent)
	{
		Item->SkeletalMeshComponent->AttachToComponent(DroneMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ItemSocket")
		);
	}
	else
	{
		// 스태틱 메시인 경우 바로 방식
		Item->AttachToComponent(DroneMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("ItemSocket")
		);
	}
}

void ABaseDrone::DetachItemFromSocket()
{
	if (!CarriedItem)
	{
		return;
	}

	// 스켈레탈 메시를 사용하는 경우
	if (CarriedItem->bUsingSkeletalMesh && CarriedItem->SkeletalMeshComponent)
	{
		// 스켈레탈 메시를 분리
		CarriedItem->SkeletalMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		// 아이템 액터 위치를 스켈레탈 메시 위치에 맞춤
		FVector SkeletalMeshLocation = CarriedItem->SkeletalMeshComponent->GetComponentLocation();
		FVector DropLocation = SkeletalMeshLocation + FVector(0, 0, -50);
		CarriedItem->SetActorLocation(DropLocation);
	}
	else
	{
		// 스태틱 메시인 경우 바로 분리
		if (DroneMesh && DroneMesh->DoesSocketExist(TEXT("ItemSocket")))
		{
			FTransform SocketTransform = DroneMesh->GetSocketTransform(TEXT("ItemSocket"));
			CarriedItem->SetActorTransform(SocketTransform);
		}

		CarriedItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		FVector DropLocation = CarriedItem->GetActorLocation() + FVector(0, 0, -50);
		CarriedItem->SetActorLocation(DropLocation);
	}

	if (UPrimitiveComponent* ActiveMesh = CarriedItem->GetActiveMeshComponent())
	{
		ActiveMesh->SetSimulatePhysics(true);
	}
}

