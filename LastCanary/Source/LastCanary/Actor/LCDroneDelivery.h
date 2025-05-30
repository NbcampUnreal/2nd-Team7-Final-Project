#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataType/ItemDropData.h"
#include "LCDroneDelivery.generated.h"

class USplineComponent;
class ALCDronePath;
class UStaticMeshComponent;
UCLASS()
class LASTCANARY_API ALCDroneDelivery : public AActor
{
	GENERATED_BODY()
	
public:
	ALCDroneDelivery();

	/** 클라이언트에서 호출해 서버에 전달 */
	UFUNCTION(BlueprintCallable)
	void StartDelivery();

    ALCDronePath* FindNearestDronePath();
	
	void SetDropItems(const TArray<FItemDropData>& InItems) { ItemsToDrop = InItems; }
    
    UFUNCTION(BlueprintCallable)
    const TArray<FItemDropData>& GetDropItems() const { return ItemsToDrop; }
    
    UPROPERTY(EditAnywhere, Category = "Drop")
    TArray<FItemDropData> ItemsToDrop;

    UFUNCTION(BlueprintCallable, Category = "Drone")
    ALCDronePath* GetDronePath() const { return DronePathActor; }

protected:
	virtual void BeginPlay() override;

	/** 서버에서 호출되는 함수 */
	UFUNCTION(Server, Reliable)
	void Server_StartDeliveryWithPath(ALCDronePath* InDronePathActor);
	void Server_StartDeliveryWithPath_Implementation(ALCDronePath* InDronePathActor);

	/** 실제 이동 시작 (서버 전용) */
	void BeginDelivery();

	/** 드랍 연출 (모든 클라이언트) */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayDropEffect();
	void Multicast_PlayDropEffect_Implementation();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayDropExplosionEffect();
    void Multicast_PlayDropExplosionEffect_Implementation();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_DropBoxDetach();
    void Multicast_DropBoxDetach_Implementation();

    UFUNCTION()
    void OnDropBoxHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void SpawnDroppedItems();

private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* DroneMesh;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* DropBoxMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float MoveDuration = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float UpdateInterval = 0.05f;

    UPROPERTY(EditDefaultsOnly, Category = "Escape")
    float EscapeDistance = 1500.f;

    UPROPERTY(EditDefaultsOnly, Category = "Escape")
    float EscapeSpeed = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "Escape")
    float EscapeUpdateInterval = 0.05f;

    UPROPERTY(EditDefaultsOnly, Category = "Escape")
    float EscapeDelay = 1.0f;

    UPROPERTY()
    USplineComponent* ExternalSpline;

    UPROPERTY()
    ALCDronePath* DronePathActor;

    FTimerHandle MoveTimerHandle;
    float ElapsedTime = 0.0f;

    void UpdateLocationOnSpline();
    void DropBox();
    void FlyAwayAndDie();
    void UpdateEscapeMovement();

    FTimerHandle EscapeMoveTimerHandle;
    FTimerHandle DestroyTimerHandle;
    float EscapeAlpha = 0.f;
    FVector StartLocation;
    FVector TargetLocation;
};