// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoxfireActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class LASTCANARY_API AFoxfireActor : public AActor
{
	GENERATED_BODY()
	
public:
    AFoxfireActor();

protected:
    virtual void BeginPlay() override;

    // 데미지 범위용 콜리전
    UPROPERTY(VisibleAnywhere, Category = "Damage")
    USphereComponent* DamageSphere;

    // 비주얼용 메쉬
    UPROPERTY(VisibleAnywhere, Category = "Visual")
    UStaticMeshComponent* MeshComp;

    // 입장 시 한 번만 데미지 주도록
    UPROPERTY(EditAnywhere, Category = "Damage")
    float DamageAmount = 10.f;

    // Overlap 콜백
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

};
