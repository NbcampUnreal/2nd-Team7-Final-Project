#include "UI/UIElement/CharacterCustomizationWidget.h"
#include "UI/Manager/LCUIManager.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Framework/GameInstance/LCOptionManager.h"
#include "Character/CustomizationMeshMap.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Character/CustomizationCharacter.h"

#include "LastCanary.h"


void UCharacterCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ViewNextFullBodyButton)
	{
		ViewNextFullBodyButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectFullBody);
	}

	if (ViewNextGloveButton)
	{
		ViewNextGloveButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectGlove);
	}

	if (ViewNextJacketButton)
	{
		ViewNextJacketButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectJacket);
	}

	if (ViewNextPantsButton)
	{
		ViewNextPantsButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectPants);
	}

	if (ViewNextBeltsButton)
	{
		ViewNextBeltsButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectBelts);
	}

	if (ViewNextHelmetsButton)
	{
		ViewNextHelmetsButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectHelmets);
	}

	if (ViewNextArmorButton)
	{
		ViewNextArmorButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectArmor);
	}

	if (ViewNextBootsButton)
	{
		ViewNextBootsButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectBoots);
	}

	if (ViewNextArmorMaterialButton)
	{
		ViewNextArmorMaterialButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::OnSelectArmorMaterial);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::CloseCustomizationWidget);
	}

	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::ApplySetting);
	}
	InitWidget();
}

void UCharacterCustomizationWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (ViewNextFullBodyButton)
	{
		ViewNextFullBodyButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectFullBody);
	}

	if (ViewNextGloveButton)
	{
		ViewNextGloveButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectGlove);
	}

	if (ViewNextJacketButton)
	{
		ViewNextJacketButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectJacket);
	}

	if (ViewNextPantsButton)
	{
		ViewNextPantsButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectPants);
	}

	if (ViewNextBeltsButton)
	{
		ViewNextBeltsButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectBelts);
	}

	if (ViewNextHelmetsButton)
	{
		ViewNextHelmetsButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectHelmets);
	}

	if (ViewNextArmorButton)
	{
		ViewNextArmorButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectArmor);
	}

	if (ViewNextBootsButton)
	{
		ViewNextBootsButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectBoots);
	}

	if (ViewNextArmorMaterialButton)
	{
		ViewNextArmorMaterialButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::OnSelectArmorMaterial);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::CloseCustomizationWidget);
	}

	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::ApplySetting);
	}
}

void UCharacterCustomizationWidget::InitWidget()
{
	//키, 버튼 바인딩
	UpdateTargetCharacter();
}

void UCharacterCustomizationWidget::CloseCustomizationWidget()
{
	RemoveFromParent();
}

void UCharacterCustomizationWidget::UpdateTargetCharacter()
{
	//캐릭터 할당해주기
		// 캐릭터 찾기 (한 번만 해도 됨)
	if (!TargetCharacter)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomizationCharacter::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			if (Actor->ActorHasTag("PreviewCharacter"))
			{
				TargetCharacter = Cast<ACustomizationCharacter>(Actor);
				break;
			}
		}
	}

	// 데이터 있으면 적용
	if (TargetCharacter && CharacterMeshData)
	{
		TargetCharacter->ApplyCustomization(CharacterMeshData);
	}


	if (!IsValid(TargetCharacter)) return;
	OnLoadCustomization();
}

void UCharacterCustomizationWidget::OnSelectFullBody()
{
	if (!IsValid(TargetCharacter)) return;
	int32 Maxindex = CharacterMeshData->DefaultBodyMeshes.Num() - 1;
	CurrentFullBodyIndex++;
	if (CurrentFullBodyIndex > Maxindex)
	{
		CurrentFullBodyIndex = 0;
	}
	CurrentSelection.DefaultBodyID = CurrentFullBodyIndex;
	LOG_Char_WARNING(TEXT("캐릭터 의상 풀바디 : CurrentFullBodyIndex : %f"), CurrentFullBodyIndex);

	// 즉시 적용
	UpdateCustomization();
}

