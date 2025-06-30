#include "Character/CinematicDummyCharacter.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"
#include "LastCanary.h"

ACinematicDummyCharacter::ACinematicDummyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsMoving = false;
	bReplicates = true;
	bAlwaysRelevant = true; // 원거리에서도 항상 복제되게
	SetReplicateMovement(true); // 추가로 반드시 활성화
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetIsReplicated(true);
	}
    // 더미는 AI도 입력도 안 받음
    AutoPossessAI = EAutoPossessAI::Disabled;
    AutoPossessPlayer = EAutoReceiveInput::Disabled;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false;

    CustomHeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomHeadMesh"));
    CustomHeadMesh->SetupAttachment(GetMesh());
    CustomHeadMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시

    CustomGloveMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomGloveMesh"));
    CustomGloveMesh->SetupAttachment(GetMesh());
    CustomGloveMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시

    CustomJacketMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomJacketMesh"));
    CustomJacketMesh->SetupAttachment(GetMesh());
    CustomJacketMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시

    CustomPantsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomPantsMesh"));
    CustomPantsMesh->SetupAttachment(GetMesh());
    CustomPantsMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시

    CustomBeltsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomBeltsMesh"));
    CustomBeltsMesh->SetupAttachment(GetMesh());
    CustomBeltsMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시

    CustomHelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomHelmetMesh"));
    CustomHelmetMesh->SetupAttachment(GetMesh());
    CustomHelmetMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시

    CustomArmorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomArmorMesh"));
    CustomArmorMesh->SetupAttachment(GetMesh());
    CustomArmorMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시

    CustomBootsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomBootsMesh"));
    CustomBootsMesh->SetupAttachment(GetMesh());
    CustomBootsMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시

    ////* 가방 메시 *////
    BackpackMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackpackMesh"));
    BackpackMesh->SetupAttachment(GetMesh());
    BackpackMesh->SetLeaderPoseComponent(GetMesh()); // GetMesh()는 전체 메시
}

void ACinematicDummyCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ACinematicDummyCharacter::ApplyAppearance(FCharacterCustomizationData Data)
{
	ApplyCustomization(CharacterMeshMap, Data);
	Multicast_ApplyAppearance(Data);
}

void ACinematicDummyCharacter::Multicast_ApplyAppearance_Implementation(FCharacterCustomizationData Data)
{
	ApplyCustomization(CharacterMeshMap, Data);

}

void ACinematicDummyCharacter::MoveToLocation(const FVector& TargetLocation, float Duration)
{
    if (Duration <= 0.f)
    {
        return;
    }

    MoveStart = GetActorLocation();
    MoveEnd = TargetLocation;
    MoveDuration = Duration;
    MoveElapsed = 0.f;
    bIsMoving = true;
}

void ACinematicDummyCharacter::PlayAnimMontageOnce(UAnimMontage* Montage)
{
    if (Montage)
    {
        PlayAnimMontage(Montage);
    }
}

void ACinematicDummyCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 권한이 있는 서버에서만 위치를 변경하도록 체크
	if (bIsMoving && HasAuthority())
	{
		TickMove(DeltaSeconds);
	}
}

void ACinematicDummyCharacter::TickMove(float DeltaTime)
{
    MoveElapsed += DeltaTime;
    float Alpha = FMath::Clamp(MoveElapsed / MoveDuration, 0.f, 1.f);
    FVector NewLocation = FMath::Lerp(MoveStart, MoveEnd, Alpha);
    SetActorLocation(NewLocation);

    if (Alpha >= 1.f)
    {
        bIsMoving = false;
    }
}


