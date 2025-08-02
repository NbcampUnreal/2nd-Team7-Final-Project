#include "Character/Component/CharacterCustomizationComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/CustomizationMeshMap.h"

#include "LastCanary.h"

USkeletalMeshComponent* UCharacterCustomizationComponent::CharacterMesh()
{
	return CachedCharacter->GetMesh();
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetHeadMesh()
{
	return CachedCharacter->CustomHeadMesh;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetGloveMesh()
{
	return CachedCharacter->CustomGloveMesh;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetJacketMesh_OwnerNoSee()
{
	return CachedCharacter->CustomJacketMesh_OwnerNoSee;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetJacketMesh_OwnerSee()
{
	return CachedCharacter->CustomJacketMesh_OwnerSee;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetPantsMesh()
{
	return CachedCharacter->CustomPantsMesh;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetBeltsMesh()
{
	return CachedCharacter->CustomBeltsMesh;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetHelmetMesh()
{
	return CachedCharacter->CustomHelmetMesh;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetArmorMesh()
{
	return CachedCharacter->CustomArmorMesh;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetBootsMesh()
{
	return CachedCharacter->CustomBootsMesh;
}

USkeletalMeshComponent* UCharacterCustomizationComponent::GetBackpackMesh()
{
	return CachedCharacter->BackpackMesh;
}

void UCharacterCustomizationComponent::SetCharacterPoseSynchronization()
{
	GetHeadMesh()->SetLeaderPoseComponent(CharacterMesh());
	GetGloveMesh()->SetLeaderPoseComponent(CharacterMesh());
	GetJacketMesh_OwnerNoSee()->SetLeaderPoseComponent(CharacterMesh());
	GetJacketMesh_OwnerSee()->SetLeaderPoseComponent(CharacterMesh());
	GetPantsMesh()->SetLeaderPoseComponent(CharacterMesh());
	GetBeltsMesh()->SetLeaderPoseComponent(CharacterMesh());
	GetHelmetMesh()->SetLeaderPoseComponent(CharacterMesh());
	GetArmorMesh()->SetLeaderPoseComponent(CharacterMesh());
	GetBootsMesh()->SetLeaderPoseComponent(CharacterMesh());
	GetBackpackMesh()->SetLeaderPoseComponent(CharacterMesh());
}

void UCharacterCustomizationComponent::ApplyCustomization(const FCharacterCustomizationData CustomizationData)
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

	SetPartMesh(CharacterMesh(), BodySkeletalMesh);
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
	SetPartMaterial(CharacterMesh(), 0, BodyMat);
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

void UCharacterCustomizationComponent::SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh)
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

void UCharacterCustomizationComponent::SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material)
{
	if (!Component || !Material) return;

	// 메시가 존재하고, 표시 상태일 경우에만 적용
	if (Component && Component->IsRegistered() && Component->IsVisible() && Component->SkeletalMesh)
	{
		Component->SetMaterial(MaterialIndex, Material);
	}
}

FCharacterCustomizationData UCharacterCustomizationComponent::GetCustomizationData()
{
	return CharacterCustomizationData;
}

void UCharacterCustomizationComponent::SetCustomizationData(const FCharacterCustomizationData& CustomizingData)
{
	CharacterCustomizationData = CustomizingData;
}

void UCharacterCustomizationComponent::Server_UpdateCustomizationData_Implementation()
{
	LOG_Char_WARNING(TEXT("서버에서 전체에게 전파 준비"));

	Multicast_SetCustomizationData(CharacterCustomizationData);
}

void UCharacterCustomizationComponent::Server_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData)
{
	LOG_Char_WARNING(TEXT("캐릭터 커스터마이징 데이터 서버에 전달됨"));
	//1. 서버의 캐릭터에 커스터마이징 정보 저장 (혹시 모르니까)
	CharacterCustomizationData = CustomizingData;

	//2. 서버 및 모든 클라이언트에 커스터마이징 데이터 저장 및 적용
	Multicast_SetCustomizationData(CustomizingData);

	//3. 플레이어 스테이트에 커스터마이징 값 저장
	/*
	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{
		PS->SetCustomizationData(CustomizingData);
	}

	//4. 게이트 퇴장시를 위해 설정 완료되었음을 게임모드에 전파
	CheckPlayerCharacterIsReadyToGameMode();
	*/
}

void UCharacterCustomizationComponent::Multicast_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData)
{
	LOG_Char_WARNING(TEXT("멀티캐스트로 전파 "));

	//1. 각 클라이언트 커스터마이징 정보 저장
	CharacterCustomizationData = CustomizingData;

	//2. 받은 커스터마이징 정보를 토대로 커스터마이징 적용
	ApplyCustomization(CharacterCustomizationData);
}
