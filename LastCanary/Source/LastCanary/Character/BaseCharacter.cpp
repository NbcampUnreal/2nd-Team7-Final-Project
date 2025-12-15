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
#include "Item/EquipmentItem/WalkieTalkie.h"
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
#include "Character/CustomizationMeshMap.h"
#include "Inventory/BackpackManager.h"
#include "Engine/DamageEvents.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Character/Component/CharacterHealthComponent.h"
#include "Character/Component/CharacterStaminaComponent.h"
#include "Character/Component/CharacterAnimationComponent.h"
#include "Character/Component/CharacterCustomizationComponent.h"
#include "Character/Component/CharacterInteractionComponent.h"
#include "Character/Component/CharacterFootstepNoiseComponent.h"
#include "Character/Component/CharacterCameraControlComponent.h"
#include "Character/Component/CharacterDisplayComponent.h"
#include "Character/Component/CharacterNameWidgetComponent.h"
#include "Character/Component/CharacterAttackComponent.h"
#include "Character/Component/CameraRecoilComponent.h"
#include "Character/Component/CharacterInputComponent.h"
#include "Character/Component/CharacterSpeedControlComponent.h"
#include "Character/Component/CharacterSoundComponent.h"
#include "Character/Component/CharacterSanityComponent.h"
#include "Character/Component/CharacterADSComponent.h"
#include "Character/Component/CharacterWeaponClippingComponent.h"


#include "GameFramework/CharacterMovementComponent.h"

ABaseCharacter::ABaseCharacter()
{
	bIsPossessed = false;
	bReplicates = true;
	UseGunBoneforOverlayObjects = true;
	bAlwaysRelevant = true;

	InitializeComponents();

	// 캐릭터 클래스의 생성자 함수 내부 
	FieldOfView = Camera->FieldOfView;

	ItemSpawner = CreateDefaultSubobject<UItemSpawnerComponent>(TEXT("ItemSpawner"));

	BackpackMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackpackMeshComponent"));
	BackpackMeshComponent->SetupAttachment(GetMesh(), TEXT("backpack1"));
	BackpackMeshComponent->SetVisibility(false);
	BackpackMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ToolbarInventoryComponent = CreateDefaultSubobject<UToolbarInventoryComponent>(TEXT("ToolbarInventoryComponent"));
}

void ABaseCharacter::InitializeComponents()
{
	InitializeDefaultComponents();
	InitializeExtraComponents();
}

void ABaseCharacter::InitializeDefaultComponents()
{
	CustomHeadMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomHeadMesh"));
	CustomGloveMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomGloveMesh"));
	CustomJacketMesh_OwnerNoSee = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomJacketMesh_OwnerNoSee"));
	CustomJacketMesh_OwnerSee = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomJacketMesh_OwnerSee"));
	CustomPantsMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomPantsMesh"));
	CustomBeltsMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomBeltsMesh"));
	CustomHelmetMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomHelmetMesh"));
	CustomArmorMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomArmorMesh"));
	CustomBootsMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("CustomBootsMesh"));

	// Backpack
	BackpackMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("BackpackMesh"));

	// Overlay
	OverlayStaticMesh = UCommonUtility::CreateAndAttachComponent<UStaticMeshComponent>(this, GetMesh(), TEXT("OverlayStaticMesh"));
	OverlaySkeletalMesh = UCommonUtility::CreateAndAttachComponent<USkeletalMeshComponent>(this, GetMesh(), TEXT("OverlaySkeletalMesh"));
	OverlaySkeletalMesh->SetupAttachment(GetMesh(), TEXT("Rifle")); // 특수 경우 따로

	// Camera, SpringArm
	SpringArm = UCommonUtility::CreateAndAttachComponent<USpringArmComponent>(this, GetMesh(), TEXT("SpringArm"));
	Camera = UCommonUtility::CreateAndAttachComponent<UCameraComponent>(this, SpringArm, TEXT("Camera"));
	// Arrow
	ThirdPersonArrow = UCommonUtility::CreateAndAttachComponent<UArrowComponent>(this, SpringArm, TEXT("FirstPersonArrow"));

	Camera->SetRelativeRotation(FRotator::ZeroRotator);
}


void ABaseCharacter::InitializeExtraComponents()
{
	HealthComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterHealthComponent>(this, TEXT("HealthComponent"), ManagedComponents);
	StaminaComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterStaminaComponent>(this, TEXT("StaminaComponent"), ManagedComponents);
	InteractionComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterInteractionComponent>(this, TEXT("InteractionComponent"), ManagedComponents);
	AnimationComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterAnimationComponent>(this, TEXT("AnimationComponent"), ManagedComponents);
	FootstepNoiseComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterFootstepNoiseComponent>(this, TEXT("FootstepNoiseComponent"), ManagedComponents);
	CameraControlComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterCameraControlComponent>(this, TEXT("CameraControlComponent"), ManagedComponents);
	DisplayComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterDisplayComponent>(this, TEXT("DisplayComponent"), ManagedComponents);
	AttackComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterAttackComponent>(this, TEXT("AttackComponent"), ManagedComponents);
	RecoilComponent = UCommonUtility::CreateAndRegisterComponent<UCameraRecoilComponent>(this, TEXT("RecoilComponent"), ManagedComponents);
	SpeedControlComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterSpeedControlComponent>(this, TEXT("SpeedControlComponent"), ManagedComponents);
	SoundPlayComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterSoundComponent>(this, TEXT("SoundPlayComponent"), ManagedComponents);
	SanityComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterSanityComponent>(this, TEXT("SanityComponent"), ManagedComponents);
	ADSComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterADSComponent>(this, TEXT("ADSComponent"), ManagedComponents);
	WeaponClippingComponent = UCommonUtility::CreateAndRegisterComponent<UCharacterWeaponClippingComponent>(this, TEXT("WeaponClippingComponent"), ManagedComponents);

	TotalComponentCount = ManagedComponents.Num();
}


