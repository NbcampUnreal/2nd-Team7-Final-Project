#include "UI/UIElement/CharacterCustomizationWidget.h"
#include "UI/Manager/LCUIManager.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#include "Framework/GameInstance/LCOptionManager.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Character/CustomizationMeshMap.h"
#include "Character/BaseCharacter.h"
#include "Character/CustomizationCharacter.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "LastCanary.h"

#define BIND_BUTTON(BUTTON_NAME, FUNC) \
	if (BUTTON_NAME) BUTTON_NAME->OnClicked.AddUniqueDynamic(this, &UCharacterCustomizationWidget::FUNC);

#define UNBIND_BUTTON(BUTTON_NAME, FUNC) \
	if (BUTTON_NAME) BUTTON_NAME->OnClicked.RemoveDynamic(this, &UCharacterCustomizationWidget::FUNC);

void UCharacterCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetVisibility(ESlateVisibility::Visible);

	// 스켈레탈 메시 //
	BIND_BUTTON(ViewNextFullBodyButton, OnSelectFullBody);
	BIND_BUTTON(ViewNextGloveButton, OnSelectGlove);
	BIND_BUTTON(ViewNextJacketButton, OnSelectJacket);
	BIND_BUTTON(ViewNextPantsButton, OnSelectPants);
	BIND_BUTTON(ViewNextBeltsButton, OnSelectBelts);
	BIND_BUTTON(ViewNextHelmetsButton, OnSelectHelmets);
	BIND_BUTTON(ViewNextArmorButton, OnSelectArmor);
	BIND_BUTTON(ViewNextBootsButton, OnSelectBoots);

	// 머티리얼 //
	BIND_BUTTON(ViewNextGloveMaterialButton, OnSelectGloveMaterial);
	BIND_BUTTON(ViewNextJacketMaterialButton, OnSelectJacketMaterial);
	BIND_BUTTON(ViewNextPantsMaterialButton, OnSelectPantsMaterial);
	BIND_BUTTON(ViewNextBeltsMaterialButton, OnSelectBeltsMaterial);
	BIND_BUTTON(ViewNextHelmetsMaterialButton, OnSelectHelmetsMaterial);
	BIND_BUTTON(ViewNextBootsMaterialButton, OnSelectBootsMaterial);
	BIND_BUTTON(ViewNextArmorMaterialButton, OnSelectArmorMaterial);
	BIND_BUTTON(ViewNextFlagMaterialButton, OnSelectFlagMaterial);

	BIND_BUTTON(ApplyButton, ApplySetting);

	InitWidget();
}

void UCharacterCustomizationWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// 스켈레탈 메시 //
	UNBIND_BUTTON(ViewNextFullBodyButton, OnSelectFullBody);
	UNBIND_BUTTON(ViewNextGloveButton, OnSelectGlove);
	UNBIND_BUTTON(ViewNextJacketButton, OnSelectJacket);
	UNBIND_BUTTON(ViewNextPantsButton, OnSelectPants);
	UNBIND_BUTTON(ViewNextBeltsButton, OnSelectBelts);
	UNBIND_BUTTON(ViewNextHelmetsButton, OnSelectHelmets);
	UNBIND_BUTTON(ViewNextArmorButton, OnSelectArmor);
	UNBIND_BUTTON(ViewNextBootsButton, OnSelectBoots);

	// 머티리얼 //
	UNBIND_BUTTON(ViewNextGloveMaterialButton, OnSelectGloveMaterial);
	UNBIND_BUTTON(ViewNextJacketMaterialButton, OnSelectJacketMaterial);
	UNBIND_BUTTON(ViewNextPantsMaterialButton, OnSelectPantsMaterial);
	UNBIND_BUTTON(ViewNextBeltsMaterialButton, OnSelectBeltsMaterial);
	UNBIND_BUTTON(ViewNextHelmetsMaterialButton, OnSelectHelmetsMaterial);
	UNBIND_BUTTON(ViewNextBootsMaterialButton, OnSelectBootsMaterial);
	UNBIND_BUTTON(ViewNextArmorMaterialButton, OnSelectArmorMaterial);
	UNBIND_BUTTON(ViewNextFlagMaterialButton, OnSelectFlagMaterial);

	// UI 관련 //
	UNBIND_BUTTON(ApplyButton, ApplySetting);

	CancelCameraLerp();
}

