#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"
#include "CharacterCustomizationWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UCustomizationMeshMap;
class ACustomizationCharacter;
class ABaseCharacter;

UENUM()
enum class EPreviewFocusGroup : uint8
{
	FullBody,
	Head, 
	Chest, 
	Flag,
	Gloves,
	Pants, 
	Boots
};

/**
 * 캐릭터 커스터마이징 UI
 */
UCLASS()
class LASTCANARY_API UCharacterCustomizationWidget : public UDesktopWindowBaseWidget
{
	GENERATED_BODY()

public:
	//-----------------
	// UUserWidget 생명주기
	//-----------------
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	//-----------------
	// 입력(드래그/휠) 처리
	//-----------------
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	//-----------------
	// 타깃(인게임 실제 캐릭터) 설정
	//-----------------
	void SetTargetCharacter(ABaseCharacter* InCharacter);

protected:
	//-----------------
	// 초기화/업데이트
	//-----------------
	void InitWidget();
	void OnLoadCustomization();
	void InitCustomizationIndex(FCharacterCustomizationData SavedCustomizationData);
	void UpdateCustomization();

	//-----------------
	// 프리뷰 캐릭터 / 캡처
	//-----------------
	void InitCharacterPreview();
	void UpdatePreviewCharacter();
	void ApplyCustomizationToCharacter(ACharacter* Character);

	//-----------------
	// 파트 적용 유틸
	//-----------------
	void SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* Mesh);
	void SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material);

	//-----------------
	// 버튼 핸들러
	//-----------------
	UFUNCTION()
	void ApplySetting();

	UFUNCTION()
	void OnResetCustomization();
	UFUNCTION()
	void OnConfirmCustomization();

	UFUNCTION()
	void OnSelectFullBody();
	UFUNCTION()
	void OnSelectGlove();
	UFUNCTION()
	void OnSelectJacket();
	UFUNCTION()
	void OnSelectPants();
	UFUNCTION()
	void OnSelectBelts();
	UFUNCTION()
	void OnSelectHelmets();
	UFUNCTION()
	void OnSelectArmor();
	UFUNCTION()
	void OnSelectBoots();

	UFUNCTION()
	void OnSelectGloveMaterial();
	UFUNCTION()
	void OnSelectJacketMaterial();
	UFUNCTION()
	void OnSelectPantsMaterial();
	UFUNCTION()
	void OnSelectBeltsMaterial();
	UFUNCTION()
	void OnSelectHelmetsMaterial();
	UFUNCTION()
	void OnSelectBootsMaterial();
	UFUNCTION()
	void OnSelectArmorMaterial();
	UFUNCTION()
	void OnSelectFlagMaterial();

protected:
	//-----------------
	// 참조(프리뷰/타깃/데이터)
	//-----------------
	UPROPERTY()
	ACustomizationCharacter* PreviewCharacter = nullptr;  // 프리뷰용 캐릭터
	UPROPERTY()
	ABaseCharacter* TargetCharacter = nullptr;   // 인게임 실제 캐릭터

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	UCustomizationMeshMap* CharacterMeshData;

	//-----------------
	// 렌더 타깃 / 씬 캡처 / 프리뷰 이미지
	//-----------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	UTextureRenderTarget2D* PreviewRenderTarget;

	UPROPERTY() USceneCaptureComponent2D* SceneCaptureComponent = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* CharacterPreviewImage;

	//-----------------
	// 현재 선택 인덱스 (메시)
	//-----------------
	int32 CurrentFullBodyIndex = 0;
	int32 CurrentGloveIndex = 0;
	int32 CurrentJacketIndex = 0;
	int32 CurrentPantsIndex = 0;
	int32 CurrentBeltsIndex = 0;
	int32 CurrentHelmetsIndex = 0;
	int32 CurrentArmorIndex = 0;
	int32 CurrentBootsIndex = 0;

	//-----------------
	// 현재 선택 인덱스 (머티리얼)
	//-----------------
	int32 CurrentFullBodyMaterialIndex = 0;
	int32 CurrentGloveMaterialIndex = 0;
	int32 CurrentJacketMaterialIndex = 0;
	int32 CurrentPantsMaterialIndex = 0;
	int32 CurrentBeltsMaterialIndex = 0;
	int32 CurrentHelmetMaterialIndex = 0;
	int32 CurrentArmorMaterialIndex = 0;
	int32 CurrentBootsMaterialIndex = 0;
	int32 CurrentFlagMaterialIndex = 0;

	//-----------------
	// 현재 선택(저장/로드용 구조체)
	//-----------------
	FCharacterCustomizationData CurrentSelection;

	//-----------------
	// 위젯 버튼 바인딩 대상
	//-----------------
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextFullBodyButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextGloveButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextJacketButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextPantsButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextBeltsButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextHelmetsButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextArmorButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextBootsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextGloveMaterialButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextJacketMaterialButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextPantsMaterialButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextBeltsMaterialButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextHelmetsMaterialButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextBootsMaterialButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextArmorMaterialButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ViewNextFlagMaterialButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ApplyButton;

private:
	//-----------------
	// 입력 상태(드래그)
	//-----------------
	bool      bIsDragging = false;
	FVector2D LastMousePosition;

	//-----------------
	// 프리뷰 카메라 파라미터
	//-----------------
	float   PreviewCamDistance = 140.f;
	float   PreviewCamHeight = 170.f;
	float   PreviewCamFOV = 55.0f;
	FVector PreviewTargetOffset = FVector(0.f, 0.f, 140.f);

	//-----------------
	// 카메라/시선 유틸
	//-----------------
	void FacePreviewToCamera();
	void UpdatePreviewCameraTransform();

	//-----------------
	// 포커스 제어 상태
	//-----------------
	EPreviewFocusGroup LastFocusedGroup = EPreviewFocusGroup::FullBody;
	void RequestFocus(EPreviewFocusGroup Group);

	UFUNCTION() 
	void RecenterPreview();

	float PreviewCamYaw = 0.f;     
	float PreviewCamPitch = 15.f;  

	//-----------------
	// 프리뷰 카메라 보간 상태
	//-----------------
	FTimerHandle PreviewCamLerpHandle;

	FVector  CamLerpStartLoc;
	FVector  CamLerpTargetLoc;
	FRotator CamLerpStartRot;
	FRotator CamLerpTargetRot;

	float  CamLerpElapsed = 0.f;
	float  CamLerpDuration = 0.25f;
	double LastLerpTickTime = 0.0;

	//-----------------
	// 프리뷰 카메라 보간 함수
	//-----------------
	void StartCameraLerp(const FVector& TargetLoc, const FRotator& TargetRot, float Duration = 0.25f);
	void TickCameraLerp();
	void CancelCameraLerp();

};
