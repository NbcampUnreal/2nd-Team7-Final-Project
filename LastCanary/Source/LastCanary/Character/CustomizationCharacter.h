// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this character's properties
	ACustomizationCharacter();

protected:
	// Called when the game starts or when spawned
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

	void ApplyCustomization(const UCustomizationMeshMap* Data);

	void SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh);

	void SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	UCustomizationMeshMap* CharacterMeshMap;
};