FReply UCharacterCustomizationWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		LastMousePosition = InMouseEvent.GetScreenSpacePosition();
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	return FReply::Unhandled();
}

FReply UCharacterCustomizationWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply UCharacterCustomizationWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();

	if (UDesktopWindowBaseWidget* ParentWindow = Cast<UDesktopWindowBaseWidget>(GetParent()))
	{
		if (ParentWindow->IsInTitleBar(MousePos))
		{
			return FReply::Unhandled();
		}
	}

	if (bIsDragging && PreviewCharacter)
	{
		constexpr float RotationSensitivity = 0.5f;

		const FVector2D CurrentMousePosition = InMouseEvent.GetScreenSpacePosition();
		const float DeltaX = CurrentMousePosition.X - LastMousePosition.X;
		LastMousePosition = CurrentMousePosition;

		// 회전값 변경
		FRotator NewRotation = PreviewCharacter->GetActorRotation();
		NewRotation.Yaw -= DeltaX * RotationSensitivity;
		PreviewCharacter->SetActorRotation(NewRotation);
	}
	return FReply::Unhandled();
}

FReply UCharacterCustomizationWidget::NativeOnMouseWheel(const FGeometry& G, const FPointerEvent& E)
{
	constexpr float ZoomStep = 12.f;
	PreviewCamDistance = FMath::Clamp(PreviewCamDistance - E.GetWheelDelta() * ZoomStep, 60.f, 260.f);
	UpdatePreviewCameraTransform();

	return FReply::Handled();
}

void UCharacterCustomizationWidget::InitWidget()
{
	//UpdateTargetCharacter();
	InitCharacterPreview();
}

void UCharacterCustomizationWidget::SetTargetCharacter(ABaseCharacter* InCharacter)
{
	TargetCharacter = InCharacter;

	if (CharacterMeshData)
	{
		OnLoadCustomization();
	}
}

