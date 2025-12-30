#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataType/TrainingType.h"
#include "GameplayTagAssetInterface.h"
#include "TrainingDummy.generated.h"

UCLASS()
class LASTCANARY_API ATrainingDummy : public AActor
{
    GENERATED_BODY()

public:
    ATrainingDummy();

    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------

    /** 더미 메시 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* DummyMesh;

    /** 데미지 받는 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UDamageReceiverComponent* DamageReceiverComponent;

    /** 루트 컴포넌트 (씬) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    //-----------------------------------------------------
    // 더미 설정
    //-----------------------------------------------------

    /** 더미 타입 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
    EDummyType DummyType = EDummyType::Static;

    /** 이동 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy", meta = (EditCondition = "DummyType != EDummyType::Static", ClampMin = "0.0", ClampMax = "1000.0"))
    float MovementSpeed = 100.0f;

    /** 순찰 경로 포인트들 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy", meta = (EditCondition = "DummyType == EDummyType::Patrol"))
    TArray<FVector> PatrolPoints;

    /** 랜덤 이동 반경 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy", meta = (EditCondition = "DummyType == EDummyType::Random", ClampMin = "0.0", ClampMax = "5000.0"))
    float RandomMovementRadius = 500.0f;

    /** 피격 시 반응 (색상 변화 등) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
    bool bShowHitReaction = true;

    /** 더미 색상 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
    FLinearColor DummyColor = FLinearColor::White;

    //-----------------------------------------------------
    // 상태
    //-----------------------------------------------------

    /** 연결된 훈련장 매니저 */
    UPROPERTY()
    class ATrainingRoomManager* TrainingManager;

    /** 현재 활성화 상태 */
    UPROPERTY(BlueprintReadOnly, Replicated, Category = "Dummy")
    bool bIsActive = true;

    /** 현재 순찰 인덱스 */
    UPROPERTY()
    int32 CurrentPatrolIndex = 0;

    /** 랜덤 이동 목표 지점 */
    UPROPERTY()
    FVector RandomTargetLocation;

    /** 랜덤 이동 타이머 */
    FTimerHandle RandomMovementTimerHandle;

    //-----------------------------------------------------
    // 공개 함수
    //-----------------------------------------------------

    /** 더미 활성화/비활성화 */
    UFUNCTION(BlueprintCallable, Category = "Dummy")
    void SetActive(bool bNewActive);

    /** 더미 리셋 */
    UFUNCTION(BlueprintCallable, Category = "Dummy")
    void ResetDummy();

    /** 매니저 설정 */
    UFUNCTION(BlueprintCallable, Category = "Dummy")
    void SetTrainingManager(ATrainingRoomManager* Manager);

    /** 더미 위치 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Dummy")
    void UpdateInitialLocation(const FVector& NewLocation);

    //-----------------------------------------------------
    // 게임플레이 태그 인터페이스
    //-----------------------------------------------------

    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 데미지 받을 때 호출 */
    UFUNCTION()
    void OnDamageReceived(AActor* DamagedActor, float DamageAmount, const FHitResult& HitInfo, AActor* DamageCauser);

    /** 체력 소진 시 호출 */
    UFUNCTION()
    void OnHealthDepleted(AActor* DamagedActor);

    /** 이동 로직 */
    void HandleMovement(float DeltaTime);

    /** 순찰 이동 */
    void HandlePatrolMovement(float DeltaTime);

    /** 랜덤 이동 */
    void HandleRandomMovement(float DeltaTime);

    /** 새로운 랜덤 목표 설정 */
    void SetNewRandomTarget();

    /** 피격 효과 표시 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_ShowHitReaction(FVector HitLocation);
    void Multicast_ShowHitReaction_Implementation(FVector HitLocation);

private:
    /** 게임플레이 태그 컨테이너 */
    FGameplayTagContainer GameplayTags;

    /** 동적 머티리얼 인스턴스 */
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;

    /** 초기 위치 저장 */
    FVector InitialLocation;
};