void ABaseCharacter::ApplyNetworkSmoothSettings(
	float InNetUpdateFrequency,       // 네트워크에서 위치/움직임 업데이트를 몇 Hz로 할지 (초당 전송 빈도)
	float InMinNetUpdateFrequency,    // 최소 보장 업데이트 빈도 (낮은 주기로도 업데이트 보장)
	float InNetCullDistance,          // 네트워크 업데이트를 전송할 최대 거리 (유닛 단위)
	ENetworkSmoothingMode InSmoothingMode, // 네트워크 보간/스무딩 방식 (Exponential, Linear 등)
	float InDeltaTime                 // 움직임 예측 시 보정 최대 시간 (MaxMoveDeltaTime)
)
{
	// 캐릭터 자체의 네트워크 업데이트 빈도 설정
	NetUpdateFrequency = InNetUpdateFrequency;

	// 최소 네트워크 업데이트 빈도 설정 (NetUpdateFrequency보다 낮게는 내려가지 않음)
	MinNetUpdateFrequency = InMinNetUpdateFrequency;

	// 캐릭터가 네트워크상에서 업데이트되는 거리 범위를 제어
	// 이 범위를 벗어나면 위치 업데이트를 보내지 않음으로 네트워크 부하 절감
	NetCullDistanceSquared = FMath::Square(InNetCullDistance);

	// 캐릭터 무브먼트 컴포넌트 참조
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		// 네트워크 보간 방식 설정
		// Exponential: 움직임을 부드럽게 보정, Linear: 직선 보간
		MoveComp->NetworkSmoothingMode = InSmoothingMode;

		// 클라이언트 예측 데이터 객체 가져오기
		// 클라이언트에서 서버 움직임 예측 및 보정 관련 데이터 관리
		if (FNetworkPredictionData_Client_Character* ClientData = MoveComp->GetPredictionData_Client_Character())
		{
			// 클라이언트에서 움직임 예측 최대 허용 델타 시간 설정
			// 이 시간보다 오래 누락된 움직임은 보정 처리됨
			ClientData->MaxMoveDeltaTime = InDeltaTime;
		}

		// 서버 예측 데이터 객체 가져오기
		// 서버 측에서도 움직임 예측을 비슷하게 처리하여 동기화 안정성 확보
		if (FNetworkPredictionData_Server_Character* ServerData = MoveComp->GetPredictionData_Server_Character())
		{
			// 서버에서 움직임 예측 최대 허용 델타 시간 설정
			ServerData->MaxMoveDeltaTime = InDeltaTime;
		}
	}
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, EquippedTags);
	DOREPLIFETIME(ABaseCharacter, bInventoryOpen);
	DOREPLIFETIME(ABaseCharacter, bBackpackMeshActive);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//네트워크 지연에 따른 캐릭터 이동 끊김 방지를 위한 설정
	ApplyNetworkSmoothSettings(
		120.f,    // NetUpdateFrequency
		60.f,     // MinNetUpdateFrequency
		15000.f,  // NetCullDistance
		ENetworkSmoothingMode::Exponential,
		0.05f
	);

	if (IsValid(ToolbarInventoryComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory Ready"));
		ToolbarInventoryComponent->OnInventoryUpdated.AddUniqueDynamic(this, &ABaseCharacter::HandleInventoryUpdated);
	}


	EnableStencilForAllMeshes(2);
	
	if (IsLocallyControlled())
	{
		Server_ClientLogin();
	}

	SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
}

void ABaseCharacter::NotifyComponentReady(UCharacterBaseComponent* Component)
{
	if (!Component || Component->bIsReady)
		return;

	Component->bIsReady = true;
	ComponentsReadyCount++;

	LOG_Char_WARNING(TEXT("컴포넌트 준비 완료: %s (%s) | %d/%d"),
		*Component->GetName(), *Component->GetClass()->GetName(), ComponentsReadyCount, TotalComponentCount);

	if (ComponentsReadyCount >= TotalComponentCount && !bAllComponentsReady)
	{
		bAllComponentsReady = true;
		LOG_Char_WARNING(TEXT("모든 컴포넌트 준비 완료. 캐릭터 초기화 시작"));

		UCommonUtility::RetryUntilValid(
			GetWorld(),
			[this]() { return bAllComponentsReady; },
			[this]() { InitializeCharacter(); },
			0.1f
		);
	}
}

void ABaseCharacter::InitializeCharacter()
{
	// 캐릭터 자체 초기화 로직
	LOG_Char_WARNING(TEXT("캐릭터 초기화 코드"));

	if (IsLocallyControlled())
	{
		//GetMesh()->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);
		SwapHeadMaterialTransparent(true);
	}
	
	RefreshOverlayObject();
	
	SetMovementSetting();

	if (IsValid(SpeedControlComponent))
	{
		SpeedControlComponent->SetCharacterMovementSpeed();
	}

	//ApplyCustomization(CharacterMeshMap);
	SetCharacterPoseSynchronization();

	//백팩은 커스터마이징과는 다르게 처리 // 기본은 투명
	SetBackpackMesh(false);

	if (IsLocallyControlled())
	{
		//1. 커스터마이징 데이터 로드 (로컬 환경)
		InitializePlayerCustomizing();
	}

	//컴포넌트 델리게이트 연결
	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDied.AddDynamic(this, &ABaseCharacter::HandlePlayerDeath);
	}

	if (IsValid(StaminaComponent))
	{
		StaminaComponent->OnStaminaChanged.AddDynamic(this, &ABaseCharacter::HandleStaminaConsumed);
		StaminaComponent->OnStaminaExhausted.AddDynamic(this, &ABaseCharacter::HandleStaminaExhausted);
		StaminaComponent->OnStaminaThresholdReached.AddDynamic(this, &ABaseCharacter::HandleStaminaThresholdReached);
	}

	if (IsValid(AnimationComponent))
	{
		AnimationComponent->OnReloadNotify.AddDynamic(this, &ABaseCharacter::OnReloadFromNotify);
		AnimationComponent->OnUseItemNotify.AddDynamic(this, &ABaseCharacter::OnUseItemFromNotify);
	}

	if (IsValid(InteractionComponent))
	{
		InteractionComponent->OnFocusChanged.AddDynamic(this, &ABaseCharacter::HandleFocusChanged);
	}
}

void ABaseCharacter::Server_ClientLogin_Implementation()
{
	if (HasAuthority())
	{
		if (ALCGameMode* GM = GetWorld()->GetAuthGameMode<ALCGameMode>())
		{
			LOG_Char_WARNING(TEXT("빙의 성공"));

			//GM->PlayerPossessedByPawn();
		}
	}
}

void ABaseCharacter::CheckPlayerCharacterIsReadyToGameMode()
{
	if (bPossessedCheck == true)
	{
		return;
	}
	if (HasAuthority())
	{
		if (ALCGameMode* GM = GetWorld()->GetAuthGameMode<ALCGameMode>())
		{
			LOG_Char_WARNING(TEXT("준비 성공"));
			bPossessedCheck = true;
			GM->PlayerPossessedByPawn();
		}
	}
}


void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void ABaseCharacter::InitializePlayerLocalSettings()
{
	//1. 커스터마이징 데이터 로드 (로컬 환경)
	InitializePlayerCustomizing();
}

void ABaseCharacter::InitializePlayerCustomizing()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	InitializeCustomization();
}

void ABaseCharacter::InitializeCustomization()
{
	UCommonUtility::RetryUntilValid(
		GetWorld(),
		[this]() -> bool
		{
			return bIsPlayerStateReady();
		},
		[this]()
		{
			LoadAndApplyCustomization();
		},
		0.2f
	);
}

bool ABaseCharacter::bIsPlayerStateReady() const
{
	APlayerState* PS = GetPlayerState();
	if (!IsValid(PS))
	{
		return false;
	}
	return true;
}

void ABaseCharacter::LoadAndApplyCustomization()
{
	LOG_Char_WARNING(TEXT("커스터마이징 데이터 로드"));

	// 1. 데이터 로드
	LoadCustomizationSettings();

	// 2. 적용
	ApplyCustomization(CharacterCustomizationData);

	// 3. 서버로 전송
	Server_SetCustomizationData(CharacterCustomizationData);
}

void ABaseCharacter::LoadCustomizationSettings()
{
	CharacterCustomizationData = ULCLocalPlayerSaveGame::LoadCustomizationData(GetWorld());
}

