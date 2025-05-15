// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataTable/ItemDataRow.h"
#include "ItemBase.generated.h"

UCLASS()
class LASTCANARY_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

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

};