void UCharacterCustomizationWidget::InitCharacterPreview()
{
	if (PreviewRenderTarget == nullptr || CharacterMeshData == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 기존 프리뷰 캐릭터 제거
	if (PreviewCharacter)
	{
		if (SceneCaptureComponent)
		{
			SceneCaptureComponent->DestroyComponent();
			SceneCaptureComponent = nullptr;
		}
		PreviewCharacter->Destroy();
		PreviewCharacter = nullptr;
	}

	// 캐릭터 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector PreviewOrigin = FVector(0.f, 0.f, 0.f);
	const FRotator PreviewRot = FRotator(0.f, 0.f, 0.f);
	PreviewCharacter = World->SpawnActor<ACustomizationCharacter>(
		ACustomizationCharacter::StaticClass(),
		PreviewOrigin,
		PreviewRot,
		SpawnParams
	);

	constexpr float MeshYawOffset = -90.f;
	if (PreviewCharacter && PreviewCharacter->GetMesh())
	{
		PreviewCharacter->GetMesh()->SetRelativeRotation(FRotator(0.f, MeshYawOffset, 0.f));
		PreviewCharacter->GetMesh()->SetRenderCustomDepth(true);
		PreviewCharacter->GetMesh()->SetCustomDepthStencilValue(1);
	}

	// SceneCapture 생성
	SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(this);
	SceneCaptureComponent->RegisterComponentWithWorld(World);
	SceneCaptureComponent->TextureTarget = PreviewRenderTarget;

	// 카메라 위치/회전 세팅
	const float CamDistance = 150.f;
	const float CamHeight = 180.f;
	const FVector CamLoc = PreviewOrigin + FVector(CamDistance, 0.f, CamHeight);
	SceneCaptureComponent->SetWorldLocation(CamLoc);

	const FVector LookTarget = PreviewOrigin + FVector(0.f, 0.f, CamHeight - 30.f);
	const FRotator CamRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, LookTarget);
	SceneCaptureComponent->SetWorldRotation(CamRot);

	SceneCaptureComponent->FOVAngle = PreviewCamFOV;
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	USpotLightComponent* PreviewLight = NewObject<USpotLightComponent>(this);
	PreviewLight->RegisterComponentWithWorld(World);
	PreviewLight->SetWorldLocation(CamLoc + FVector(-30.f, 0.f, 60.f));
	PreviewLight->SetWorldRotation(CamRot);
	PreviewLight->Intensity = 15000.f;
	PreviewLight->AttenuationRadius = 1000.f;
	PreviewLight->InnerConeAngle = 20.f;
	PreviewLight->OuterConeAngle = 45.f;
	PreviewLight->bUseInverseSquaredFalloff = false;
	PreviewLight->LightColor = FColor::White;
	PreviewLight->AttachToComponent(SceneCaptureComponent, FAttachmentTransformRules::KeepWorldTransform);

	const TCHAR* MaterialPath = TEXT("/Game/_LastCanary/Art/Metal/M_PP_CustomDepthOnly.M_PP_CustomDepthOnly");
	UMaterialInterface* PPAsset = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, MaterialPath));
	if (PPAsset)
	{
		SceneCaptureComponent->PostProcessSettings.WeightedBlendables.Array.Add(
			FWeightedBlendable(1.0f, PPAsset)
		);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ PostProcess 머티리얼 로드 실패: %s"), MaterialPath);
	}

	// 환경 효과 제거
	SceneCaptureComponent->ShowFlags.SetSkyLighting(false);
	SceneCaptureComponent->ShowFlags.SetFog(false);
	SceneCaptureComponent->ShowFlags.SetAtmosphere(false);
	SceneCaptureComponent->ShowFlags.SetMotionBlur(false);
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR; // 조명 포함
	SceneCaptureComponent->ShowFlags.SetGame(true); // 게임 조명 등 포함
	SceneCaptureComponent->ShowFlags.SetLighting(true); // 조명 계산 허용


	// 미리보기 머티리얼 적용
	if (UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/_LastCanary/Art/Metal/M_UIRTPreviewDisplay.M_UIRTPreviewDisplay")))
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		DynMat->SetTextureParameterValue("PreviewTexture", PreviewRenderTarget);
		CharacterPreviewImage->SetBrushFromMaterial(DynMat);
	}

	UpdatePreviewCharacter();
	FacePreviewToCamera();
	UpdatePreviewCameraTransform();
}

void UCharacterCustomizationWidget::UpdatePreviewCharacter()
{
	if (PreviewRenderTarget == nullptr)
	{
		return;
	}
	if (CharacterMeshData == nullptr)
	{
		return;
	}

	// Body
	SetPartMesh(PreviewCharacter->GetMesh(), CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultBodyMeshes, CurrentFullBodyIndex));
	SetPartMaterial(PreviewCharacter->GetMesh(), 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->DefaultBodyMaterials, CurrentFullBodyMaterialIndex));

	// Head
	SetPartMesh(PreviewCharacter->CustomHeadMesh, CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultHeadMeshes, CurrentFullBodyIndex));
	SetPartMaterial(PreviewCharacter->CustomHeadMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->DefaultHeadMaterials, CurrentFullBodyMaterialIndex));

	// Helmet
	SetPartMesh(PreviewCharacter->CustomHelmetMesh, CharacterMeshData->GetMeshByID(CharacterMeshData->HelmetMeshes, CurrentHelmetsIndex));
	SetPartMaterial(PreviewCharacter->CustomHelmetMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->HelmetMaterials, CurrentHelmetMaterialIndex));
	SetPartMaterial(PreviewCharacter->CustomHelmetMesh, 1, CharacterMeshData->GetMaterialByID(CharacterMeshData->FlagMaterials, CurrentFlagMaterialIndex));

	// Glove
	SetPartMesh(PreviewCharacter->CustomGloveMesh, CharacterMeshData->GetMeshByID(CharacterMeshData->GloveMeshes, CurrentGloveIndex));
	SetPartMaterial(PreviewCharacter->CustomGloveMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->GloveMaterials, CurrentGloveMaterialIndex));

	// Jacket
	SetPartMesh(PreviewCharacter->CustomJacketMesh, CharacterMeshData->GetMeshByID(CharacterMeshData->JacketMeshes, CurrentJacketIndex));
	SetPartMaterial(PreviewCharacter->CustomJacketMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->JacketMaterials, CurrentJacketMaterialIndex));

	// Pants
	SetPartMesh(PreviewCharacter->CustomPantsMesh, CharacterMeshData->GetMeshByID(CharacterMeshData->PantsMeshes, CurrentPantsIndex));
	SetPartMaterial(PreviewCharacter->CustomPantsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->PantsMaterials, CurrentPantsMaterialIndex));

	// Belts
	SetPartMesh(PreviewCharacter->CustomBeltsMesh, CharacterMeshData->GetMeshByID(CharacterMeshData->BeltsMeshes, CurrentBeltsIndex));
	SetPartMaterial(PreviewCharacter->CustomBeltsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->BeltsMaterials, CurrentBeltsMaterialIndex));

	// Armor
	SetPartMesh(PreviewCharacter->CustomArmorMesh, CharacterMeshData->GetMeshByID(CharacterMeshData->ArmorMeshes, CurrentArmorIndex));
	SetPartMaterial(PreviewCharacter->CustomArmorMesh, 1, CharacterMeshData->GetMaterialByID(CharacterMeshData->ArmorMaterials, CurrentArmorMaterialIndex));
	SetPartMaterial(PreviewCharacter->CustomArmorMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->FlagMaterials, CurrentFlagMaterialIndex));

	// Boots
	SetPartMesh(PreviewCharacter->CustomBootsMesh, CharacterMeshData->GetMeshByID(CharacterMeshData->BootsMeshes, CurrentBootsIndex));
	SetPartMaterial(PreviewCharacter->CustomBootsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->BootsMaterials, CurrentBootsMaterialIndex));
}