void UCharacterCustomizationWidget::OnSelectGlove()
{
	if (!IsValid(TargetCharacter)) return;
	if (!CharacterMeshData) return;
	int32 Maxindex = CharacterMeshData->GloveMeshes.Num() - 1;
	CurrentGloveIndex++;
	if (CurrentGloveIndex > Maxindex)
	{
		CurrentGloveIndex = 0;
	}
	CurrentSelection.GloveID = CurrentGloveIndex;

	// 즉시 적용
	UpdateCustomization();
}
void UCharacterCustomizationWidget::OnSelectJacket()
{
	if (!IsValid(TargetCharacter)) return;
	if (!CharacterMeshData) return;
	int32 Maxindex = CharacterMeshData->JacketMeshes.Num() - 1;
	CurrentJacketIndex++;
	if (CurrentJacketIndex > Maxindex)
	{
		CurrentJacketIndex = 0;
	}
	CurrentSelection.JacketID = CurrentJacketIndex;

	// 즉시 적용
	UpdateCustomization();
}


void UCharacterCustomizationWidget::OnSelectPants()
{
	if (!IsValid(TargetCharacter)) return;
	if (!CharacterMeshData) return;
	int32 Maxindex = CharacterMeshData->PantsMeshes.Num() - 1;
	CurrentPantsIndex++;
	if (CurrentPantsIndex > Maxindex)
	{
		CurrentPantsIndex = 0;
	}
	CurrentSelection.PantsID = CurrentPantsIndex;

	// 즉시 적용
	UpdateCustomization();
}
void UCharacterCustomizationWidget::OnSelectBelts()
{
	if (!IsValid(TargetCharacter)) return;
	if (!CharacterMeshData) return;
	int32 Maxindex = CharacterMeshData->BeltsMeshes.Num() - 1;
	CurrentBeltsIndex++;
	if (CurrentBeltsIndex > Maxindex)
	{
		CurrentBeltsIndex = 0;
	}
	CurrentSelection.BeltsID = CurrentBeltsIndex;

	// 즉시 적용
	UpdateCustomization();
}

void UCharacterCustomizationWidget::OnSelectHelmets()
{
	if (!IsValid(TargetCharacter)) return;
	if (!CharacterMeshData) return;
	int32 Maxindex = CharacterMeshData->HelmetMeshes.Num() - 1;
	CurrentHelmetsIndex++;
	if (CurrentHelmetsIndex > Maxindex)
	{
		CurrentHelmetsIndex = 0;
	}
	CurrentSelection.HelmetID = CurrentHelmetsIndex;

	// 즉시 적용
	UpdateCustomization();
}

void UCharacterCustomizationWidget::OnSelectArmor()
{
	if (!IsValid(TargetCharacter)) return;
	if (!CharacterMeshData) return;
	int32 Maxindex = CharacterMeshData->ArmorMeshes.Num() - 1;
	CurrentArmorIndex++;
	if (CurrentArmorIndex > Maxindex)
	{
		CurrentArmorIndex = 0;
	}
	CurrentSelection.ArmorID = CurrentArmorIndex;

	// 즉시 적용
	UpdateCustomization();
}

void UCharacterCustomizationWidget::OnSelectBoots()
{
	if (!IsValid(TargetCharacter)) return;
	if (!CharacterMeshData) return;
	int32 Maxindex = CharacterMeshData->BootsMeshes.Num() - 1;
	CurrentBootsIndex++;
	if (CurrentBootsIndex > Maxindex)
	{
		CurrentBootsIndex = 0;
	}
	CurrentSelection.BootsID = CurrentBootsIndex;

	// 즉시 적용
	UpdateCustomization();
}

void UCharacterCustomizationWidget::OnSelectArmorMaterial()
{
	if (!IsValid(TargetCharacter)) return;
	if (!CharacterMeshData) return;
	int32 Maxindex = CharacterMeshData->ArmorMaterials.Num() - 1;
	CurrentArmorMaterialIndex++;
	if (CurrentArmorMaterialIndex > Maxindex)
	{
		CurrentArmorMaterialIndex = 0;
	}
	CurrentSelection.ArmorMaterialID = CurrentArmorMaterialIndex;

	// 즉시 적용
	UpdateCustomization();
}

void UCharacterCustomizationWidget::OnResetCustomization()
{
	CurrentSelection = FCharacterCustomizationData();
}

