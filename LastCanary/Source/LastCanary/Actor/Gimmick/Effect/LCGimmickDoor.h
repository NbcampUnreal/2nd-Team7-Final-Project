#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/GimmickEffectInterface.h"
#include "LCGimmickDoor.generated.h"

UCLASS()
class LASTCANARY_API ALCGimmickDoor : public AActor, public IGimmickEffectInterface
{
	GENERATED_BODY()
	
public:
    ALCGimmickDoor();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* DoorMesh;

    UPROPERTY(EditAnywhere, Category = "Door")
    FRotator OpenRotation = FRotator(0.f, 90.f, 0.f); // 열린 방향

    UPROPERTY(EditAnywhere, Category = "Door")
    bool bSlideInstead = false; // 슬라이딩 방식으로도 대응 가능

    UPROPERTY(EditAnywhere, Category = "Door")
    FVector SlideOffset = FVector(0.f, 0.f, 200.f);

    UPROPERTY(VisibleInstanceOnly)
    bool bIsOpen = false;

public:
    virtual void TriggerEffect_Implementation() override;
};