void UCharacterCustomizationWidget::UpdateCustomization()
{
	if (CharacterMeshData->IsValidLowLevel() == false)
	{
		LOG_Char_WARNING(TEXT("캐릭터 메시 데이터 invalid"));
		return;
	}

	if (IsValid(PreviewCharacter))
	{
		ApplyCustomizationToCharacter(PreviewCharacter);
	}
}

void UCharacterCustomizationWidget::OnSelectFullBody()
{
	int32 Maxindex = CharacterMeshData->DefaultBodyMeshes.Num() - 1;
	CurrentFullBodyIndex++;
	if (CurrentFullBodyIndex > Maxindex)
	{
		CurrentFullBodyIndex = 0;
	}
	CurrentSelection.DefaultBodyID = CurrentFullBodyIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::FullBody);
}

void UCharacterCustomizationWidget::OnSelectGlove()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->GloveMeshes.Num() - 1;
	CurrentGloveIndex++;
	if (CurrentGloveIndex > Maxindex)
	{
		CurrentGloveIndex = 0;
	}
	CurrentSelection.GloveID = CurrentGloveIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Gloves);
}
void UCharacterCustomizationWidget::OnSelectJacket()
{
	LOG_Char_WARNING(TEXT("자켓 버튼 클릭"));

	if (CharacterMeshData == nullptr)
	{
		LOG_Char_WARNING(TEXT("데이터 없음"));
		return;
	}
	LOG_Char_WARNING(TEXT("maxindex = %d"), CharacterMeshData->JacketMeshes.Num());
	int32 Maxindex = CharacterMeshData->JacketMeshes.Num() - 1;
	LOG_Char_WARNING(TEXT("CurrentJacketIndex= %d"), CurrentJacketIndex);
	CurrentJacketIndex++;
	if (CurrentJacketIndex > Maxindex)
	{
		CurrentJacketIndex = 0;
	}
	CurrentSelection.JacketID = CurrentJacketIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Chest);
}

void UCharacterCustomizationWidget::OnSelectPants()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->PantsMeshes.Num() - 1;
	CurrentPantsIndex++;
	if (CurrentPantsIndex > Maxindex)
	{
		CurrentPantsIndex = 0;
	}
	CurrentSelection.PantsID = CurrentPantsIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Pants);
}
void UCharacterCustomizationWidget::OnSelectBelts()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->BeltsMeshes.Num() - 1;
	CurrentBeltsIndex++;
	if (CurrentBeltsIndex > Maxindex)
	{
		CurrentBeltsIndex = 0;
	}
	CurrentSelection.BeltsID = CurrentBeltsIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Pants);
}

