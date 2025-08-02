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


ABaseCharacter::ABaseCharacter()
{
	bIsPossessed = false;
	bReplicates = true;
	UseGunBoneforOverlayObjects = true;
	bAlwaysRelevant = true;
	NetCullDistanceSquared = FMath::Square(20000.f); // 최대 동기화 거리 증가

	CustomHeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomHeadMesh"));
	CustomHeadMesh->SetupAttachment(GetMesh());

	CustomGloveMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomGloveMesh"));
	CustomGloveMesh->SetupAttachment(GetMesh());

	CustomJacketMesh_OwnerNoSee = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomJacketMesh_OwnerNoSee"));
	CustomJacketMesh_OwnerNoSee->SetupAttachment(GetMesh());

	CustomJacketMesh_OwnerSee = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomJacketMesh_OwnerSee"));
	CustomJacketMesh_OwnerSee->SetupAttachment(GetMesh());

	CustomPantsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomPantsMesh"));
	CustomPantsMesh->SetupAttachment(GetMesh());

	CustomBeltsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomBeltsMesh"));
	CustomBeltsMesh->SetupAttachment(GetMesh());

	CustomHelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomHelmetMesh"));
	CustomHelmetMesh->SetupAttachment(GetMesh());
	
	CustomArmorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomArmorMesh"));
	CustomArmorMesh->SetupAttachment(GetMesh());

	CustomBootsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomBootsMesh"));
	CustomBootsMesh->SetupAttachment(GetMesh());

	////* 가방 메시 *////
	BackpackMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackpackMesh"));
	BackpackMesh->SetupAttachment(GetMesh());

	SetCharacterPoseSynchronization();

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

	KickHitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("KickHitBox"));
	KickHitBox->SetupAttachment(GetMesh(), TEXT("foot_l")); // or "foot_l"
	KickHitBox->SetBoxExtent(FVector(20, 30, 30));
	KickHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	KickHitBox->SetCollisionObjectType(ECC_WorldDynamic);
	KickHitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	KickHitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);


	//캐릭터 컴포넌트
	HealthComponent = CreateDefaultSubobject<UCharacterHealthComponent>(TEXT("HealthComponent"));
	StaminaComponent = CreateDefaultSubobject<UCharacterStaminaComponent>(TEXT("StaminaComponent"));
	InteractionComponent = CreateDefaultSubobject<UCharacterInteractionComponent>(TEXT("InteractionComponent"));
	AnimationComponent = CreateDefaultSubobject<UCharacterAnimationComponent>(TEXT("AnimationComponent"));
	CustomizationComponent = CreateDefaultSubobject<UCharacterCustomizationComponent>(TEXT("CustomizationComponent"));
	FootstepNoiseComponent = CreateDefaultSubobject<UCharacterFootstepNoiseComponent>(TEXT("FootstepNoiseComponent"));
	CameraControlComponent = CreateDefaultSubobject<UCharacterCameraControlComponent>(TEXT("CameraControlComponent"));
	//DisplayComponent = CreateDefaultSubobject<UCharacterDisplayComponent>(TEXT("DisplayComponent"));
	NameComponent = CreateDefaultSubobject<UCharacterNameWidgetComponent>(TEXT("NameWidgetComponent"));
	AttackComponent = CreateDefaultSubobject<UCharacterAttackComponent>(TEXT("AttackComponent"));
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
		ToolbarInventoryComponent->OnInventoryUpdated.AddUniqueDynamic(this, &ABaseCharacter::HandleInventoryUpdated);
	}

	EnableStencilForAllMeshes(2);


	if (IsLocallyControlled() && CustomPostProcessComponent)
	{
		CustomPostProcessComponent->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
		CustomPostProcessComponent->Settings.bOverride_AutoExposureMinBrightness = true;
		CustomPostProcessComponent->Settings.bOverride_AutoExposureMaxBrightness = true;
		CustomPostProcessComponent->Settings.bOverride_AutoExposureBias = true;
		// 블렌드 웨이트 1.0으로 보정 적용 보장
		CustomPostProcessComponent->BlendWeight = 1.0f;
		CustomPostProcessComponent->Priority = 100.0f;
	}
	SetMovementSetting();

	if (NameComponent)
	{
		NameComponent->InitializeWidget();
	}

	if (NameComponent && IsValid(NameComponent->GetWidget()))
	{
		APlayerState* PS = GetPlayerState();
		if (IsValid(PS))
		{
			NameComponent->SetPlayerName(PS->GetPlayerName());
		}

		if (IsLocallyControlled())
		{
			NameComponent->SetWidgetVisibility(false);
		}

		NameComponent->SetCastShadowEnabled(false);
	}

	//ApplyCustomization(CharacterMeshMap);
	SetCharacterPoseSynchronization();

	//백팩은 커스터마이징과는 다르게 처리 // 기본은 투명
	SetBackpackMesh(false);

	KickHitBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ABaseCharacter::OnKickHitBoxOverlap);


	if (IsLocallyControlled())
	{
		Server_ClientLogin();
		InitializePlayerLocalSettings();
	}

	//컴포넌트 델리게이트 연결
	if (HealthComponent)
	{
		HealthComponent->OnDied.AddDynamic(this, &ABaseCharacter::HandlePlayerDeath);
	}

	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaChanged.AddDynamic(this, &ABaseCharacter::HandleStaminaConsumed);
	}

	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaExhausted.AddDynamic(this, &ABaseCharacter::HandleStaminaExhausted);
	}

	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaThresholdReached.AddDynamic(this, &ABaseCharacter::HandleStaminaThresholdReached);
	}
	
	if (AnimationComponent)
	{
		AnimationComponent->OnReloadNotify.AddDynamic(this, &ABaseCharacter::OnReloadFromNotify);
		AnimationComponent->OnInteractionNotify.AddDynamic(this, &ABaseCharacter::OnInteractionFromNotify);
		AnimationComponent->OnUseItemNotify.AddDynamic(this, &ABaseCharacter::OnUseItemFromNotify);
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

	//2. 플레이어 네임 위젯 초기화
	InitializePlayerNameWidget();
}

