// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"
#include "CharacterCustomizationWidget.generated.h"

class ACustomizationCharacter;
class UCustomizationMeshMap;
class UButton;

UCLASS()
class LASTCANARY_API UCharacterCustomizationWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
public:
	void InitWidget();
	void UpdateTargetCharacter();
public:
	// 부모 캐릭터 참조
	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	ACustomizationCharacter* TargetCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	UCustomizationMeshMap* CharacterMeshData;

	// 현재 선택된 ID들
	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	FCharacterCustomizationData CurrentSelection;

	// 스켈레탈 메시 //

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
	
	
	// 머티리얼 //

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


	// UI 관련 //

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ApplyButton;


	UFUNCTION(BlueprintCallable)
	void CloseCustomizationWidget();

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentFullBodyIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentGloveIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentJacketIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentPantsIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentBeltsIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentHelmetsIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentArmorIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentBootsIndex;

	// 머티리얼 인덱스 추가
	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentFullBodyMaterialIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentGloveMaterialIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentJacketMaterialIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentPantsMaterialIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentBeltsMaterialIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentHelmetMaterialIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentArmorMaterialIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentBootsMaterialIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	int32 CurrentFlagMaterialIndex;

	//* 버튼 클릭 바인딩 함수 *//

	// 스켈레탈 메시 // 
	UFUNCTION(BlueprintCallable)
	void OnSelectFullBody();

	UFUNCTION(BlueprintCallable)
	void OnSelectGlove();

	UFUNCTION(BlueprintCallable)
	void OnSelectJacket();

	UFUNCTION(BlueprintCallable)
	void OnSelectPants();

	UFUNCTION(BlueprintCallable)
	void OnSelectBelts();

	UFUNCTION(BlueprintCallable)
	void OnSelectHelmets();

	UFUNCTION(BlueprintCallable)
	void OnSelectArmor();

	UFUNCTION(BlueprintCallable)
	void OnSelectBoots();

	// 머티리얼 //
	UFUNCTION(BlueprintCallable)
	void OnSelectGloveMaterial();

	UFUNCTION(BlueprintCallable)
	void OnSelectJacketMaterial();

	UFUNCTION(BlueprintCallable)
	void OnSelectPantsMaterial();

	UFUNCTION(BlueprintCallable)
	void OnSelectBeltsMaterial();

	UFUNCTION(BlueprintCallable)
	void OnSelectHelmetsMaterial();

	UFUNCTION(BlueprintCallable)
	void OnSelectBootsMaterial();

	UFUNCTION(BlueprintCallable)
	void OnSelectArmorMaterial();

	UFUNCTION(BlueprintCallable)
	void OnSelectFlagMaterial();

	UFUNCTION(BlueprintCallable)
	void OnLoadCustomization(); // SaveGame 에서 저장된 값 호출

	UFUNCTION(BlueprintCallable)
	void OnConfirmCustomization(); // SaveGame 저장

	UFUNCTION(BlueprintCallable)
	void OnResetCustomization(); // 기본 값으로 초기화

	void InitCustomizationIndex(FCharacterCustomizationData SavedCustomizationData);
	void UpdateCustomization();

	UFUNCTION(BlueprintCallable)
	void SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh);
	
	UFUNCTION(BlueprintCallable)
	void SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material);


	UFUNCTION(BlueprintCallable)
	void ApplySetting();
};
