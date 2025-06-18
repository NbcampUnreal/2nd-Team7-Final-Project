#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "AttachedSyncComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTCANARY_API UAttachedSyncComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UAttachedSyncComponent();

protected:
	virtual void BeginPlay() override;

	/** 오버랩 시작 시 처리 */
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/** 오버랩 종료 시 처리 */
	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

public:
	/** 감지 대상 액터 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sync")
	FName TargetActorTag;

	/** 현재 감지된 액터 목록 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sync")
	TArray<AActor*> AttachedActors;

	/** 현재 감지된 액터 목록 반환 */
	const TArray<AActor*>& GetAttachedActors() const;

	/** 부모 이동 시작 시 동기화 호출 */
	void BroadcastStartMovement(const FVector& From, const FVector& To, float Duration);

	/** 부모 회전 시작 시 동기화 호출 */
	void BroadcastStartRotation(const FQuat& From, const FQuat& To, float Duration);
};