void ABaseCharacter::InitializePlayerNameWidget()
{
	if (NameComponent)
	{
		NameComponent->InitializeNameWidget();
	}
}

void ABaseCharacter::InitializePlayerCustomizing()
{
	//로컬 환경에서만
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerState* PS = GetPlayerState();
	if (IsValid(PS)) // 플레이어 스테이트가 존재하면
	{
		LOG_Char_WARNING(TEXT("커스터마이징 데이터 로드"));
		CharacterCustomizationData = ULCLocalPlayerSaveGame::LoadCustomizationData(GetWorld());

		//2. 로드한 커스터마이징 데이터를 적용
		ApplyCustomization(CharacterCustomizationData);

		//3. 서버로 커스터마이징 데이터 전송 (서버 RPC)
		Server_SetCustomizationData(CharacterCustomizationData);
	}
	else //존재하지 않으면 몇초 뒤 다시 시도
	{
		// PlayerState가 아직 준비 안 됐으므로 타이머로 재시도
		GetWorldTimerManager().SetTimer(
			RetryInitializeCustomizingHandle,
			this,
			&ABaseCharacter::InitializePlayerCustomizing,
			0.2f,    // 0.2초 후에 재시도
			false    // 반복 호출 아님 (한 번만 실행)
		);
	}
}

void ABaseCharacter::SetCharacterPoseSynchronization()
{
	CustomHeadMesh->SetLeaderPoseComponent(GetMesh());
	CustomGloveMesh->SetLeaderPoseComponent(GetMesh());
	CustomJacketMesh_OwnerNoSee->SetLeaderPoseComponent(GetMesh());
	CustomJacketMesh_OwnerSee->SetLeaderPoseComponent(GetMesh());
	CustomPantsMesh->SetLeaderPoseComponent(GetMesh());
	CustomBeltsMesh->SetLeaderPoseComponent(GetMesh());
	CustomHelmetMesh->SetLeaderPoseComponent(GetMesh());
	CustomArmorMesh->SetLeaderPoseComponent(GetMesh());
	CustomBootsMesh->SetLeaderPoseComponent(GetMesh());
	BackpackMesh->SetLeaderPoseComponent(GetMesh());
}

float ABaseCharacter::GetCurrentNoiseLevel() const
{
	return FootstepNoiseComponent ? FootstepNoiseComponent->GetCurrentNoiseLevel() : 0.f;
}

FCharacterCustomizationData ABaseCharacter::GetCustomizationData()
{
	return CharacterCustomizationData;
}

void ABaseCharacter::SetCustomizationData(const FCharacterCustomizationData& CustomizingData)
{
	CharacterCustomizationData = CustomizingData;
}

void ABaseCharacter::Server_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData)  //이 부분의 안의 내용을 커스터마이징 컴포넌트의 함수로 변경하기
{
	LOG_Char_WARNING(TEXT("캐릭터 커스터마이징 데이터 서버에 전달됨"));
	//1. 서버의 캐릭터에 커스터마이징 정보 저장 (혹시 모르니까)
	CharacterCustomizationData = CustomizingData;

	//2. 서버 및 모든 클라이언트에 커스터마이징 데이터 저장 및 적용
	Multicast_SetCustomizationData(CustomizingData);

	//3. 플레이어 스테이트에 커스터마이징 값 저장
	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{
		PS->SetCustomizationData(CustomizingData);
	}

	//4. 게이트 퇴장시를 위해 설정 완료되었음을 게임모드에 전파
	CheckPlayerCharacterIsReadyToGameMode();
}

void ABaseCharacter::Multicast_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData)
{
	LOG_Char_WARNING(TEXT("멀티캐스트로 전파 "));

	//1. 각 클라이언트 커스터마이징 정보 저장
	CharacterCustomizationData = CustomizingData;

	//2. 받은 커스터마이징 정보를 토대로 커스터마이징 적용
	ApplyCustomization(CharacterCustomizationData);
}

