#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerChecker.generated.h"

class UBoxComponent;


UCLASS()
class LASTCANARY_API APlayerChecker : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerChecker();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ReadyZone")
	UBoxComponent* TriggerVolume;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

};
