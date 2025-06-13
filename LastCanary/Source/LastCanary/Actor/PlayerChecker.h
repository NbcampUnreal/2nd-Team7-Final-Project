#pragma once

#include "CoreMinimal.h"5
#include "GameFramework/Actor.h"
#include "PlayerChecker.generated.h"

class UBoxComponent;
class UTimelineComponent;
class UCurveFloat;
class UStaticMeshComponent;

UCLASS()
class LASTCANARY_API APlayerChecker : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerChecker();

    void InitializeChecker();

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

protected:
	// 문 관련 변수 및 함수
    UPROPERTY(VisibleAnywhere, Category = "Door")
	UStaticMeshComponent* LeftDoorMesh;
    UPROPERTY(VisibleAnywhere, Category = "Door")
	UStaticMeshComponent* RightDoorMesh;

    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    UCurveFloat* DoorOpenCurve;

    UPROPERTY()
    UTimelineComponent* DoorTimeline;

    UPROPERTY()
    FRotator InitialLeftRotation;

    UPROPERTY()
    FRotator InitialRightRotation;

    UPROPERTY(ReplicatedUsing = OnRep_DoorState)
    bool bIsDoorOpen;

	UFUNCTION()
    void OnRep_DoorState();

    UPROPERTY(EditAnywhere, Category = "Door|Animation")
    float DoorOpenAngle = 90.f;

    UFUNCTION()
    void HandleDoorProgress(float Value);

    UFUNCTION()
    void OnTimelineFinished();

public:
    // Server 호출 → 클라이언트에 전파
    UFUNCTION(Server, Reliable)
    void Server_OpenDoors();
    void Server_OpenDoors_Implementation();

    UFUNCTION(Server, Reliable)
    void Server_CloseDoors();
	void Server_CloseDoors_Implementation();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OpenDoors();
    void Multicast_OpenDoors_Implementation();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_CloseDoors();
	void Multicast_CloseDoors_Implementation();

    void PlayDoorTimelineForward();   // 열기
    void PlayDoorTimelineReverse();   // 닫기

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
};