void ABaseCharacter::Server_UpdateCustomizationData_Implementation()
{
	LOG_Char_WARNING(TEXT("서버에서 전체에게 전파 준비"));

	Multicast_SetCustomizationData(CharacterCustomizationData);
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
	SetPartMesh(CustomHeadMesh, HeadSkeletalMesh);
	SetPartMesh(CustomHelmetMesh, HelmetSkeletalMesh);
	SetPartMesh(CustomGloveMesh, GloveSkeletalMesh);
	SetPartMesh(CustomJacketMesh_OwnerNoSee, JacketSkeletalMesh_OwnerNosee);
	SetPartMesh(CustomJacketMesh_OwnerSee, JacketSkeletalMesh);
	SetPartMesh(CustomPantsMesh, PantsSkeletalMesh);
	SetPartMesh(CustomBeltsMesh, BeltsSkeletalMesh);
	SetPartMesh(CustomArmorMesh, ArmorSkeletalMesh);
	SetPartMesh(CustomBootsMesh, BootsSkeletalMesh);



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
	SetPartMaterial(CustomHeadMesh, 0, HeadMat);
	SetPartMaterial(CustomHelmetMesh, 0, HelmetMat);
	SetPartMaterial(CustomGloveMesh, 0, GloveMat);
	SetPartMaterial(CustomJacketMesh_OwnerNoSee, 0, JacketMat);
	SetPartMaterial(CustomJacketMesh_OwnerSee, 0, JacketMat);
	SetPartMaterial(CustomPantsMesh, 0, PantsMat);
	SetPartMaterial(CustomBeltsMesh, 0, BeltsMat);
	SetPartMaterial(CustomArmorMesh, 1, ArmorMat);
	SetPartMaterial(CustomBootsMesh, 0, BootsMat);
	
	//플래그
	SetPartMaterial(CustomHelmetMesh, 1, FlagMat);
	SetPartMaterial(CustomArmorMesh, 0, FlagMat);

	SetCharacterPoseSynchronization();
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


void ABaseCharacter::OnKickHitBoxOverlap(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 서버에서만 처리
	if (!HasAuthority())
	{
		return;
	}
	const FVector Start = GetActorLocation() + GetActorForwardVector() * 50.f + FVector(0, 0, 50.f);
	const FVector End = Start; // 박스는 이동하지 않음

	const FVector BoxExtent = FVector(100.f, 100.f, 100.f); // 크기 조절 가능
	const FRotator Rotation = GetActorRotation();

	TArray<FHitResult> HitResults;

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetWorld(),
		Start,
		End,
		BoxExtent,
		Rotation,
		{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		false,
		{ this },
		EDrawDebugTrace::None,
		HitResults,
		true // ignore self
	);

	for (const FHitResult& Hit : HitResults)
	{
		ACharacter* TargetCharacter = Cast<ACharacter>(Hit.GetActor());
		if (!TargetCharacter || TargetCharacter == this) continue;
		if (TargetCharacter->IsA<ABaseBossMonsterCharacter>())
		{
			continue;
		}
		
		// 넉백 처리
		FVector KnockbackDir = GetActorForwardVector();
		KnockbackDir.Z = 0;
		KnockbackDir.Normalize();

		const float KnockbackStrength = 1000.f;
		const float UpwardStrength = 200.f;

		TargetCharacter->LaunchCharacter(KnockbackDir * KnockbackStrength + FVector(0, 0, UpwardStrength), true, true);
	}
}



void ABaseCharacter::StartKickHit()
{
	KickHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABaseCharacter::EndKickHit()
{
	KickHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	/*
	float baseBrightness = FMath::Lerp(-10.0f, 10.0f, Value); // 0~1 값을 0.5~2.0 범위로 매핑
	CustomPostProcessComponent->Settings.AutoExposureMinBrightness = baseBrightness - 0.1f;
	CustomPostProcessComponent->Settings.AutoExposureMaxBrightness = baseBrightness + 0.1f;
	CustomPostProcessComponent->Settings.AutoExposureBias = baseBrightness; // 유저 설정값 반영
	*/
	// UI 슬라이더 값: 0 ~ 100 → 0.0 ~ 1.0
	float Normalized = FMath::Clamp(Value, 0.0f, 1.0f);

	// 로그 스케일 매핑 (예: log10 스케일)
	float BrightnessValue = MinBrightness * FMath::Pow((MaxBrightness / MinBrightness), Normalized);

	CustomPostProcessComponent->Settings.AutoExposureBias = BrightnessValue;

	// 옵션: Min/MaxBrightness로 clamp
	CustomPostProcessComponent->Settings.AutoExposureMinBrightness = BrightnessValue; -0.01f;
	CustomPostProcessComponent->Settings.AutoExposureMaxBrightness = BrightnessValue; + 0.01f;
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
	// 전환 중일 때만 부드러운 이동 처리
	if (bIsTransitioning)
	{
		FVector CurrentLocation = SpringArm->GetComponentLocation();
		FVector TargetLocation;

		// 목표 위치 결정
		if (bIsAiming && IsValid(CurrentRifleMesh) && !bIsReloading)
		{
			TargetLocation = OverlaySkeletalMesh->GetSocketLocation(FName("Scope"));
			//TargetLocation = CurrentRifleMesh->GetSocketLocation(FName("Scope"));
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

			AGunBase* Gun = Cast<AGunBase>(GetToolbarInventoryComponent()->GetCurrentEquippedItem());
			if (IsValid(Gun))
			{
				if (Gun->HasScopeAttached())
				{
					SpringArm->AttachToComponent(OverlaySkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("RifleScope"));
					return;
				}
			}
			SpringArm->AttachToComponent(OverlaySkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Scope"));
			//SpringArm->AttachToComponent(CurrentRifleMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Scope"));
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
		if (EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")) 
			|| EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")) 
			|| EquipmentItem->ItemData.ItemType == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
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


	/*
	if (NameWidgetComponent == nullptr)
	{
		return;
	}

	// 누적 시간 계산
	
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
			// Pitch와 Roll 제거 → Yaw만 남김
			FRotator YawOnlyRotation = FRotator(0.f, LookAtRotation.Yaw, 0.f);

			// 적용
			NameWidgetComponent->SetWorldRotation(YawOnlyRotation);
		}
	}
	*/
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
		SoundLoudness*6,                   // float: 소리의 크기 (기본값은 1.0f, 감지 거리 등에 영향을 줌)
		SoundCauser,                 // AActor*: 소리의 주체 (보통 this)
		MaxSoundRange,                   // float: 소리를 들을 수 있는 최대 거리
		FName("Boss")                         // FName: 태그로 필터링 가능 (선택사항)
	);
}

void ABaseCharacter::Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity, float ZoomSensivity)
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
	//ReduceRecoil(0.3f);
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
		&ABaseCharacter::ResetShotCounter, 0.25f, false);
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
			AddControllerPitchInput(-AccumulatedRecoil.X * RecoilRecoveryAmount);
			AddControllerYawInput(-AccumulatedRecoil.Y * RecoilRecoveryAmount);
			AccumulatedRecoil = FVector2D::ZeroVector;
			TargetRecoil = FVector2D::ZeroVector;
			GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
		}
		else
		{
			AddControllerPitchInput(-RecoveryDelta.X * RecoilRecoveryAmount);
			AddControllerYawInput(-RecoveryDelta.Y * RecoilRecoveryAmount);
			AccumulatedRecoil -= RecoveryDelta * (1 / RecoilRecoveryAmount);
			TargetRecoil -= RecoveryDelta * (1 / RecoilRecoveryAmount);
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
		StaminaComponent->StopStaminaDrain();
		StaminaComponent->StartStaminaRecoverAfterDelay();
		return;
	}
	if (StaminaComponent->bIsExhausted) //만약 지친 상태라면 불가
	{
		return;
	}

	StopGunAutoFire();

	if (MyPlayerState->SprintInputMode == EInputMode::Hold)
	{
		if (Value < 0.5f) //입력이 떼지는 거면 어차피 뛰는 거 아님..
		{
			bIsSprinting = false;
			FootSoundModifier = MyPlayerState->RunningFootSoundModifier;
			SetDesiredGait(AlsGaitTags::Running);
			StaminaComponent->StopStaminaDrain();
			StaminaComponent->StartStaminaRecoverAfterDelay();
			return;
		}
		FootSoundModifier = MyPlayerState->SprintingFootSoundModifier;
		
		//달리기 시작하면서 스테미나 소모 시작
		StaminaComponent->StartStaminaDrain();
		StaminaComponent->StopStaminaRecovery();
		StaminaComponent->StopStaminaRecoverAfterDelay();
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
				StaminaComponent->StopStaminaDrain();
				StaminaComponent->StartStaminaRecoverAfterDelay();
			}
			else if (GetDesiredGait() == AlsGaitTags::Running)
			{
				//bIsSprinting = true;
				FootSoundModifier = MyPlayerState->SprintingFootSoundModifier;
				//SetDesiredAiming(false);
				Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				Camera->SetRelativeLocation(FVector::ZeroVector);
				Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
				StaminaComponent->StopStaminaRecovery();
				StaminaComponent->StopStaminaRecoverAfterDelay();
				StaminaComponent->StartStaminaDrain();
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
				StaminaComponent->StopStaminaRecovery();
				StaminaComponent->StopStaminaRecoverAfterDelay();
				StaminaComponent->StartStaminaDrain();
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
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}
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
	Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Camera->SetRelativeLocation(FVector::ZeroVector);
	Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
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
	bIsSprinting = false;
	SetDesiredAiming(true);
	SetDesiredGait(AlsGaitTags::Running);
	StaminaComponent->StopStaminaDrain();
	StaminaComponent->StartStaminaRecoverAfterDelay();
}

void ABaseCharacter::HandleStaminaThresholdReached()
{

}

void ABaseCharacter::PlayerIsSprint()
{
	float CurrentPlayerSpeed = GetPlayerMovementSpeed();
	if (FrontInput < 0.1f)
	{
		bIsSprinting = false;
		SetDesiredGait(AlsGaitTags::Running);
		//일단 회복 시키기는 해
		StaminaComponent->StartStaminaRecoverAfterDelay();
		LOG_Char_WARNING(TEXT("속도가 모자라서 스태미나 회복"));
		return;
	}
	bIsSprinting = true;
	SetDesiredAiming(false);
	SetDesiredGait(AlsGaitTags::Sprinting);
	Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Camera->SetRelativeLocation(FVector::ZeroVector);
	Camera->SetRelativeRotation(FRotator::ZeroRotator); // 필요 시 원래 회전 복구
	StaminaComponent->StopStaminaRecovery();
	StaminaComponent->StopStaminaRecoverAfterDelay();
	StaminaComponent->StartStaminaRecoverAfterDelayOnJump();

	StaminaComponent->StartStaminaDrain();


	//진짜 달리기 중인지 판단하는 로직이 필요

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
	/*
	bIsReloading = true;
	Server_PlayReload();
	*/
	AnimationComponent->PlayGunReloadMontage();
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
	Gun->DropMagazine();
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
		EmoteMode = false;
		CustomHeadMesh->SetOwnerNoSee(true);
		SwapHeadMaterialTransparent(true);
		SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
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
		SpringArm->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FirstPersonCamera"));
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
	if (CheckPlayerCurrentState() == EPlayerInGameStatus::Spectating)
	{
		return;
	}

	if (!CurrentFocusedActor)
	{
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
			InteractAfterPlayMontage(actor);
			//AnimationComponent->PlayInteractMontage(actor);
		}
	}
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
		
		if (!ToolbarInventoryComponent->CanAddItem(Item))
		{
			if (!Item->IsCollectible())
			{
				return;
			}
			
			if (!bBackpackMeshActive)
			{
				return;
			}
		}
		
		MontageToPlay = InteractMontageOnUnderObject;
	}
	else
	{
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
			//게이트 등 애니메이션 필요 없는 인터랙트 개체들을 위해...
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
			return;
		}
	}
	
	if (!IsValid(MontageToPlay))
	{
		return;
	}
	CurrentInteractMontage = MontageToPlay;
	bIsPlayingInteractionMontage = true;
	bIsPlayingAnimation = true;
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
	bIsPlayingAnimation = false;
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
	bIsPlayingAnimation = true;
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
		bIsMining = true;
		Client_SetMiningState(bIsMining);
	}
	else
	{
		//태그가 없으면 바로 실행
		EquippedItem->UseItem();
		return;
	}
	if (!IsValid(MontageToPlay))
	{
		return;
	}
	CurrentUseItemMontage = MontageToPlay;	
	bIsPlayingUseItemMontage = true;
	bIsPlayingAnimation = true;
	Server_PlayMontage(MontageToPlay, EAnimationType::UseItem);
}

