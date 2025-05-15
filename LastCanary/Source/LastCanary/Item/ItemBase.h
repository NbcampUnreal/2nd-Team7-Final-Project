// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataTable/ItemDataRow.h"
#include "ItemBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemStateChanged);

UCLASS()
class LASTCANARY_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemBase();

protected:
	virtual void BeginPlay() override;

public:	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
    FName ItemRowName;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Item, meta = (ShowOnlyInnerProperties))
    FItemDataRow ItemData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UDataTable* ItemDataTable;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    class USphereComponent* SphereComponent;

    UPROPERTY(BlueprintReadWrite)
    bool bIsEquipped;

    // UI는 아마도 UI매니저에서 실행되지 않을까 생각됨
    //UPROPERTY(EditAnywhere, Category = "Widget")
    //TSubclassOf<class UUserWidget> PickupWidgetClass;

    //UPROPERTY()
    //UUserWidget* PickupWidget;

    //UPROPERTY(VisibleAnywhere, Category = "Widget")
    //UWidgetComponent* PickupWidgetComponent;

    //UFUNCTION()
    //void ShowPickupPrompt(bool bShow);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 데이터테이블로 부터 받아온 값을 적용시키는 함수
    UFUNCTION()
    void ApplyItemDataFromTable();

    // 툴바에 장비된 아이템 사용 시 알림용
    UPROPERTY(BlueprintAssignable)
    FOnItemStateChanged OnItemStateChanged;

    UFUNCTION()
    void UseItem();

    UFUNCTION()
    bool IsCollectible() const;

    // 변동이 있는 변수들은 아이템이 직접 보유하고 있는 것이 좋다고 한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;     // 수량(아이템 하나에 겹쳐져 있는 수량)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Durability;   // 사용량(장비 같은 아이템에서 배터리와 같은 사용량 int32로 해도 괜찮을 지도 고민 필요)



#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    /*virtual void OnConstruction(const FTransform& Transform) override;*/
#endif
};
