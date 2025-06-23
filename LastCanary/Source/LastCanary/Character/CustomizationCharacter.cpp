// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CustomizationCharacter.h"
#include "Character/CustomizationMeshMap.h"
#include "LastCanary.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"

// Sets default values
ACustomizationCharacter::ACustomizationCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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
}

// Called when the game starts or when spawned
void ACustomizationCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyCustomization(CharacterMeshMap);
}

void ACustomizationCharacter::ApplyCustomization(const UCustomizationMeshMap* CharacterMeshData)
{
	if (!CharacterMeshData || !CharacterMeshData->IsValidLowLevel())
	{
		LOG_Char_WARNING(TEXT("캐릭터 메시 데이터 invalid"));
		return;
	}
	FCharacterCustomizationData CustomizationData = ULCLocalPlayerSaveGame::LoadCustomizationData(GetWorld());
	int BodyId = CustomizationData.DefaultBodyID;
	int HeadId = CustomizationData.DefaultBodyID;
	int HelmetId = CustomizationData.HelmetID;
	int GloveId = CustomizationData.GloveID;
	int JacketId = CustomizationData.JacketID;
	int PantsId = CustomizationData.PantsID;
	int BeltsId = CustomizationData.BeltsID;
	int ArmorId = CustomizationData.ArmorID;
	int BootsId = CustomizationData.BootsID;
	LOG_Char_WARNING(TEXT("Customization Info - BodyId: %d, HeadId: %d, HelmetId: %d, GloveId: %d, JacketId: %d, PantsId: %d, BeltsId: %d, ArmorId: %d, BootsId: %d"),
		BodyId, HeadId, HelmetId, GloveId, JacketId, PantsId, BeltsId, ArmorId, BootsId);

	// Body
	USkeletalMesh* BodySkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultBodyMeshes, BodyId);
	SetPartMesh(GetMesh(), BodySkeletalMesh);

	USkeletalMesh* HeadSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultHeadMeshes, HeadId);
	SetPartMesh(CustomHeadMesh, HeadSkeletalMesh);

	USkeletalMesh* HelmetSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->HelmetMeshes, HelmetId);
	SetPartMesh(CustomHelmetMesh, HelmetSkeletalMesh);

	USkeletalMesh* GloveSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->GloveMeshes, GloveId);
	SetPartMesh(CustomGloveMesh, GloveSkeletalMesh);

	USkeletalMesh* JacketSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->JacketMeshes, JacketId);
	SetPartMesh(CustomJacketMesh, JacketSkeletalMesh);

	USkeletalMesh* PantsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->PantsMeshes, PantsId);
	SetPartMesh(CustomPantsMesh, PantsSkeletalMesh);

	USkeletalMesh* BeltsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->BeltsMeshes, BeltsId);
	SetPartMesh(CustomBeltsMesh, BeltsSkeletalMesh);

	USkeletalMesh* ArmorSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->ArmorMeshes, ArmorId);
	SetPartMesh(CustomArmorMesh, ArmorSkeletalMesh);

	USkeletalMesh* BootsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->BootsMeshes, BootsId);
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

	// 머티리얼 적용 함수 호출 (보통 0번 슬롯만 적용한다고 가정)
	SetPartMaterial(GetMesh(), 0, BodyMat);
	SetPartMaterial(CustomHeadMesh, 0, HeadMat);
	SetPartMaterial(CustomHelmetMesh, 0, HelmetMat);
	SetPartMaterial(CustomGloveMesh, 0, GloveMat);
	SetPartMaterial(CustomJacketMesh, 0, JacketMat);
	SetPartMaterial(CustomPantsMesh, 0, PantsMat);
	SetPartMaterial(CustomBeltsMesh, 0, BeltsMat);
	SetPartMaterial(CustomArmorMesh, 0, ArmorMat);
	SetPartMaterial(CustomBootsMesh, 0, BootsMat);

}

void ACustomizationCharacter::SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh)
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

void ACustomizationCharacter::SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material)
{
	if (!Component || !Material) return;

	// 메시가 존재하고, 표시 상태일 경우에만 적용
	if (Component && Component->IsRegistered() && Component->IsVisible() && Component->SkeletalMesh)
	{
		Component->SetMaterial(MaterialIndex, Material);
	}
}