void UCharacterCustomizationWidget::OnConfirmCustomization()
{
	if (UWorld* World = GetWorld())
	{
		ULCLocalPlayerSaveGame::SaveCustomizationData(GetWorld(), CurrentSelection);
	}
}

void UCharacterCustomizationWidget::OnLoadCustomization()
{
	if (UWorld* World = GetWorld())
	{
		FCharacterCustomizationData SavedCustomizationData = ULCLocalPlayerSaveGame::LoadCustomizationData(World);
		//여기서 캐릭터에 적용
		InitCustomizationIndex(SavedCustomizationData);
	}
}

void UCharacterCustomizationWidget::SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh)
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

void UCharacterCustomizationWidget::SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material)
{
	if (!Component || !Material) return;

	// 메시가 존재하고, 표시 상태일 경우에만 적용
	if (Component && Component->IsRegistered() && Component->IsVisible() && Component->SkeletalMesh)
	{
		Component->SetMaterial(MaterialIndex, Material);
	}
}


void UCharacterCustomizationWidget::InitCustomizationIndex(FCharacterCustomizationData SavedCustomizationData)
{
	if (!CharacterMeshData || !CharacterMeshData->IsValidLowLevel())
	{
		LOG_Char_WARNING(TEXT("캐릭터 메시 데이터 invalid"));
		return;
	}

	// 메시 인덱스 초기화
	CurrentFullBodyIndex = SavedCustomizationData.DefaultBodyID;
	CurrentGloveIndex = SavedCustomizationData.GloveID;
	CurrentJacketIndex = SavedCustomizationData.JacketID;
	CurrentPantsIndex = SavedCustomizationData.PantsID;
	CurrentBeltsIndex = SavedCustomizationData.BeltsID;
	CurrentHelmetsIndex = SavedCustomizationData.HelmetID;
	CurrentArmorIndex = SavedCustomizationData.ArmorID;
	CurrentBootsIndex = SavedCustomizationData.BootsID;

	// 머티리얼 인덱스 초기화
	CurrentFullBodyMaterialIndex = SavedCustomizationData.DefaultBodyMaterialID;
	CurrentGloveMaterialIndex = SavedCustomizationData.GloveMaterialID;
	CurrentJacketMaterialIndex = SavedCustomizationData.JacketMaterialID;
	CurrentPantsMaterialIndex = SavedCustomizationData.PantsMaterialID;
	CurrentBeltsMaterialIndex = SavedCustomizationData.BeltsMaterialID;
	CurrentHelmetMaterialIndex = SavedCustomizationData.HelmetMaterialID;
	CurrentArmorMaterialIndex = SavedCustomizationData.ArmorMaterialID;
	CurrentBootsMaterialIndex = SavedCustomizationData.BootsMaterialID;

	LOG_Char_WARNING(TEXT("Current Customization Indexes - FullBody: %d, Glove: %d, Jacket: %d, Pants: %d, Belts: %d, Helmet: %d, Armor: %d, Boots: %d | Material - Body: %d, Glove: %d, Jacket: %d, Pants: %d, Belts: %d, Helmet: %d, Armor: %d, Boots: %d"),
		CurrentFullBodyIndex, CurrentGloveIndex, CurrentJacketIndex, CurrentPantsIndex, CurrentBeltsIndex,
		CurrentHelmetsIndex, CurrentArmorIndex, CurrentBootsIndex,
		CurrentFullBodyMaterialIndex, CurrentGloveMaterialIndex, CurrentJacketMaterialIndex, CurrentPantsMaterialIndex,
		CurrentBeltsMaterialIndex, CurrentHelmetMaterialIndex, CurrentArmorMaterialIndex, CurrentBootsMaterialIndex);

	UpdateCustomization();
}

