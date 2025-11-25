#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h" 
#include "CustomizationCharacter.generated.h"

class UCustomizationMeshMap;

UCLASS()
class LASTCANARY_API ACustomizationCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACustomizationCharacter();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomHeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomGloveMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomJacketMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomPantsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomBeltsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomHelmetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomArmorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CustomBootsMesh;

	/** 캐릭터에 메시/머티리얼 적용 */
	void ApplyCustomization(const UCustomizationMeshMap* Data);

	/** SkeletalMesh 교체 */
	void SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh);

	/** Material 교체 */
	void SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	UCustomizationMeshMap* CharacterMeshMap;
};