void ABaseCharacter::Client_SetMiningState_Implementation(bool NewValue)
{
	bIsMining = NewValue;
}

void ABaseCharacter::UseItemAnimationNotified()
{
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
	bIsPlayingAnimation = false;
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

void ABaseCharacter::OnInteractionFromNotify()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (IsValid(InteractTargetActor))
	{
		IInteractableInterface::Execute_Interact(InteractTargetActor, PC);
	}
}

void ABaseCharacter::OnUseItemFromNotify()
{
	if (IsValid(CurrentUsingItem))
	{
		CurrentUsingItem->UseItem();
	}
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
	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f);
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
	// 1. 총을 들고 있는 상태인지 확인
	AItemBase* EquippedItem = ToolbarInventoryComponent->GetCurrentEquippedItem();
	if (!IsValid(EquippedItem))
	{
		WallClipAimOffsetPitch = FMath::FInterpTo(WallClipAimOffsetPitch, 0.0f, DeltaTime, 5.0f);
		return;
	}

	AEquipmentItemBase* EquipmentItem = Cast<AEquipmentItemBase>(EquippedItem);
	if (!IsValid(EquipmentItem))
	{
		WallClipAimOffsetPitch = FMath::FInterpTo(WallClipAimOffsetPitch, 0.0f, DeltaTime, 5.0f);
		return;
	}

	if (EquipmentItem->ItemData.ItemType != FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle"))
		&& EquipmentItem->ItemData.ItemType != FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun"))
		&& EquipmentItem->ItemData.ItemType != FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")))
	{
		WallClipAimOffsetPitch = FMath::FInterpTo(WallClipAimOffsetPitch, 0.0f, DeltaTime, 5.0f);
		return;
	}

	AGunBase* RifleItem = Cast<AGunBase>(EquippedItem);
	if (!IsValid(RifleItem))
	{
		WallClipAimOffsetPitch = FMath::FInterpTo(WallClipAimOffsetPitch, 0.0f, DeltaTime, 5.0f);
		return;
	}

	USkeletalMeshComponent* RifleMesh = RifleItem->GetSkeletalMeshComponent();
	if (!IsValid(RifleMesh))
	{
		WallClipAimOffsetPitch = FMath::FInterpTo(WallClipAimOffsetPitch, 0.0f, DeltaTime, 5.0f);
		return;
	}

	FVector MuzzleLoc = RifleMesh->GetSocketLocation("Muzzle");
	FTransform MuzzleTransform = RifleMesh->GetSocketTransform("Muzzle", RTS_World);

	// 머즐의 앞 방향과 Pitch 각도 얻기
	FVector MuzzleForward = MuzzleTransform.GetUnitAxis(EAxis::Z);
	FRotator MuzzleRot = MuzzleForward.Rotation();
	float MuzzlePitch = MuzzleRot.Pitch;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(RifleItem); // 총 자체도 무시

	// 더 안정적인 트레이스 설정
	static constexpr float GunWallTraceDistance = 1.0f;
	FVector TraceStart = MuzzleLoc - MuzzleForward * 150.0f;
	FVector TraceEnd = MuzzleLoc + MuzzleForward * GunWallTraceDistance;

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

	// 벽과의 거리 비율 계산 (데드존 추가)
	static constexpr float WallClipTriggerDistance = 100.0f;
	static constexpr float DeadZone = 10.0f; // 10cm 데드존

	float TargetWallRatio = 0.0f;
	if (bHit)
	{
		float Dist = (Hit.Location - MuzzleLoc).Size();
		if (Dist < WallClipTriggerDistance)
		{
			// 데드존 적용
			float AdjustedDist = FMath::Max(Dist - DeadZone, 0.0f);
			float AdjustedMaxDist = WallClipTriggerDistance - DeadZone;
			TargetWallRatio = 1.0f - (AdjustedDist / AdjustedMaxDist);
			TargetWallRatio = FMath::Clamp(TargetWallRatio, 0.0f, 1.0f);
		}
	}

	// 인스턴스 변수로 변경 (헤더 파일에 추가 필요)
	SmoothedWallRatio = FMath::FInterpTo(SmoothedWallRatio, TargetWallRatio, DeltaTime, 10.0f); // 보간 속도 감소

	// Pitch 보정값 계산
	float DirectionSign = MuzzlePitch >= 0 ? 1.0f : -1.0f;
	float TargetOffset = FMath::Lerp(0.0f, MaxWallClipPitch, SmoothedWallRatio) * DirectionSign;

	// 더 부드러운 보간
	WallClipAimOffsetPitch = FMath::FInterpTo(WallClipAimOffsetPitch, TargetOffset, DeltaTime, 4.0f);
	if (abs(WallClipAimOffsetPitch) > 10.0f)
	{
		bIsCloseToWall = true;
	}
	else
	{
		bIsCloseToWall = false;
	}
	// 애님 인스턴스에 전달
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
	CancelUseItem();
	CancelInteraction();
	StopReload();
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

float ABaseCharacter::TakeSpiritDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	LOG_Char_WARNING(TEXT("캐릭터가 정신력에 타격을 받음"));
	if (!HasAuthority())	
	{
		return 0;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return 0;
	}
	if (MyPlayerState->bInfiniteSpirit == true)
	{
		return 0;
	}
	float FinalDamage = CalculateTakeSpiritDamage(DamageAmount);
	float CurrentSpirit = MyPlayerState->GetSpirit();
	float MaxSpirit = MyPlayerState->MaxSpirit;
	float CalCulatedSpirit = FMath::Clamp(CurrentSpirit - FinalDamage, 0.0f, MaxSpirit);
	MyPlayerState->SetSpirit(CalCulatedSpirit);
	LOG_Char_WARNING(TEXT("Current Spirit : %f"), CalCulatedSpirit);
	if (CalCulatedSpirit <= MyPlayerState->PanicTriggerThreshold)
	{
		//: 정신력 낮음 처리
		EnterPanicState();
	}
	return DamageAmount;
}

