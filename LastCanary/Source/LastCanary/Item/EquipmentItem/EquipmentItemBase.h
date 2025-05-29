// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemBase.h"
#include "EquipmentItemBase.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API AEquipmentItemBase : public AItemBase
{
	GENERATED_BODY()
	
public:
    AEquipmentItemBase();

    // 장비 아이템 사용 함수를 가상 함수로 선언
    virtual void UseItem() override;


    // 장비 상태 확인
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool IsEquipped() const { return bIsEquipped; }

    // 장비 착용/해제 처리
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    virtual void SetEquipped(bool bNewEquipped);

};