void ACinematicDummyCharacter::ApplyCustomization(const UCustomizationMeshMap* CharacterMeshData, FCharacterCustomizationData CustomizationData)
{
	if (!CharacterMeshData || !CharacterMeshData->IsValidLowLevel())
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
	USkeletalMesh* BodySkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultBodyMeshes, BodyId);
	USkeletalMesh* HeadSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultHeadMeshes, HeadId);
	USkeletalMesh* HelmetSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->HelmetMeshes, HelmetId);
	USkeletalMesh* GloveSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->GloveMeshes, GloveId);
	USkeletalMesh* JacketSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->JacketMeshes, JacketId);
	USkeletalMesh* PantsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->PantsMeshes, PantsId);
	USkeletalMesh* BeltsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->BeltsMeshes, BeltsId);
	USkeletalMesh* ArmorSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->ArmorMeshes, ArmorId);
	USkeletalMesh* BootsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->BootsMeshes, BootsId);

	SetPartMesh(GetMesh(), BodySkeletalMesh);
	SetPartMesh(CustomHeadMesh, HeadSkeletalMesh);
	SetPartMesh(CustomHelmetMesh, HelmetSkeletalMesh);
	SetPartMesh(CustomGloveMesh, GloveSkeletalMesh);
	SetPartMesh(CustomJacketMesh, JacketSkeletalMesh);
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
	UMaterialInterface* BodyMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->DefaultBodyMaterials, BodyMatId);
	UMaterialInterface* HeadMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->DefaultBodyMaterials, HeadMatId);
	UMaterialInterface* HelmetMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->HelmetMaterials, HelmetMatId);
	UMaterialInterface* GloveMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->GloveMaterials, GloveMatId);
	UMaterialInterface* JacketMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->JacketMaterials, JacketMatId);
	UMaterialInterface* PantsMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->PantsMaterials, PantsMatId);
	UMaterialInterface* BeltsMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->BeltsMaterials, BeltsMatId);
	UMaterialInterface* ArmorMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->ArmorMaterials, ArmorMatId);
	UMaterialInterface* BootsMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->BootsMaterials, BootsMatId);
	UMaterialInterface* FlagMat = CharacterMeshData->GetMaterialByID(CharacterMeshData->FlagMaterials, FlagMatId);

	// 머티리얼 적용 함수 호출 (보통 0번 슬롯만 적용한다고 가정)
	SetPartMaterial(GetMesh(), 0, BodyMat);
	SetPartMaterial(CustomHeadMesh, 0, HeadMat);
	SetPartMaterial(CustomHelmetMesh, 0, HelmetMat);
	SetPartMaterial(CustomGloveMesh, 0, GloveMat);
	SetPartMaterial(CustomJacketMesh, 0, JacketMat);
	SetPartMaterial(CustomPantsMesh, 0, PantsMat);
	SetPartMaterial(CustomBeltsMesh, 0, BeltsMat);
	SetPartMaterial(CustomArmorMesh, 1, ArmorMat);
	SetPartMaterial(CustomBootsMesh, 0, BootsMat);

	//플래그
	SetPartMaterial(CustomHelmetMesh, 1, FlagMat);
	SetPartMaterial(CustomArmorMesh, 0, FlagMat);

	SetCharacterPoseSynchronization();
}

void ACinematicDummyCharacter::SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh)
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

void ACinematicDummyCharacter::SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material)
{
	if (!Component || !Material) return;

	// 메시가 존재하고, 표시 상태일 경우에만 적용
	if (Component && Component->IsRegistered() && Component->IsVisible() && Component->SkeletalMesh)
	{
		Component->SetMaterial(MaterialIndex, Material);
	}
}

void ACinematicDummyCharacter::SetCharacterPoseSynchronization()
{
	CustomHeadMesh->SetLeaderPoseComponent(GetMesh());
	CustomGloveMesh->SetLeaderPoseComponent(GetMesh());
	CustomJacketMesh->SetLeaderPoseComponent(GetMesh());
	CustomPantsMesh->SetLeaderPoseComponent(GetMesh());
	CustomBeltsMesh->SetLeaderPoseComponent(GetMesh());
	CustomHelmetMesh->SetLeaderPoseComponent(GetMesh());
	CustomArmorMesh->SetLeaderPoseComponent(GetMesh());
	CustomBootsMesh->SetLeaderPoseComponent(GetMesh());
	BackpackMesh->SetLeaderPoseComponent(GetMesh());
}