void ABaseCharacter::TriggerSpiritTickDamage()
{
	GetWorld()->GetTimerManager().SetTimer(
		SpiritTickDamageHandle,
		this,
		&ABaseCharacter::TakeSpiritTickDamage,
		SpiritDamageTickInterval,
		true,           // 반복
		0.01f    // 처음 실행까지의 지연 시간
	);
}

void ABaseCharacter::TakeSpiritTickDamage()
{
	FDamageEvent DamageEvent;
	float DamageAmount = SpiritTickDamage;
	AController* InstigatorController = GetController(); // 또는 nullptr
	AActor* DamageCauser = this; // 또는 원하는 액터

	TakeSpiritDamage(DamageAmount, DamageEvent, GetController(), DamageCauser);
}


float ABaseCharacter::RestoreSpirit(float Amount)
{
	LOG_Char_WARNING(TEXT("캐릭터가 정신력을 회복함"));
	if (!HasAuthority())
	{
		return 0;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return 0;
	}
	float CurrentSpirit = MyPlayerState->GetSpirit();
	float MaxSpirit = MyPlayerState->MaxSpirit;
	float CalCulatedSpirit = FMath::Clamp(CurrentSpirit + Amount, 0.0f, MaxSpirit);
	MyPlayerState->SetSpirit(CalCulatedSpirit);
	LOG_Char_WARNING(TEXT("Current Spirit : %f"), CalCulatedSpirit);
	if (CalCulatedSpirit > MyPlayerState->PanicTriggerThreshold)
	{
		//: 정신력 높아짐 처리
		ExitPanicState();
	}
	return Amount;
}

