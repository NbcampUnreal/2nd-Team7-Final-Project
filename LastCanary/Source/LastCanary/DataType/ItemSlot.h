// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemBase.h"
#include "ItemSlot.generated.h"

USTRUCT(BlueprintType)
struct FItemSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY()
    AItemBase* ItemActor = nullptr;

    //FInventorySlot()
    //    : ItemRowName(NAME_None), Quantity(0) {
    //}
};