void UCharacterCustomizationWidget::OnSelectHelmets()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->HelmetMeshes.Num() - 1;
	CurrentHelmetsIndex++;
	if (CurrentHelmetsIndex > Maxindex)
	{
		CurrentHelmetsIndex = 0;
	}
	CurrentSelection.HelmetID = CurrentHelmetsIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Head);
}

void UCharacterCustomizationWidget::OnSelectArmor()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->ArmorMeshes.Num() - 1;
	CurrentArmorIndex++;
	if (CurrentArmorIndex > Maxindex)
	{
		CurrentArmorIndex = 0;
	}
	CurrentSelection.ArmorID = CurrentArmorIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Chest);
}

void UCharacterCustomizationWidget::OnSelectBoots()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->BootsMeshes.Num() - 1;
	CurrentBootsIndex++;
	if (CurrentBootsIndex > Maxindex)
	{
		CurrentBootsIndex = 0;
	}
	CurrentSelection.BootsID = CurrentBootsIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Boots);
}

void UCharacterCustomizationWidget::OnSelectGloveMaterial()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->GloveMaterials.Num() - 1;
	CurrentGloveMaterialIndex++;
	if (CurrentGloveMaterialIndex > Maxindex)
	{
		CurrentGloveMaterialIndex = 0;
	}
	CurrentSelection.GloveMaterialID = CurrentGloveMaterialIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Gloves);
}

void UCharacterCustomizationWidget::OnSelectJacketMaterial()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->JacketMaterials.Num() - 1;
	CurrentJacketMaterialIndex++;
	if (CurrentJacketMaterialIndex > Maxindex)
	{
		CurrentJacketMaterialIndex = 0;
	}
	CurrentSelection.JacketMaterialID = CurrentJacketMaterialIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Chest);
}

void UCharacterCustomizationWidget::OnSelectPantsMaterial()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->PantsMaterials.Num() - 1;
	CurrentPantsMaterialIndex++;
	if (CurrentPantsMaterialIndex > Maxindex)
	{
		CurrentPantsMaterialIndex = 0;
	}
	CurrentSelection.PantsMaterialID = CurrentPantsMaterialIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Pants);
}

void UCharacterCustomizationWidget::OnSelectBeltsMaterial()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->BeltsMaterials.Num() - 1;
	CurrentBeltsMaterialIndex++;
	if (CurrentBeltsMaterialIndex > Maxindex)
	{
		CurrentBeltsMaterialIndex = 0;
	}
	CurrentSelection.BeltsMaterialID = CurrentBeltsMaterialIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Pants);
}

void UCharacterCustomizationWidget::OnSelectHelmetsMaterial()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->HelmetMaterials.Num() - 1;
	CurrentHelmetMaterialIndex++;
	if (CurrentHelmetMaterialIndex > Maxindex)
	{
		CurrentHelmetMaterialIndex = 0;
	}
	CurrentSelection.HelmetMaterialID = CurrentHelmetMaterialIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Head);
}

void UCharacterCustomizationWidget::OnSelectBootsMaterial()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->BootsMaterials.Num() - 1;
	CurrentBootsMaterialIndex++;
	if (CurrentBootsMaterialIndex > Maxindex)
	{
		CurrentBootsMaterialIndex = 0;
	}
	CurrentSelection.BootsMaterialID = CurrentBootsMaterialIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Boots);
}

void UCharacterCustomizationWidget::OnSelectArmorMaterial()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->ArmorMaterials.Num() - 1;
	CurrentArmorMaterialIndex++;
	if (CurrentArmorMaterialIndex > Maxindex)
	{
		CurrentArmorMaterialIndex = 0;
	}
	CurrentSelection.ArmorMaterialID = CurrentArmorMaterialIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Chest);
}

