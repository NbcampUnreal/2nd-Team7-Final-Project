#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataType/TrainingType.h"
#include "TrainingTargetComponent.generated.h"

class ATrainingRoomManager;
class UDamageReceiverComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UTrainingTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTrainingTargetComponent();


    /** 현재 타겟의 더미 타입 (이동/정지 등) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Training")
    EDummyType DummyType = EDummyType::Static;

    /** 연결된 훈련장 매니저 */
    UPROPERTY(BlueprintReadOnly, Category = "Training")
    ATrainingRoomManager* TrainingManager = nullptr;

    /** 타겟 활성/비활성 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void SetTargetEnabled(bool bEnabled);

    /** 타겟 리셋 (체력, 위치 등) */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void ResetTarget();

    /** 매니저 설정 */
    UFUNCTION(BlueprintCallable, Category = "Training")
    void SetTrainingManager(ATrainingRoomManager* NewManager);
    
    UFUNCTION(BlueprintCallable, Category = "Training")
    void UpdateInitialLocation(const FVector& NewLocation);

protected:
    virtual void BeginPlay() override;

    /** 데미지 수신시 호출 */
    UFUNCTION()
    void OnDamageReceived(AActor* DamagedActor, float DamageAmount, const FHitResult& HitInfo, AActor* DamageCauser);

    /** 체력 0 시 호출 */
    UFUNCTION()
    void OnHealthDepleted(AActor* DamagedActor);

private:
    /** 캐시된 데미지 컴포넌트 */
    UPROPERTY()
    UDamageReceiverComponent* DamageReceiver = nullptr;

    /** 초기 위치 저장 */
    FVector InitialLocation;

    /** 내부 초기화 */
    void InitializeInternal();
};