void ABaseCharacter::SaveCustomizationDataToPlayerState(const FCharacterCustomizationData& CustomizingData)
{
	if (!bIsPlayerStateReady())
	{
		return;
	}

	APlayerState* PS = GetPlayerState();
	
	if (ABasePlayerState* BPS = Cast<ABasePlayerState>(PS))
	{
		BPS->SetCustomizationData(CustomizingData);
	}
}

float ABaseCharacter::GetCurrentNoiseLevel() const
{
	return FootstepNoiseComponent ? FootstepNoiseComponent->GetCurrentFootstepNoiseLevel() : 0.f;
}

void ABaseCharacter::Server_UpdateCustomizationData_Implementation()
{
	LOG_Char_WARNING(TEXT("서버에서 전체에게 전파 준비"));

	Multicast_SetCustomizationData(CharacterCustomizationData);
}

void ABaseCharacter::Server_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData)
{
	LOG_Char_WARNING(TEXT("캐릭터 커스터마이징 데이터 서버에 전달됨"));
	CharacterCustomizationData = CustomizingData;
	Multicast_SetCustomizationData(CustomizingData);
	APlayerState* PS = GetPlayerState();

	if (ABasePlayerState* BPS = Cast<ABasePlayerState>(PS))
	{
		BPS->SetCustomizationData(CustomizingData);
	}

	//게이트 퇴장시를 위해 설정 완료되었음을 게임모드에 전파
	CheckPlayerCharacterIsReadyToGameMode();

}

void ABaseCharacter::Multicast_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData)
{
	CharacterCustomizationData = CustomizingData;
	ApplyCustomization(CharacterCustomizationData);
}

void ABaseCharacter::Server_ApplyCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData)
{
	LOG_Char_WARNING(TEXT("캐릭터 커스터마이징 데이터 서버에 전달됨"));
	//1. 서버의 캐릭터에 커스터마이징 정보 저장 (혹시 모르니까)
	CharacterCustomizationData = CustomizingData;

	//2. 서버 및 모든 클라이언트에 커스터마이징 데이터 저장 및 적용
	Multicast_SetCustomizationData(CustomizingData);

	//3. 플레이어 스테이트에 커스터마이징 값 저장
	APlayerState* PS = GetPlayerState();

	if (ABasePlayerState* BPS = Cast<ABasePlayerState>(PS))
	{
		BPS->SetCustomizationData(CustomizingData);
	}
}

void ABaseCharacter::SetCharacterPoseSynchronization()
{
	GetHeadMesh()->SetLeaderPoseComponent(GetMesh());
	GetGloveMesh()->SetLeaderPoseComponent(GetMesh());
	GetJacketMesh_OwnerNoSee()->SetLeaderPoseComponent(GetMesh());
	GetJacketMesh_OwnerSee()->SetLeaderPoseComponent(GetMesh());
	GetPantsMesh()->SetLeaderPoseComponent(GetMesh());
	GetBeltsMesh()->SetLeaderPoseComponent(GetMesh());
	GetHelmetMesh()->SetLeaderPoseComponent(GetMesh());
	GetArmorMesh()->SetLeaderPoseComponent(GetMesh());
	GetBootsMesh()->SetLeaderPoseComponent(GetMesh());
	GetBackpackMesh()->SetLeaderPoseComponent(GetMesh());
}

void ABaseCharacter::SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh)
{
	if (!Component) return;

	if (LoadedMesh)
	{
		Component->SetVisibility(true);
		Component->EmptyOverrideMaterials();
		Component->SetSkeletalMesh(LoadedMesh);
	}
	else
	{
		Component->SetLeaderPoseComponent(nullptr); // 메시 해제할 땐 잠시 끊기
		Component->SetVisibility(false);
		Component->SetSkeletalMesh(nullptr);
	}
}

void ABaseCharacter::SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material)
{
	if (!Component || !Material) return;

	// 메시가 존재하고, 표시 상태일 경우에만 적용
	if (Component && Component->IsRegistered() && Component->IsVisible() && Component->SkeletalMesh)
	{
		Component->SetMaterial(MaterialIndex, Material);
	}
}

void ABaseCharacter::ApplyCustomizationToAllPlayers(const FCharacterCustomizationData CustomizationData)
{
	ApplyCustomization(CustomizationData);
	Server_ApplyCustomizationData(CustomizationData);
}

void ABaseCharacter::ApplyCustomization(const FCharacterCustomizationData CustomizationData)
{
	LOG_Char_WARNING(TEXT("캐릭터 커스터마이징 어플라이"));
	if (!CharacterMeshMap || !CharacterMeshMap->IsValidLowLevel())
	{
		return;
	}

	int BodyId = CustomizationData.DefaultBodyID;
	int HeadId = CustomizationData.DefaultBodyID;
	int HelmetId = CustomizationData.HelmetID;
	int GloveId = CustomizationData.GloveID;
	int JacketId = CustomizationData.JacketID;
	int PantsId = CustomizationData.PantsID;
	int BeltsId = CustomizationData.BeltsID;
	int ArmorId = CustomizationData.ArmorID;
	int BootsId = CustomizationData.BootsID;
	// Body
	USkeletalMesh* BodySkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->DefaultBodyMeshes, BodyId);
	USkeletalMesh* HeadSkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->DefaultHeadMeshes, HeadId);
	USkeletalMesh* HelmetSkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->HelmetMeshes, HelmetId);
	USkeletalMesh* GloveSkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->GloveMeshes, GloveId);
	USkeletalMesh* JacketSkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->JacketMeshes_OwnerSee, JacketId);
	USkeletalMesh* JacketSkeletalMesh_OwnerNosee = CharacterMeshMap->GetMeshByID(CharacterMeshMap->JacketMeshes, JacketId);
	USkeletalMesh* PantsSkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->PantsMeshes, PantsId);
	USkeletalMesh* BeltsSkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->BeltsMeshes, BeltsId);
	USkeletalMesh* ArmorSkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->ArmorMeshes, ArmorId);
	USkeletalMesh* BootsSkeletalMesh = CharacterMeshMap->GetMeshByID(CharacterMeshMap->BootsMeshes, BootsId);

	SetPartMesh(GetMesh(), BodySkeletalMesh);
	SetPartMesh(GetHeadMesh(), HeadSkeletalMesh);
	SetPartMesh(GetHelmetMesh(), HelmetSkeletalMesh);
	SetPartMesh(GetGloveMesh(), GloveSkeletalMesh);
	SetPartMesh(GetJacketMesh_OwnerNoSee(), JacketSkeletalMesh_OwnerNosee);
	SetPartMesh(GetJacketMesh_OwnerSee(), JacketSkeletalMesh);
	SetPartMesh(GetPantsMesh(), PantsSkeletalMesh);
	SetPartMesh(GetBeltsMesh(), BeltsSkeletalMesh);
	SetPartMesh(GetArmorMesh(), ArmorSkeletalMesh);
	SetPartMesh(GetBootsMesh(), BootsSkeletalMesh);



	// 전제: CustomizationData 안에 머티리얼 ID도 들어있음
	int BodyMatId = CustomizationData.DefaultBodyMaterialID;
	int HeadMatId = CustomizationData.DefaultBodyMaterialID;
	int HelmetMatId = CustomizationData.HelmetMaterialID;
	int GloveMatId = CustomizationData.GloveMaterialID;
	int JacketMatId = CustomizationData.JacketMaterialID;
	int PantsMatId = CustomizationData.PantsMaterialID;
	int BeltsMatId = CustomizationData.BeltsMaterialID;
	int ArmorMatId = CustomizationData.ArmorMaterialID;
	int BootsMatId = CustomizationData.BootsMaterialID;
	int FlagMatId = CustomizationData.FlagMaterialID;

	// 머티리얼도 매핑용 에셋에서 가져옴 (이미 블루프린트에서 세팅되어 있다고 가정)
	UMaterialInterface* BodyMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->DefaultBodyMaterials, BodyMatId);
	UMaterialInterface* HeadMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->DefaultBodyMaterials, HeadMatId);
	UMaterialInterface* HelmetMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->HelmetMaterials, HelmetMatId);
	UMaterialInterface* GloveMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->GloveMaterials, GloveMatId);
	UMaterialInterface* JacketMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->JacketMaterials, JacketMatId);
	UMaterialInterface* PantsMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->PantsMaterials, PantsMatId);
	UMaterialInterface* BeltsMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->BeltsMaterials, BeltsMatId);
	UMaterialInterface* ArmorMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->ArmorMaterials, ArmorMatId);
	UMaterialInterface* BootsMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->BootsMaterials, BootsMatId);
	UMaterialInterface* FlagMat = CharacterMeshMap->GetMaterialByID(CharacterMeshMap->FlagMaterials, FlagMatId);

	// 머티리얼 적용 함수 호출 (보통 0번 슬롯만 적용한다고 가정)
	SetPartMaterial(GetMesh(), 0, BodyMat);
	SetPartMaterial(GetHeadMesh(), 0, HeadMat);
	SetPartMaterial(GetHelmetMesh(), 0, HelmetMat);
	SetPartMaterial(GetGloveMesh(), 0, GloveMat);
	SetPartMaterial(GetJacketMesh_OwnerNoSee(), 0, JacketMat);
	SetPartMaterial(GetJacketMesh_OwnerSee(), 0, JacketMat);
	SetPartMaterial(GetPantsMesh(), 0, PantsMat);
	SetPartMaterial(GetBeltsMesh(), 0, BeltsMat);
	SetPartMaterial(GetArmorMesh(), 1, ArmorMat);
	SetPartMaterial(GetBootsMesh(), 0, BootsMat);

	//플래그
	SetPartMaterial(GetHelmetMesh(), 1, FlagMat);
	SetPartMaterial(GetArmorMesh(), 0, FlagMat);

	SetCharacterPoseSynchronization();
}