float ABaseCharacter::CalculateTakeSpiritDamage(float DamageAmount)
{
	//TODO: 여기에다가 추가로 뭔가 장비나 방어력이 추가 되면 여기서 계산하고 넘겨도 됨.
	return DamageAmount;
}

void ABaseCharacter::EnterPanicState()
{
	//TODO: 서버에서의 처리
	LOG_Char_WARNING(TEXT("패닉 상태 진입"));

	//클라이언트에서의 처리
	Client_EnterPanicState();
}

void ABaseCharacter::ExitPanicState()
{
	Client_ExitPanicState();
}


void ABaseCharacter::Client_EnterPanicState_Implementation()
{
	StartPanicBehaviorLoop();
}

void ABaseCharacter::Client_ExitPanicState_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(PanicActionTimerHandle);

}

void ABaseCharacter::StartPanicBehaviorLoop()
{
	GetWorld()->GetTimerManager().SetTimer(
		PanicActionTimerHandle,
		this,
		&ABaseCharacter::PerformRandomPanicAction,
		RepeatRate,
		true,           // 반복
		InitialDelay    // 처음 실행까지의 지연 시간
	);
}

void ABaseCharacter::StopPanicBehaviorLoop()
{
	GetWorld()->GetTimerManager().ClearTimer(PanicActionTimerHandle);
}

