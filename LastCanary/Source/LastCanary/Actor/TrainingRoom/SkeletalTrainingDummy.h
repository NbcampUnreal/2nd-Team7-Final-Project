#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "DataType/TrainingType.h"
#include "SkeletalTrainingDummy.generated.h"

class UDamageReceiverComponent;
class UTrainingTargetComponent;

UCLASS()
class LASTCANARY_API ASkeletalTrainingDummy : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:	
	ASkeletalTrainingDummy();

    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------

    /** 루트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    /** 스켈레탈 메시 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* DummyMesh;

    /** 데미지 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDamageReceiverComponent* DamageReceiverComponent;

    /** 훈련 타겟 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTrainingTargetComponent* TrainingTargetComponent;

    //-----------------------------------------------------
    // 더미 설정
    //-----------------------------------------------------

    /** 이동 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
    float MovementSpeed = 100.0f;

    /** 순찰 경로 포인트들 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy", meta = (EditCondition = "DummyType == EDummyType::Patrol"))
    TArray<FVector> PatrolPoints;

    /** 랜덤 이동 반경 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy", meta = (EditCondition = "DummyType == EDummyType::Random", ClampMin = "0.0", ClampMax = "5000.0"))
    float RandomMovementRadius = 500.0f;

    /** 애니메이션 할당 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSubclassOf<UAnimInstance> AnimClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bUseAnimationBlueprint = false;

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

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

    /** 데미지 받을 때 호출 */
    UFUNCTION()
    void OnDamageReceived(AActor* DamagedActor, float DamageAmount, const FHitResult& HitInfo, AActor* DamageCauser);

    /** 체력 소진 시 호출 */
    UFUNCTION()
    void OnHealthDepleted(AActor* DamagedActor);

    /** 피격 효과 표시 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_ShowHitReaction(FVector HitLocation);
    void Multicast_ShowHitReaction_Implementation(FVector HitLocation);

private:
    /** 태그 컨테이너 */
    FGameplayTagContainer GameplayTags;

    /** 동적 머티리얼 인스턴스 */
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;
};