void ABaseCharacter::SetBrightness(float Value)
{
	float Normalized = FMath::Clamp(Value, 0.0f, 1.0f);

	if (IsValid(DisplayComponent))
	{
		DisplayComponent->ApplyBrightness(Normalized);
	}
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
	Super::CalcCamera(DeltaTime, ViewInfo);
	if (!IsLocallyControlled()) return;
	
	if (bIsMantling)
	{
		bIsAiming = false;
		bIsTransitioning = false;
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
		StopGunAutoFire();
		return;
	}
	if (bIsMining)
	{
		FRotator ControlRot = GetControlRotation();
		FRotator NewRot = FRotator(0.f, ControlRot.Yaw, 0.f);
		SetActorRotation(NewRot);
		ViewInfo.Rotation.Pitch = FMath::Clamp(ViewInfo.Rotation.Pitch, -30.f, 80.f);

		// 컨트롤러 회전도 이 값으로 덮어쓰기
		if (Controller)
		{
			FRotator ControlRotataion = Controller->GetControlRotation();
			ControlRotataion.Pitch = ViewInfo.Rotation.Pitch;
			Controller->SetControlRotation(ControlRotataion);
		}

	}
	ViewInfo.Rotation.Roll = 0.0f;
}

void ABaseCharacter::ResetCameraLocationToDefault()
{
	SpringArm->bUsePawnControlRotation = true;
	bIsAiming = false;
	bIsTransitioning = false;
}

AItemBase* ABaseCharacter::GetCurrentItem()
{
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	return EquippedItem;
}

AGunBase* ABaseCharacter::GetCurrentGunItem()
{
	if (IsValid(GetCurrentItem()))
	{
		AGunBase* Gun = Cast<AGunBase>(GetCurrentItem());
		return Gun;
	}		
	return nullptr;
}

USkeletalMeshComponent* ABaseCharacter::GetCurrentGunItemSkeletalMesh()
{
	if (IsValid(GetCurrentGunItem()))
	{
		USkeletalMeshComponent* RifleMesh = GetCurrentGunItem()->GetSkeletalMeshComponent();
		return RifleMesh;
	}
	return nullptr;
}


void ABaseCharacter::Handle_Aim(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Aim())
	{
		return;
	}

	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!EquippedItem)
	{
		return;
	}

	AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem);
	if (!EquipmentItem)
	{
		return;
	}
	if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
	{
		//곡괭이를 들면 근접공격
		if (AttackComponent)
		{
			if (ActionValue.Get<float>() > 0.5f)
			{
				AttackComponent->Handle_Attack(EAttackType::ItemAttack);
			}
			return;
		}
	}



	//총을 들면 우클릭


	if (bIsSprinting || bIsReloading || bIsClose || bIsMantling)
	{
		CameraControlComponent->StopAiming();
		return;
	}
	AGunBase* Gun = Cast<AGunBase>(EquipmentItem);
	if (!IsValid(Gun))
	{
		return;
	}
	if (Gun)
	{
		USkeletalMeshComponent* RifleMesh = Gun->GetSkeletalMeshComponent();
		CurrentRifleMesh = RifleMesh;


		if (ActionValue.Get<float>() > 0.5f && bIsCloseToWall == false)
		{
			if (ADSComponent)
			{
				ADSComponent->SwitchADS(true);
			}
		}
		else
		{
			if (ADSComponent)
			{
				ADSComponent->SwitchADS(false);
			}
		}

	}
	/*
	if(EquipmentItem)
	if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle"))
		|| EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol"))
		|| EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
	{
		AGunBase* RifleItem = Cast<AGunBase>(EquippedItem);

	}
	*/
}

void ABaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetDesiredAiming(true);
}// 전환이 완료되었는지 확인하는 유틸리티 함수 (선택사항)

void ABaseCharacter::SetIsCloseToWall(bool _bIsCloseToWall)
{
	bIsCloseToWall = _bIsCloseToWall;
}

bool ABaseCharacter::GetIsCloseToWall()
{
	return bIsCloseToWall;
}

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
		SoundLoudness*6,                   // float: 소리의 크기 (기본값은 1.0f, 감지 거리 등에 영향을 줌)
		SoundCauser,                 // AActor*: 소리의 주체 (보통 this)
		MaxSoundRange,                   // float: 소리를 들을 수 있는 최대 거리
		FName("Boss")                         // FName: 태그로 필터링 가능 (선택사항)
	);
}