void UCharacterCustomizationWidget::OnSelectFlagMaterial()
{
	if (CharacterMeshData == nullptr)
	{
		return;
	}
	int32 Maxindex = CharacterMeshData->FlagMaterials.Num() - 1;
	CurrentFlagMaterialIndex++;
	if (CurrentFlagMaterialIndex > Maxindex)
	{
		CurrentFlagMaterialIndex = 0;
	}
	CurrentSelection.FlagMaterialID = CurrentFlagMaterialIndex;

	UpdateCustomization();
	RequestFocus(EPreviewFocusGroup::Flag);
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
		InitCustomizationIndex(SavedCustomizationData);
	}

	ACharacter* MyChar = nullptr;
	if (IsValid(TargetCharacter))
	{
		MyChar = TargetCharacter;
	}
	else if (APlayerController* PC = GetOwningPlayer())
	{
		MyChar = Cast<ACharacter>(PC->GetPawn());
	}
	if (IsValid(MyChar))
	{
		ApplyCustomizationToCharacter(MyChar);
	}
}

void UCharacterCustomizationWidget::SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh)
{
	if (Component == nullptr)
	{
		return;
	}

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
	CurrentFlagMaterialIndex = SavedCustomizationData.FlagMaterialID;

	UpdateCustomization();
}

void UCharacterCustomizationWidget::ApplyCustomizationToCharacter(ACharacter* Character)
{
	if (!Character || !CharacterMeshData)
	{
		return;
	}

	// Body
	USkeletalMesh* BodySkeletalMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultBodyMeshes, CurrentFullBodyIndex);
	SetPartMesh(Character->GetMesh(), BodySkeletalMesh);
	SetPartMaterial(Character->GetMesh(), 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->DefaultBodyMaterials, CurrentFullBodyMaterialIndex));

	// PreviewCharacter나 TargetCharacter 모두 지원하게 캐스팅 시도
	if (ACustomizationCharacter* CustomChar = Cast<ACustomizationCharacter>(Character))
	{
		LOG_Char_WARNING(TEXT("캐릭터 메시 데이터 invalid"));

		// Head
		USkeletalMesh* HeadMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->DefaultHeadMeshes, CurrentFullBodyIndex);
		SetPartMesh(CustomChar->CustomHeadMesh, HeadMesh);
		SetPartMaterial(CustomChar->CustomHeadMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->DefaultHeadMaterials, CurrentFullBodyMaterialIndex));

		// Helmet
		USkeletalMesh* HelmetMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->HelmetMeshes, CurrentHelmetsIndex);
		SetPartMesh(CustomChar->CustomHelmetMesh, HelmetMesh);
		SetPartMaterial(CustomChar->CustomHelmetMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->HelmetMaterials, CurrentHelmetMaterialIndex));
		SetPartMaterial(CustomChar->CustomHelmetMesh, 1, CharacterMeshData->GetMaterialByID(CharacterMeshData->FlagMaterials, CurrentFlagMaterialIndex));

		// Glove
		USkeletalMesh* GloveMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->GloveMeshes, CurrentGloveIndex);
		SetPartMesh(CustomChar->CustomGloveMesh, GloveMesh);
		SetPartMaterial(CustomChar->CustomGloveMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->GloveMaterials, CurrentGloveMaterialIndex));

		// Pants
		USkeletalMesh* PantsMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->PantsMeshes, CurrentPantsIndex);
		SetPartMesh(CustomChar->CustomPantsMesh, PantsMesh);
		SetPartMaterial(CustomChar->CustomPantsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->PantsMaterials, CurrentPantsMaterialIndex));

		// Belts
		USkeletalMesh* BeltsMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->BeltsMeshes, CurrentBeltsIndex);
		SetPartMesh(CustomChar->CustomBeltsMesh, BeltsMesh);
		SetPartMaterial(CustomChar->CustomBeltsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->BeltsMaterials, CurrentBeltsMaterialIndex));

		//jacket
		USkeletalMesh* JacketMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->JacketMeshes, CurrentJacketIndex);
		SetPartMesh(CustomChar->CustomJacketMesh, JacketMesh);
		SetPartMaterial(CustomChar->CustomJacketMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->JacketMaterials, CurrentJacketMaterialIndex));

		// Armor
		USkeletalMesh* ArmorMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->ArmorMeshes, CurrentArmorIndex);
		SetPartMesh(CustomChar->CustomArmorMesh, ArmorMesh);
		SetPartMaterial(CustomChar->CustomArmorMesh, 1, CharacterMeshData->GetMaterialByID(CharacterMeshData->ArmorMaterials, CurrentArmorMaterialIndex));
		SetPartMaterial(CustomChar->CustomArmorMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->FlagMaterials, CurrentFlagMaterialIndex));

		// Boots
		USkeletalMesh* BootsMesh = CharacterMeshData->GetMeshByID(CharacterMeshData->BootsMeshes, CurrentBootsIndex);
		SetPartMesh(CustomChar->CustomBootsMesh, BootsMesh);
		SetPartMaterial(CustomChar->CustomBootsMesh, 0, CharacterMeshData->GetMaterialByID(CharacterMeshData->BootsMaterials, CurrentBootsMaterialIndex));
	}
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
	CurrentSelection.FlagMaterialID = CurrentFlagMaterialIndex;

	ULCLocalPlayerSaveGame::SaveCustomizationData(GetWorld(), CurrentSelection);

	ACharacter* MyChar = nullptr;
	if (IsValid(TargetCharacter))
	{
		MyChar = TargetCharacter;
	}
	else if (APlayerController* PC = GetOwningPlayer())
	{
		MyChar = Cast<ACharacter>(PC->GetPawn());
	}

	if (IsValid(MyChar))
	{
		ABaseCharacter* BaseChar = Cast<ABaseCharacter>(MyChar);
		BaseChar->ApplyCustomizationToAllPlayers(CurrentSelection);
		//ApplyCustomizationToCharacter(BaseChar);
	}
	else
	{
		LOG_Char_WARNING(TEXT("ApplySetting: 적용할 대상 캐릭터를 찾지 못했습니다 (TargetCharacter/소유 Pawn 모두 없음)."));
	}

	if (IsValid(PreviewCharacter))
	{
		ApplyCustomizationToCharacter(PreviewCharacter);
	}

	//if (APlayerController* PC = GetOwningPlayer())
	//{
	//	if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(PC))
	//	{
	//		RoomPC->Server_ApplyCustomization(CurrentSelection);
	//	}
	//}

	//LOG_Char_WARNING(TEXT("세팅 저장 + 타겟/프리뷰 적용 + 서버 요청 완료"));
}