void ABaseCharacter::PerformRandomPanicAction()
{
	LOG_Char_WARNING(TEXT("패닉 행동 실행"));

	TArray<TFunction<void()>> PanicActions;

	PanicActions.Add([this]() { PlayScreamSound_Local(); });
	PanicActions.Add([this]() { TriggerPanicVoice(PanicDuration); });
	PanicActions.Add([this]() { UseItemUnexpectedly(); });
	PanicActions.Add([this]() { PlaySighSoundForAll(); });
	PanicActions.Add([this]() { ForceSetMouseSensitivity(PanicSensitivity, PanicDuration); });
	PanicActions.Add([this]() { ForceInvertMouseTemporary(true, PanicDuration); });

	// 랜덤 선택해서 실행
	if (PanicActions.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, PanicActions.Num() - 1);
		PanicActions[RandomIndex]();
	}
	
  //TODO: 정신력 0 처리

}

void ABaseCharacter::PlayScreamSound_Local()
{
	if (IsLocallyControlled())
	{
		UGameplayStatics::PlaySoundAtLocation(this, ScreamSound, GetActorLocation());
	}
}

void ABaseCharacter::UseItemUnexpectedly()
{
	LOG_Char_WARNING(TEXT("갑자기 아이템 사용"));

	UseEquippedItem(1.0f);
	UseEquippedItem(0.0f);
}

void ABaseCharacter::PlaySighSoundForAll()
{
	if (IsLocallyControlled())
	{
		Server_PlaySighSound();
	}
}

void ABaseCharacter::Server_PlaySighSound_Implementation()
{
	if (HasAuthority()) // 서버에서만 멀티캐스트 호출
	{
		Multicast_PlaySighSound();
	}
}


void ABaseCharacter::Multicast_PlaySighSound_Implementation()
{
	if (SighSound)
	{
		UGameplayStatics::SpawnSoundAttached(
			SighSound,
			GetRootComponent(),         // 또는 GetMesh() 등 캐릭터에 붙일 컴포넌트
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			true                        // bStopWhenAttachedToDestroyed
		);

	}
}

void ABaseCharacter::ForceSetMouseSensitivity(float NewSensitivity, float Duration)
{
	LOG_Char_WARNING(TEXT("마우스 반전"));

	MouseSensitivityMultiplier = 10.0f;
	// 기존 타이머 제거 후 새 타이머 시작
	GetWorld()->GetTimerManager().ClearTimer(MouseSensitivityRestoreHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MouseSensitivityRestoreHandle,
		this,
		&ABaseCharacter::RestoreOriginalMouseSensitivity,
		Duration,
		false
	);
}

void ABaseCharacter::RestoreOriginalMouseSensitivity()
{
	MouseSensitivityMultiplier = 1.0f; // 초기화
}

void ABaseCharacter::ForceInvertMouse(bool bInvert)
{
	MouseInvertMultiplier = bInvert ? -1.0f : 1.0f;
}

void ABaseCharacter::ForceInvertMouseTemporary(bool bInvert, float Duration)
{
	// 반전 적용
	ForceInvertMouse(true);

	// 기존 타이머 제거 후 새로 시작
	GetWorld()->GetTimerManager().ClearTimer(MouseInvertResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MouseInvertResetTimerHandle,
		this,
		&ABaseCharacter::RestoreMouseInvert,
		Duration,
		false
	);
}

void ABaseCharacter::RestoreMouseInvert()
{
	ForceInvertMouse(false);
}

void ABaseCharacter::TriggerPanicVoice(float Duration)
{
	LOG_Char_WARNING(TEXT("보이스 변경"));

	EnterPanicVoice();
	GetWorld()->GetTimerManager().ClearTimer(PanicVoiceDurationHandle);
	GetWorld()->GetTimerManager().SetTimer(
		PanicVoiceDurationHandle,
		this,
		&ABaseCharacter::ExitPanicVoice,
		Duration,
		false
	);
}