void UCharacterCustomizationWidget::UpdateCustomization()
{
	if (!CharacterMeshData->IsValidLowLevel())
	{
		LOG_Char_WARNING(TEXT("캐릭터 메시 데이터 invalid"));
		return;
	}

	// Body
	USkeletalMesh* BodySkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultBodyMeshes, CurrentFullBodyIndex);
	SetPartMesh(TargetCharacter->GetMesh(), BodySkeletalMesh);
	SetPartMaterial(TargetCharacter->GetMesh(), 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->DefaultBodyMaterials, CurrentFullBodyMaterialIndex));

	USkeletalMesh* HeadSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultHeadMeshes, CurrentFullBodyIndex);
	SetPartMesh(TargetCharacter->CustomHeadMesh, HeadSkeletalMesh);
	SetPartMaterial(TargetCharacter->CustomHeadMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->DefaultHeadMaterials, CurrentFullBodyMaterialIndex));

	USkeletalMesh* HelmetSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->HelmetMeshes, CurrentHelmetsIndex);
	SetPartMesh(TargetCharacter->CustomHelmetMesh, HelmetSkeletalMesh);
	SetPartMaterial(TargetCharacter->CustomHelmetMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->HelmetMaterials, CurrentHelmetMaterialIndex));

	USkeletalMesh* GloveSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->GloveMeshes, CurrentGloveIndex);
	SetPartMesh(TargetCharacter->CustomGloveMesh, GloveSkeletalMesh);
	SetPartMaterial(TargetCharacter->CustomGloveMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->GloveMaterials, CurrentGloveMaterialIndex));

	USkeletalMesh* JacketSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->JacketMeshes, CurrentJacketIndex);
	SetPartMesh(TargetCharacter->CustomJacketMesh, JacketSkeletalMesh);
	SetPartMaterial(TargetCharacter->CustomJacketMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->JacketMaterials, CurrentJacketMaterialIndex));

	USkeletalMesh* PantsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->PantsMeshes, CurrentPantsIndex);
	SetPartMesh(TargetCharacter->CustomPantsMesh, PantsSkeletalMesh);
	SetPartMaterial(TargetCharacter->CustomPantsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->PantsMaterials, CurrentPantsMaterialIndex));

	USkeletalMesh* BeltsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->BeltsMeshes, CurrentBeltsIndex);
	SetPartMesh(TargetCharacter->CustomBeltsMesh, BeltsSkeletalMesh);
	SetPartMaterial(TargetCharacter->CustomBeltsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->BeltsMaterials, CurrentBeltsMaterialIndex));

	USkeletalMesh* ArmorSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->ArmorMeshes, CurrentArmorIndex);
	SetPartMesh(TargetCharacter->CustomArmorMesh, ArmorSkeletalMesh);
	SetPartMaterial(TargetCharacter->CustomArmorMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->ArmorMaterials, CurrentArmorMaterialIndex));

	USkeletalMesh* BootsSkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->BootsMeshes, CurrentBootsIndex);
	SetPartMesh(TargetCharacter->CustomBootsMesh, BootsSkeletalMesh);
	SetPartMaterial(TargetCharacter->CustomBootsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->BootsMaterials, CurrentBootsMaterialIndex));
}

void UCharacterCustomizationWidget::ApplySetting()
{
	LOG_Char_WARNING(TEXT("세팅 저장"));
	CurrentSelection.DefaultBodyID = CurrentFullBodyIndex;
	CurrentSelection.GloveID = CurrentGloveIndex;
	CurrentSelection.JacketID = CurrentJacketIndex;
	CurrentSelection.PantsID = CurrentPantsIndex;
	CurrentSelection.BeltsID = CurrentBeltsIndex;
	CurrentSelection.HelmetID = CurrentHelmetsIndex;
	CurrentSelection.ArmorID = CurrentArmorIndex;
	CurrentSelection.BootsID = CurrentBootsIndex;
	CurrentSelection.DefaultBodyMaterialID = CurrentFullBodyMaterialIndex;
	CurrentSelection.GloveMaterialID = CurrentGloveMaterialIndex;
	CurrentSelection.JacketMaterialID = CurrentJacketMaterialIndex;
	CurrentSelection.PantsMaterialID = CurrentPantsMaterialIndex;
	CurrentSelection.BeltsMaterialID = CurrentBeltsMaterialIndex;
	CurrentSelection.HelmetMaterialID = CurrentHelmetMaterialIndex;
	CurrentSelection.ArmorMaterialID = CurrentArmorMaterialIndex;
	CurrentSelection.BootsMaterialID = CurrentBootsMaterialIndex;

	ULCLocalPlayerSaveGame::SaveCustomizationData(GetWorld(), CurrentSelection);
}