void ABaseCharacter::Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity, float ZoomSensivity)
{
	if (!CheckCondition_LookMouse())
	{
		return;
	}

	const FVector2f Value{ ActionValue.Get<FVector2D>() };

	if (bIsAiming)
	{
		AddControllerYawInput(Value.X * ZoomSensivity * MouseSensitivityMultiplier * MouseInvertMultiplier);
		AddControllerPitchInput(Value.Y * ZoomSensivity * MouseSensitivityMultiplier * MouseInvertMultiplier);
	}
	else
	{
		AddControllerYawInput(Value.X * Sensivity * MouseSensitivityMultiplier * MouseInvertMultiplier);
		AddControllerPitchInput(Value.Y * Sensivity * MouseSensitivityMultiplier * MouseInvertMultiplier);
	}
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

// 스무스하게 반동주기
void ABaseCharacter::ApplySmoothRecoil(float Vertical, float Horizontal)
{
	if (RecoilComponent)
	{
		RecoilComponent->ApplyRecoil(Vertical);
	}
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

void ABaseCharacter::Handle_Attack(const FInputActionValue& ActionValue)
{
	if (ADSComponent)
	{
		ADSComponent->SwitchADS(false);
	}
	if (AttackComponent)
	{
		AttackComponent->Handle_Attack(EAttackType::Kick);
	}
}

void ABaseCharacter::Handle_Emote(const FInputActionValue& ActionValue)
{
	const float Value = ActionValue.Get<float>();
	//if Value > 0.5f -> Emote UI 오픈 & 입력 차단(정확히는 캐릭터의 행동 차단)

	//Value < 0.5 -> Emote UI 해제 및 선택된 춤 재생 // 춤 재생 동안에는 다른 행동 금지 or 이동이나 다른 조작 시에 끊기게끔...
}

void ABaseCharacter::Handle_Move(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Move())
	{
		return;
	}

	const auto Value{ UAlsVector::ClampMagnitude012D(ActionValue.Get<FVector2D>()) };

	CancelInteraction();
	FrontInput = Value.Y;
	const auto ForwardDirection{ UAlsVector::AngleToDirectionXY(UE_REAL_TO_FLOAT(GetViewState().Rotation.Yaw)) };
	const auto RightDirection{ UAlsVector::PerpendicularCounterClockwiseXY(ForwardDirection) };

	AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}

void ABaseCharacter::Handle_Sprint(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Sprint())
	{
		return;
	}
	if (!IsValid(StaminaComponent))
	{
		return;
	}

	const float Value = ActionValue.Get<float>();

	if (CheckHardLandState())
	{
		bIsSprinting = false;
		SetDesiredGait(AlsGaitTags::Running);
		StaminaComponent->StopStaminaDrain();
		StaminaComponent->StartStaminaRecoverAfterDelay();
		return;
	}

	StopGunAutoFire(); // 총 연사상태면 해제하기

	if (Value < 0.5f) //입력이 떼지는 거면 어차피 뛰는 거 아님..
	{
		bIsSprinting = false;
		SetDesiredGait(AlsGaitTags::Running);
		StaminaComponent->StopStaminaDrain();
		StaminaComponent->StartStaminaRecoverAfterDelay();
		return;
	}

	if (StaminaComponent->bIsExhausted) //만약 지친 상태라면 불가
	{
		return;
	}
	//달리기 시작하면서 스테미나 소모 시작
	StaminaComponent->StartStaminaDrain();
	StaminaComponent->StopStaminaRecovery();
	StaminaComponent->StopStaminaRecoverAfterDelay();
}

void ABaseCharacter::Handle_Walk(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Walk())
	{
		return;
	}

	const float Value = ActionValue.Get<float>();

	if (Value > 0.5f)
	{
		SetDesiredGait(AlsGaitTags::Walking);
	}
	else
	{
		SetDesiredGait(AlsGaitTags::Running);
	}
}

void ABaseCharacter::Handle_Crouch(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Crouch())
	{
		return;
	}

	CancelInteraction();

	const float Value = ActionValue.Get<float>();
	if (Value > 0.5f)
	{
		SetDesiredStance(AlsStanceTags::Crouching);
	}
	else
	{
		SetDesiredStance(AlsStanceTags::Standing);
	}
}

void ABaseCharacter::Handle_Jump(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Jump())
	{
		return;
	}

	const float Value = ActionValue.Get<float>();

	CancelInteraction();

	if (Value > 0.5f)
	{
		if (StopRagdolling())
		{
			return;
		}
		if (StartMantlingGrounded())
		{
			SetDesiredAiming(false);
			//GetCharacter()->SpringArm->AttachToComponent(GetCharacter()->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
			return;
		}
		if (GetStance() == AlsStanceTags::Crouching)
		{
			SetDesiredStance(AlsStanceTags::Standing);
			return;
		}
		if (StaminaComponent->CanJump())
		{
			Jump();
			if (!CanJump())
			{
				return;
			}
			StaminaComponent->ConsumeStaminaOnJump();
		}
	}
	else
	{
		StopJumping();
	}
}

void ABaseCharacter::HandleStaminaConsumed()
{
	float CurrentPlayerSpeed = GetPlayerMovementSpeed();
	if (FrontInput < 0.1f)
	{
		StaminaComponent->bCanCharacterSprint = false;
		bIsSprinting = false;
		SetDesiredGait(AlsGaitTags::Running);
		//일단 회복 시키기는 해
		StaminaComponent->StartStaminaRecoverAfterDelay();
		return;
	}
	StaminaComponent->bCanCharacterSprint = true;
	bIsSprinting = true;
	SetDesiredAiming(false);
	SetDesiredGait(AlsGaitTags::Sprinting);
	StaminaComponent->StopStaminaRecovery();
	StaminaComponent->StopStaminaRecoverAfterDelay();
	StaminaComponent->StartStaminaRecoverAfterDelayOnJump();
	
	StaminaComponent->StartStaminaDrain();
}

void ABaseCharacter::HandleStaminaExhausted()
{
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	MyPlayerState->SetPlayerMovementState(ECharacterMovementState::Exhausted);
}

void ABaseCharacter::HandleStaminaThresholdReached()
{

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
	if (ADSComponent)
	{
		ADSComponent->SwitchADS(false);
	}
	//bIsReloading = true;
	/*
	Server_PlayReload();
	*/
	AnimationComponent->PlayGunReloadMontage();
}

//////////////////////////////////////애니메이션 사이에 노티파이 클래스로 집어넣어야됨//////////////////////////////////
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
	Gun->DropMagazine();
}
//////////////////////////////////////애니메이션 사이에 노티파이 클래스로 집어넣어야됨//////////////////////////////////





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
	if (!CheckCondition_ViewMode())
	{
		return;
	}
	//아래 코드 카메라 컴포넌트 함수로 변경
	bIsFPSCamera = !(bIsFPSCamera);
	SetCameraMode(bIsFPSCamera);
}

void ABaseCharacter::SetCameraMode(bool bIsFirstPersonView)
{
	if (bIsFirstPersonView)
	{
		EmoteMode = false;
		CustomHeadMesh->SetOwnerNoSee(true);
		SwapHeadMaterialTransparent(true);
		//SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
		SpringArm->TargetArmLength = 0.0f;
	}
	else
	{
		EmoteMode = true;
		CustomHeadMesh->SetOwnerNoSee(false);
		SwapHeadMaterialTransparent(false);
		SpringArm->TargetArmLength = 200.0f;
	}
}