void UCharacterCustomizationWidget::FacePreviewToCamera()
{
	if (PreviewCharacter == nullptr || SceneCaptureComponent == nullptr)
	{
		return;
	}

	const FVector CharLoc = PreviewCharacter->GetActorLocation();
	const FVector CamLoc = SceneCaptureComponent->GetComponentLocation();

	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(CharLoc, CamLoc);
	const FRotator YawOnly(0.f, LookAt.Yaw, 0.f);

	PreviewCharacter->SetActorRotation(YawOnly);
}

void UCharacterCustomizationWidget::UpdatePreviewCameraTransform()
{
	if (PreviewCharacter == nullptr || SceneCaptureComponent == nullptr)
	{
		return;
	}

	const FVector CharLoc = PreviewCharacter->GetActorLocation();
	FVector Target = CharLoc;

	switch (LastFocusedGroup)
	{
	case EPreviewFocusGroup::Head:
	{
		Target.Z += 170.f;
		break;
	}
	case EPreviewFocusGroup::Chest:
	{
		Target.Z += 140.f;
		break;
	}
	case EPreviewFocusGroup::Flag:
	{
		Target.Z += 140.f;
		break;
	}
	case EPreviewFocusGroup::Pants:
	{
		Target.Z += 110.f;
		break;
	}
	case EPreviewFocusGroup::Boots:
	{
		Target.Z += 55.f;
		break;
	}
	default:
	{
		Target.Z += 140.f;
		break;
	}
	}

	const FRotator CurrentRot = SceneCaptureComponent->GetComponentRotation();
	const FVector CamDir = CurrentRot.Vector();
	const FVector NewCamLoc = Target - CamDir * PreviewCamDistance;

	SceneCaptureComponent->SetWorldLocationAndRotation(NewCamLoc, CurrentRot);
}

