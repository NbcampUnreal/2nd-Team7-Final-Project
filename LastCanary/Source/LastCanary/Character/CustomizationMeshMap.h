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

UCLASS(BlueprintType, Blueprintable)
class LASTCANARY_API UCustomizationMeshMap : public UDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> DefaultBodyMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> DefaultHeadMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> GloveMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> JacketMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> PantsMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> BeltsMeshes;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMeshPartEntry> HelmetMeshes;
    // ... 다른 부위들 추가 가능

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
};