void ABaseCharacter::SetCameraEmoteMode(bool bIsFirstPersonView)
{
	if (bIsFirstPersonView)
	{
		EmoteMode = false;
		SpringArm->bDoCollisionTest = false;
		SpringArm->ProbeChannel = ECC_Camera;
		SpringArm->ProbeSize = 3.0f;
		CustomHeadMesh->SetOwnerNoSee(true);
		SwapHeadMaterialTransparent(true);
		//SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
		SpringArm->TargetArmLength = 0.0f;
	}
	else
	{
		EmoteMode = true;
		SpringArm->bDoCollisionTest = true;
		SpringArm->ProbeChannel = ECC_Camera;
		SpringArm->ProbeSize = 3.0f;
		CustomHeadMesh->SetOwnerNoSee(false);
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

void ABaseCharacter::Handle_Interact(const FInputActionValue& ActionValue)
{
	if (!CheckCondition_Interact())
	{
		return;
	}

	if (InteractionComponent->CurrentFocusedActor->Implements<UInteractableInterface>())
	{
		AActor* actor = InteractionComponent->CurrentFocusedActor;
		if (!IsValid(actor))
		{
			return;
		}

		if (GetController())
		{
			//CancelInteraction();
			//IInteractableInterface::Execute_Interact(CurrentFocusedActor, PC);
			LOG_Char_WARNING(TEXT("Handle_Interact: Called Interact on %s"), *actor->GetName());
			//GetCharacter()->InteractAfterPlayMontage(actor);
			//GetCharacter()->AnimationComponent->PlayInteractMontage(actor);

			InteractionComponent->Handle_Interact();
		}
	}
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
	bIsPlayingAnimation = false;
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
	bIsPlayingAnimation = false;
	bIsMining = false;
	AnimInstance->Montage_Stop(0.2f, CurrentInteractMontage); // 부드럽게 블렌드 아웃
}

void ABaseCharacter::Client_SetMiningState_Implementation(bool NewValue)
{
	bIsMining = NewValue;
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
	bIsPlayingAnimation = false;
	bIsMining = false;
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
	bIsPlayingAnimation = false;
	AnimInstance->Montage_Stop(0.2f, CurrentUseItemMontage); // 부드럽게 블렌드 아웃
}

void ABaseCharacter::OnReloadFromNotify()
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

void ABaseCharacter::OnUseItemFromNotify()
{
	if (IsValid(CurrentUsingItem))
	{
		CurrentUsingItem->UseItem();
	}
}

void ABaseCharacter::HandleFocusChanged(AActor* NewFocus)
{
	// 상태 필터링
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating || bIsSpawnDrone)
	{
		return;
	}

	// HUD 업데이트
	if (NewFocus && NewFocus->Implements<UInteractableInterface>())
	{
		FString Message = IInteractableInterface::Execute_GetInteractMessage(NewFocus);
		if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			if (ULCUIManager* UIManager = Subsystem->GetUIManager())
			{
				if (UInGameHUD* HUD = Cast<UInGameHUD>(UIManager->GetInGameHUD()))
				{
					HUD->SetInteractMessage(Message);
					HUD->SetInteractMessageVisible(true);
					LOG_Char_WARNING(TEXT("SetInteractMessage to %s"), *Message);
				}
			}
		}
	}
	else
	{
		LOG_Char_WARNING(TEXT("변경"));
		if (CurrentFocusedActor)
		{
			CurrentFocusedActor = nullptr;
		}
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
	CancelUseItem();
	CancelInteraction();
	StopReload();
	if (ADSComponent)
	{
		ADSComponent->SwitchADS(false);
	}
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
	CheckAndStopWalkieTalkie();
	ToolbarInventoryComponent->EquipItemAtSlot(Index);
	// 동기화된 장착 요청
	Multicast_ResetAnimationAndCamera(Index);
}

void ABaseCharacter::Multicast_ResetAnimationAndCamera_Implementation(int32 Index)
{
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
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		//만약 재생중인 몽타주가 있으면(예시: 장전모션) 강제로 해제
		AnimInstance->Montage_Stop(0.25f); // 페이드 아웃 시간: 0.25초 //AnimInstance->Montage_Stop(0.25f, ReloadMontage);이런 것도 가능
		bIsPlayingAnimation = false;
	}
}

void ABaseCharacter::HandleInventoryUpdated()
{
	LOG_Item_WARNING(TEXT("Inventory updated!"));
	RefreshOverlayObject();
}

void ABaseCharacter::UnequipCurrentItem()
{
	if (!IsEquipped() || !ToolbarInventoryComponent)
	{
		LOG_Item_WARNING(TEXT("현재 장비 상태가 아니거나 툴바가 없습니다."));
		return;
	}

	CheckAndStopWalkieTalkie();

	// 클라이언트에서 호출된 경우 서버에 요청
	if (GetLocalRole() < ROLE_Authority)
	{
		LOG_Item_WARNING(TEXT("[ABaseCharacter::UnequipCurrentItem] 클라이언트에서 서버로 요청"));
		Server_UnequipCurrentItem();
		return;
	}

	// 현재 장착된 아이템 정보 가져오기 (로그용)
	AItemBase* CurrentEquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	FString ItemName = CurrentEquippedItem ? CurrentEquippedItem->ItemRowName.ToString() : TEXT("Unknown");

	// 툴바 컴포넌트에서 실제 해제 처리
	ToolbarInventoryComponent->UnequipCurrentItem();
}

void ABaseCharacter::Server_UnequipCurrentItem_Implementation()
{
	UnequipCurrentItem();
}

