// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CustomizationMeshMap.generated.h"

USTRUCT(BlueprintType)
struct FMeshPartEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    USkeletalMesh* Mesh;
};

USTRUCT(BlueprintType)
struct FMaterialPartEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UMaterialInterface* Material;
};

UCLASS(BlueprintType, Blueprintable)
class LASTCANARY_API UCustomizationMeshMap : public UDataAsset
{
	GENERATED_BODY()
public:
    ///* 스켈레탈 메시 *///

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> DefaultBodyMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> DefaultHeadMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> GloveMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> JacketMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> JacketMeshes_OwnerSee;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> PantsMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> BeltsMeshes;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> HelmetMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> ArmorMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> BootsMeshes;
    // ... 다른 부위들 추가 가능

    
    ///* 머티리얼 *///
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> DefaultBodyMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> DefaultHeadMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> GloveMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> JacketMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> PantsMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> BeltsMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> HelmetMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> ArmorMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> BootsMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMaterialPartEntry> FlagMaterials;

    UFUNCTION(BlueprintCallable)
    USkeletalMesh* GetMeshByID(const TArray<FMeshPartEntry>& Entries, int32 ID) const
    {
        for (const auto& Entry : Entries)
        {
            if (Entry.ID == ID)
                return Entry.Mesh;
        }
        return nullptr;
    }

    UFUNCTION(BlueprintCallable)
    UMaterialInterface* GetMaterialByID(const TArray<FMaterialPartEntry>& Entries, int32 ID) const
    {
        for (const auto& Entry : Entries)
        {
            if (Entry.ID == ID)
                return Entry.Material;
        }
        return nullptr;
    }
};