void ABaseCharacter::Client_PlayHitSound_Implementation()
{
	if (IsLocallyControlled())
	{
		UGameplayStatics::PlaySoundAtLocation(this, OnHitSound, GetActorLocation());
	}
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
	if (MyPlayerState->bInfiniteHP == true) // 삭제하고 싶은데, 삭제하면 치트매니저에 문제가 생길 것 같은 느낌
	{
		return 0;
	}
	HealthComponent->TakeDamage(DamageAmount);
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

	TurnOffNameWidget();

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
	Client_TurnOffNameWidget();
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
	TurnOffNameWidget();
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
	Client_TurnOffNameWidget();
	TurnOffNameWidget();
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
	TurnOffNameWidget();
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
	FName Socketname = "Rifle";
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
	TSubclassOf<UAnimInstance> OverlayAnimationInstanceClass;
	if (bIsSpawnDrone)  // 태그에 컨트롤러 들 때 사용할 태그 추가해야됨...
	{
		OverlayAnimationInstanceClass = BinocularsAnimationClass;
		if (IsValid(OverlayAnimationInstanceClass))
		{
			GetMesh()->LinkAnimClassLayers(OverlayAnimationInstanceClass);
		}
		else
		{
			GetMesh()->LinkAnimClassLayers(DefaultAnimationClass);
		}
		return;
	}
	if (!ItemTag.IsValid())
	{
		OverlayAnimationInstanceClass = DefaultAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Rifle")))
	{
		OverlayAnimationInstanceClass = RifleAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.FlashLight")))
	{
		OverlayAnimationInstanceClass = TorchAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Pistol")))
	{
		OverlayAnimationInstanceClass = PistolTwoHandedAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Shotgun")))
	{
		OverlayAnimationInstanceClass = RifleAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Spawnable.Drone")))
	{
		OverlayAnimationInstanceClass = PistolOneHandedAnimationClass;
	}
	else if (ItemTag == FGameplayTag::RequestGameplayTag(TEXT("ItemType.Equipment.Tool.Pickaxe")))
	{
		OverlayAnimationInstanceClass = PickaxeAnimationClass;
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

	FItemDataRow Data = Item->ItemData;
	if (Data.bPlayCharacterAnimation == true)
	{
		AnimationComponent->PlayUseItemMontage(Item); //애니메이션 컴포넌트로 먼저 실행 후, 노티파이로 아이템 사용
	}
	else
	{
		Item->UseItem();
	}
	//UseItemAfterPlayMontage(Item);
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

	/*
	UpdateNameWidget(); // PlayerState가 복제될 때 UI 갱신
		
	if (IsLocallyControlled() && NameWidgetComponent)
	{
		NameWidgetComponent->SetVisibility(false, true);
	}
	*/
	if (APlayerState* PS = GetPlayerState())
	{
		if (NameComponent)
		{
			NameComponent->TryInitializeOnPlayerState(PS, IsLocallyControlled(), HasAuthority());
		}
	}

	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{
		SetCustomizationData(PS->GetCustomizationData());
		ApplyCustomization(CharacterCustomizationData);
	}
}

void ABaseCharacter::UpdateNameWidget()
{
	//1. 위젯에 내 이름 적용
	ApplyNameToWidget();

	if (IsLocallyControlled())
	{
		Server_UpdateNameWidget();
	}
}

void ABaseCharacter::Server_UpdateNameWidget_Implementation()
{
	ApplyNameToWidget();
}

void ABaseCharacter::ApplyNameToWidget()
{
	if (!IsValid(NameComponent))
	{
		return;
	}

	UUserWidget* Widget = NameComponent->GetWidget();
	if (!IsValid(Widget))
	{
		return;
	}

	UPlayerNameWidget* NameWidget = Cast<UPlayerNameWidget>(Widget);
	if (!IsValid(NameWidget))
	{
		return;
	}
	
	APlayerState* PS = GetPlayerState();
	if (!IsValid(PS))
	{
		return;
	}

	const FString Name = PS->GetPlayerName();
	NameWidget->SetPlayerName(Name);

	if (!HasAuthority())
	{
		return;
	}

	ABasePlayerState* BPS = Cast<ABasePlayerState>(PS);
	if (!IsValid(BPS))
	{
		BPS->SetPlayerInGameName(Name);
	}
}

void ABaseCharacter::TurnOffNameWidget()
{
	if (!IsValid(NameComponent))
	{
		return;
	}
	if (NameComponent)
	{
		NameComponent->HideNameWidget();
	}
	//NameWidgetComponent->SetVisibility(false, true);
}

void ABaseCharacter::Client_TurnOffNameWidget_Implementation()
{
	LOG_Char_WARNING(TEXT("관전시 위젯 해제"));

	AGameStateBase* GameState = GetWorld()->GetGameState<AGameStateBase>();
	if (!IsValid(GameState))
	{
		LOG_Char_WARNING(TEXT("GameState Is Invalid"));
		return;
	}

	if (GameState->PlayerArray.Num() <= 0)
	{
		return;
	}

	for (APlayerState* PS : GameState->PlayerArray)
	{
		ABasePlayerState* BasePS = Cast<ABasePlayerState>(PS);
		if (!IsValid(BasePS))
		{
			continue;
		}

		ABaseCharacter* Char = Cast<ABaseCharacter>(BasePS->GetPawn());
		if (!IsValid(Char))
		{
			continue;
		}

		Char->TurnOffNameWidget();
	}
}