float ABaseCharacter::TakeSanityDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())	
	{
		return 0.0f;
	}
	if (!IsValid(SanityComponent))
	{
		return 0.0f;
	}

	SanityComponent->TakeSanityDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	return DamageAmount;
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}	
	if (IsValid(HealthComponent))
	{
		HealthComponent->TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
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

	HealthComponent->TakeFallDamage(Velocity);
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

void ABaseCharacter::SetDamageEnabled(bool bEnabled)
{
	if (!IsValid(HealthComponent))
	{
		return;
	}

	if (bEnabled)
	{
		HealthComponent->bInfiniteHP = false;
	}
	else
	{
		HealthComponent->bInfiniteHP = true;
	}
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
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
	SpeedMultiplier = CalculateMovementSpeedMultiplier();

	//스테이트에 바뀐 값 저장
	float CrouchSpeed = MyPlayerState->DefaultCrouchSpeed * SpeedMultiplier;
	float WalkSpeed = MyPlayerState->DefaultWalkSpeed * SpeedMultiplier;
	float RunSpeed = MyPlayerState->DefaultRunSpeed * SpeedMultiplier;
	float SprintSpeed = MyPlayerState->DefaultSprintSpeed * SpeedMultiplier;
	float JumpZVelocity = MyPlayerState->DefaultJumpZVelocity * SpeedMultiplier;

	MyPlayerState->CrouchSpeed = CrouchSpeed;
	MyPlayerState->WalkSpeed = WalkSpeed;
	MyPlayerState->RunSpeed = RunSpeed;
	MyPlayerState->SprintSpeed = SprintSpeed;
	MyPlayerState->JumpZVelocity = JumpZVelocity;

	AlsCharacterMovement->SetPlayerMovementSpeed(CrouchSpeed, WalkSpeed, RunSpeed, SprintSpeed);
	AlsCharacterMovement->JumpZVelocity = JumpZVelocity;
}

void ABaseCharacter::SetMovementSetting()
{
	if (HasAuthority())
	{
		Client_SetMovementSetting();
		//return;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}

	SpeedMultiplier = CalculateMovementSpeedMultiplier();

	//스테이트에 바뀐 값 저장
	float CrouchSpeed = MyPlayerState->DefaultCrouchSpeed * SpeedMultiplier;
	float WalkSpeed = MyPlayerState->DefaultWalkSpeed * SpeedMultiplier;
	float RunSpeed = MyPlayerState->DefaultRunSpeed * SpeedMultiplier;
	float SprintSpeed = MyPlayerState->DefaultSprintSpeed * SpeedMultiplier;
	float JumpZVelocity = MyPlayerState->DefaultJumpZVelocity * SpeedMultiplier;

	MyPlayerState->CrouchSpeed = CrouchSpeed;
	MyPlayerState->WalkSpeed = WalkSpeed;
	MyPlayerState->RunSpeed = RunSpeed;
	MyPlayerState->SprintSpeed = SprintSpeed;
	MyPlayerState->JumpZVelocity = JumpZVelocity;

	AlsCharacterMovement->SetPlayerMovementSpeed(CrouchSpeed, WalkSpeed, RunSpeed, SprintSpeed);
	AlsCharacterMovement->JumpZVelocity = JumpZVelocity;
	
}

float ABaseCharacter::CalculateMovementSpeedMultiplier()
{
	float Calculated = 1.0f;
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return Calculated;
	}
	float MyWeight = GetTotalCarryingWeight() * MyPlayerState->WeightSlowdownMultiplier;
	float WeightFactor = FMath::Clamp(1 - MyWeight / MaxWeight, 0.5f, 1.0f);
	float MyDebuff = CalculateDebuffMultiplier();
	float DebuffFactor = FMath::Clamp(MyDebuff, 0.0f, 1.0f);
	Calculated = 1.0f * WeightFactor * DebuffFactor;
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

	//static FGameplayTag CurrentItemTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Player.Equipped"));  // 참고용
	AItemBase* CurrentItem = GetToolbarInventoryComponent()->GetCurrentEquippedItem();
	FGameplayTag ItemTag;
	FGameplayTag Overlay = AlsOverlayModeTags::Default;
	bool bIsDesireAiming = false;
	FName Socketname = "ik_hand_gun";
	bool bUseLeftGunBone = true;
	UStaticMesh* AttachMesh = NULL;
	USkeletalMesh* AttachSkeletalMesh = NULL;
	OverlaySkeletalMesh->SetOwnerNoSee(false);
	OverlaySkeletalMesh->SetOnlyOwnerSee(false);
	if (IsValid(CurrentItem))
	{
		ItemTag = CurrentItem->ItemData.ItemType;
	}
	if (!ItemTag.IsValid())
	{
		SetDesiredGait(Overlay);
		SetOverlayMode(Overlay);
		RefreshOverlayLinkedAnimationLayer(ItemTag);
		SetDesiredAiming(bIsDesireAiming);
		AttachOverlayObject(AttachMesh, NULL, NULL, Socketname, bUseLeftGunBone);
	}
	

	if (bIsSpawnDrone == true)
	{
		LOG_Char_WARNING(TEXT("Drone Controller"));
		Overlay = AlsOverlayModeTags::Binoculars;
		AttachMesh = RCController;
		Socketname = "DroneController";
		bUseLeftGunBone = true;
	}
	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))  // 또는 HasTag 등 비교 방식에 따라
	{
		if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(CurrentItem))
		{
			AGunBase* RifleItem = Cast<AGunBase>(EquipmentItem);
			USkeletalMeshComponent* RifleMesh = RifleItem->GetSkeletalMeshComponent();
			CurrentRifleMesh = RifleMesh;
			AttachSkeletalMesh = EquipmentItem->ItemData.SkeletalMesh;
		}
		OverlaySkeletalMesh->SetOwnerNoSee(true);
		OverlaySkeletalMesh->SetOnlyOwnerSee(true);
		Overlay = AlsOverlayModeTags::Rifle;
		bIsDesireAiming = true;
		Socketname = "ik_hand_gun";

		SetHasGunOnHand(true);
	}
	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")))  // 또는 HasTag 등 비교 방식에 따라
	{
		if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(CurrentItem))
		{
			AGunBase* RifleItem = Cast<AGunBase>(EquipmentItem);
			USkeletalMeshComponent* RifleMesh = RifleItem->GetSkeletalMeshComponent();
			CurrentRifleMesh = RifleMesh;
			Socketname = "Pistol";
			AttachSkeletalMesh = EquipmentItem->ItemData.SkeletalMesh;
		}
		OverlaySkeletalMesh->SetOwnerNoSee(true);
		OverlaySkeletalMesh->SetOnlyOwnerSee(true);
		Overlay = AlsOverlayModeTags::PistolTwoHanded;
		bIsDesireAiming = true;
		SetHasGunOnHand(true);
	}
	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))  // 또는 HasTag 등 비교 방식에 따라
	{
		if (AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(CurrentItem))
		{
			AGunBase* RifleItem = Cast<AGunBase>(EquipmentItem);
			USkeletalMeshComponent* RifleMesh = RifleItem->GetSkeletalMeshComponent();
			CurrentRifleMesh = RifleMesh;
			Socketname = "Shotgun";
			AttachSkeletalMesh = EquipmentItem->ItemData.SkeletalMesh;
		}
		OverlaySkeletalMesh->SetOwnerNoSee(true);
		OverlaySkeletalMesh->SetOnlyOwnerSee(true);
		Overlay = AlsOverlayModeTags::Rifle;
		bIsDesireAiming = true;
		SetHasGunOnHand(true);
	}
	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.FlashLight")))
	{
		Overlay = AlsOverlayModeTags::Torch;
	}
	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Spawnable.Drone")))
	{
		Overlay = AlsOverlayModeTags::PistolOneHanded;
	}
	if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
	{
		Overlay = AlsOverlayModeTags::Barrel;
	}
	SetDesiredGait(Overlay);
	SetOverlayMode(Overlay);
	RefreshOverlayLinkedAnimationLayer(ItemTag);
	SetDesiredAiming(bIsDesireAiming);
	AttachOverlayObject(AttachMesh, AttachSkeletalMesh, NULL, Socketname, bUseLeftGunBone);

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
	OverlaySkeletalMesh->SetAnimInstanceClass(NewAnimationClass);
	OverlaySkeletalMesh->AttachToComponent(GetMesh(), AttachRules, ResultSocketName);
}

