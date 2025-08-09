// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"
#include "CharacterCustomizationComponent.generated.h"

class UCustomizationMeshMap;
struct FCharacterCustomizationData;
class UCustomizationMeshMap;

UCLASS()
class LASTCANARY_API UCharacterCustomizationComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
	USkeletalMeshComponent* CharacterMesh();

	USkeletalMeshComponent* GetHeadMesh();

	USkeletalMeshComponent* GetGloveMesh();

	USkeletalMeshComponent* GetJacketMesh_OwnerNoSee();

	USkeletalMeshComponent* GetJacketMesh_OwnerSee();

	USkeletalMeshComponent* GetPantsMesh();

	USkeletalMeshComponent* GetBeltsMesh();

	USkeletalMeshComponent* GetHelmetMesh();

	USkeletalMeshComponent* GetArmorMesh();

	USkeletalMeshComponent* GetBootsMesh();

	USkeletalMeshComponent* GetBackpackMesh();

	void SetCharacterPoseSynchronization();


public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	UCustomizationMeshMap* CharacterMeshMap;

	void ApplyCustomization(const FCharacterCustomizationData CustomizationData);

	void SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh);

	void SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material);

	FCharacterCustomizationData GetCustomizationData();
	void SetCustomizationData(const FCharacterCustomizationData& CustomizingData);

	UFUNCTION(Server, Reliable)
	void Server_UpdateCustomizationData();  // 이건 안쓰는듯?
	void Server_UpdateCustomizationData_Implementation();

	UFUNCTION(Server, Reliable)
	void Server_SetCustomizationData(const FCharacterCustomizationData& CustomizingData);
	void Server_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCustomizationData(const FCharacterCustomizationData& CustomizingData);
	void Multicast_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData);

	FCharacterCustomizationData CharacterCustomizationData;

	FTimerHandle RetryCustomizationHandle;
	void InitializeCustomization();
};