void UCharacterCustomizationWidget::RequestFocus(EPreviewFocusGroup Group)
{
	if (PreviewCharacter == nullptr || SceneCaptureComponent == nullptr)
	{
		return;
	}

	const FVector CharLoc = PreviewCharacter->GetActorLocation();
	FVector Target = CharLoc;

	float CamDist = PreviewCamDistance;
	FRotator NewCamRot = SceneCaptureComponent->GetComponentRotation();

	switch (Group)
	{
	case EPreviewFocusGroup::FullBody:
	{
		Target.Z += 100.f;
		CamDist = 180.f;
		NewCamRot = FRotator(-15.f, 180.f, 0.f);
		break;
	}
	case EPreviewFocusGroup::Head:
	{
		Target.Z += 170.f;
		CamDist *= 0.6f;
		break;
	}
	case EPreviewFocusGroup::Chest:
	{
		Target.Z += 140.f;
		break;
	}
	case EPreviewFocusGroup::Flag:
	{
		Target.Z += 140.f;
		CamDist *= 0.4f;
		break;
	}
	case EPreviewFocusGroup::Gloves:
	{
		Target.Z += 140.f;
		CamDist *= 1.2f;
		break;
	}
	case EPreviewFocusGroup::Pants:
	{
		Target.Z += 110.f;
		break;
	}
	case EPreviewFocusGroup::Boots:
	{
		Target.Z += 55.f;
		break;
	}
	default:
	{
		Target.Z += 140.f;
		break;
	}
	}

	const FVector CamDir = NewCamRot.Vector();
	const FVector NewCamLoc = Target - CamDir * CamDist;

	StartCameraLerp(NewCamLoc, NewCamRot, 0.25f);
	LastFocusedGroup = Group;
}

void UCharacterCustomizationWidget::RecenterPreview()
{
	if (SceneCaptureComponent == nullptr || PreviewCharacter == nullptr)
	{
		return;
	}

	const FVector CharLoc = PreviewCharacter->GetActorLocation();
	const FVector Target = CharLoc + FVector(0, 0, 140.f);
	const FVector NewLoc = Target + FVector(PreviewCamDistance, 0, PreviewCamHeight);
	const FRotator NewRot = UKismetMathLibrary::FindLookAtRotation(NewLoc, Target);

	StartCameraLerp(NewLoc, NewRot, 0.25f);
}

void UCharacterCustomizationWidget::StartCameraLerp(const FVector& TargetLoc, const FRotator& TargetRot, float Duration)
{
	if (SceneCaptureComponent == nullptr)
	{
		return;
	}

	CancelCameraLerp();

	CamLerpStartLoc = SceneCaptureComponent->GetComponentLocation();
	CamLerpStartRot = SceneCaptureComponent->GetComponentRotation();

	CamLerpTargetLoc = TargetLoc;
	CamLerpTargetRot = TargetRot;

	CamLerpDuration = FMath::Max(0.01f, Duration);
	CamLerpElapsed = 0.f;
	LastLerpTickTime = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PreviewCamLerpHandle,
			this, &UCharacterCustomizationWidget::TickCameraLerp,
			0.01f, true
		);
	}
}

void UCharacterCustomizationWidget::TickCameraLerp()
{
	if (SceneCaptureComponent == nullptr)
	{
		CancelCameraLerp();
		return;
	}

	const double Now = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
	const float  DT = static_cast<float>(Now - LastLerpTickTime);
	LastLerpTickTime = Now;

	CamLerpElapsed += DT;
	const float Alpha = FMath::Clamp(CamLerpElapsed / CamLerpDuration, 0.f, 1.f);

	const FVector  NewLoc = FMath::Lerp(CamLerpStartLoc, CamLerpTargetLoc, Alpha);
	const FQuat    SR = CamLerpStartRot.Quaternion();
	const FQuat    TR = CamLerpTargetRot.Quaternion();
	const FRotator NewRot = FQuat::Slerp(SR, TR, Alpha).Rotator();

	SceneCaptureComponent->SetWorldLocation(NewLoc);
	SceneCaptureComponent->SetWorldRotation(NewRot);

	if (Alpha >= 1.f)
	{
		CancelCameraLerp();
	}
}

void UCharacterCustomizationWidget::CancelCameraLerp()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PreviewCamLerpHandle);
	}
}