void ABaseCharacter::RefreshOverlayLinkedAnimationLayer(FGameplayTag ItemTag)
{
	if (AnimationComponent)
	{
		AnimationComponent->RefreshOverlayLinkedAnimationLayer(ItemTag);
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

	if (ItemGameplayTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")) || ItemGameplayTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")) || ItemGameplayTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
	{
		LOG_Char_WARNING(TEXT("[Aim Check] Rifle tag detected"));

		if (bIsSprinting)
		{
			LOG_Char_WARNING(TEXT("[Aim Check] Blocked - Sprinting"));
			return;
		}

		if (AnimationComponent->GetIsPlayingGunReloadMontage())
		{
			LOG_Char_WARNING(TEXT("[Aim Check] Blocked - Reloading"));
			return;
		}

		// 다른 행동 체크
		if (AnimationComponent)
		{
			if (AnimationComponent->GetIsPlayingAttackMontage())
			{
				LOG_Char_WARNING(TEXT("[Aim Check] Blocked - Attack Montage Playing"));
				return;
			}
		}
		else
		{
			LOG_Char_WARNING(TEXT("[Aim Check] Warning - AnimationComponent is null"));
		}

		if (IsDesiredAiming() == false)
		{
			LOG_Char_WARNING(TEXT("[Aim Check] Blocked - IsDesiredAiming() == false"));
			return;
		}

		LOG_Char_WARNING(TEXT("[Aim Check] Passed all checks"));
	}


	FItemDataRow Data = Item->ItemData;
	if (Data.bPlayCharacterAnimation == true)
	{
		AnimationComponent->PlayUseItemMontage(Item); //애니메이션 컴포넌트로 먼저 실행 후, 노티파이로 아이템 사용
	}
	else
	{
		Item->UseItem();
	}
}

void ABaseCharacter::CancelUseItem(AItemBase* Item)
{
	FGameplayTag ItemGameplayTag = Item->ItemData.ItemType;
	if (ItemGameplayTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.WalkieTalkie")))
	{
		Item->UseItem();
	}
	if (ItemGameplayTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
	{
		CancelInteraction();
	}
	AGunBase* Rifle = Cast<AGunBase>(Item);
	if (!IsValid(Rifle))
	{
		return;
	}
	if (Rifle->CurrentFireMode == EFireMode::FullAuto)
	{
		Rifle->StopAutoFire();

	}	// 연사 리셋 타이머
}

void ABaseCharacter::StopGunAutoFire()
{
	AGunBase* Rifle = Cast<AGunBase>(ToolbarInventoryComponent->GetCurrentEquippedItem());
	if (IsValid(Rifle))
	{
		if (Rifle->CurrentFireMode == EFireMode::FullAuto)
		{
			Rifle->StopAutoFire();
		}
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
	CheckAndStopWalkieTalkie();
	CancelUseItem();
	CancelInteraction();
	CameraControlComponent->StopAiming();
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

	if (SlotIndex == ToolbarInventoryComponent->GetCurrentEquippedSlotIndex())
	{
		CheckAndStopWalkieTalkie();
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

void ABaseCharacter::SetBackpackMesh(bool bIsEquipBackpack)
{
	//서버에서 실행
	if (bBackpackMeshActive == bIsEquipBackpack)
	{
		return;
	}

	bBackpackMeshActive = bIsEquipBackpack;

	if (bIsEquipBackpack)
	{
		BackpackMesh->SetSkeletalMesh(BackpackSkeletalMesh);
		BackpackMesh->SetVisibility(true);
		LOG_Char_WARNING(TEXT("가방 메시 활성화"));
	}
	else
	{
		BackpackMesh->SetSkeletalMesh(nullptr);
		BackpackMesh->SetVisibility(false);
		LOG_Char_WARNING(TEXT("가방 메시 비활성화"));
	}

	Multicast_SetBackpackMesh(bBackpackMeshActive);
}

void ABaseCharacter::Multicast_SetBackpackMesh_Implementation(bool bIsEquipBackpack)
{
	SetBackpackMesh(bIsEquipBackpack);
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

	TargetNode->HarvestResource(GetController<APlayerController>());
}

void ABaseCharacter::StartHealing(float TotalHealAmount, float Duration)
{
	if (HealthComponent)
	{
		HealthComponent->StartHealing(TotalHealAmount, Duration);
	}
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

void ABaseCharacter::CheckAndStopWalkieTalkie()
{
	AItemBase* CurrentItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (AWalkieTalkie* WalkieTalkie = Cast<AWalkieTalkie>(CurrentItem))
	{
		if (WalkieTalkie->IsWalkieTalkieActive() && WalkieTalkie->IsOwnedByLocalPlayer())
		{
			LOG_Item_WARNING(TEXT("워키토키 사용 중지: 장비 해제/교체/드랍 (bUseWalkie: %s)"),
				WalkieTalkie->IsWalkieTalkieActive() ? TEXT("true") : TEXT("false"));
			WalkieTalkie->StopWalkieTalkie();
		}
	}
}

bool ABaseCharacter::IsCurrentWalkieTalkieActive() const
{
	AItemBase* CurrentItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (AWalkieTalkie* WalkieTalkie = Cast<AWalkieTalkie>(CurrentItem))
	{
		return WalkieTalkie->IsWalkieTalkieActive() && WalkieTalkie->IsOwnedByLocalPlayer();
	}
	return false;
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

	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{
		SetCustomizationData(PS->GetCustomizationData());
		ApplyCustomization(PS->GetCustomizationData());
		
	}
}

bool ABaseCharacter::CheckCondition_LookMouse()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (bIsPlayingInteractionMontage)
	{
		return false;
	}
	if (GetLocomotionAction() == AlsLocomotionActionTags::Mantling)
	{
		return false;
	}

	return true;
}

bool ABaseCharacter::CheckCondition_Move()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (CheckHardLandState())
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::CheckCondition_Sprint()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::CheckCondition_Walk()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (CheckHardLandState())
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::CheckCondition_Crouch()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}

	if (CheckHardLandState())
	{
		return false;
	}

	return true;
}

bool ABaseCharacter::CheckCondition_Jump()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (CheckHardLandState())
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::CheckCondition_Aim()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (CheckHardLandState())
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::CheckCondition_Interact()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (!InteractionComponent->CurrentFocusedActor)
	{
		return false;
	}

	LOG_Char_WARNING(TEXT("Interacted with: %s"), *InteractionComponent->CurrentFocusedActor->GetName());

	if (bIsPlayingInteractionMontage)
	{
		return false;
	}

	return true;
}

bool ABaseCharacter::CheckCondition_ViewMode()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}

	return true;
}

bool ABaseCharacter::CheckCondition_Reload()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	if (bIsReloading)
	{
		return false;
	}
	if (bIsUsingItem)
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::CheckCondition_VoiceChatting()
{
	if (!Check_DefaultCondition())
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::Check_PlayerController()
{
	if (!GetController())
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::Check_PlayerState()
{
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating || CheckPlayerCurrentState() == EPlayerInGameStatus::None)
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::Check_InputEnabled()
{
	if (!IsInputEnabled())
	{
		return false;
	}
	return true;
}

bool ABaseCharacter::Check_DefaultCondition()
{
	if (!Check_PlayerController())
	{
		return false;
	}
	if (!Check_PlayerState())
	{
		return false;
	}
	if (!Check_InputEnabled())
	{
		return false;
	}
	return true;
}