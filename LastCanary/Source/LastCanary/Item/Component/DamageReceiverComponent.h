#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "Engine/DamageEvents.h"
#include "DamageReceiverComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageReceived, AActor*, DamagedActor, float, DamageAmount, const FHitResult&, HitInfo, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthDepleted, AActor*, DamagedActor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UDamageReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDamageReceiverComponent();

    //-----------------------------------------------------
    // 체력 시스템
    //-----------------------------------------------------

    /** 최대 체력 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.1"))
    float MaxHealth = 100.0f;

    /** 현재 체력 (복제됨) */
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
    float CurrentHealth = 100.0f;

    /** 체력이 0이 되면 액터를 파괴할지 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bDestroyOnHealthDepleted = true;

    /** 파괴 지연 타이머 핸들 */
    FTimerHandle DestructionTimerHandle;

    /** 파괴 지연 시간 (초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (EditCondition = "bDestroyOnHealthDepleted"))
    float DestructionDelay = 0.0f;

    float DamageMultiplier = 1.0f;

    //-----------------------------------------------------
    // 게임플레이 태그 설정
    //-----------------------------------------------------

    /** 이 컴포넌트가 가지는 게임플레이 태그들 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    FGameplayTagContainer OwnedGameplayTags;

    /** 자동으로 Enemy 태그 추가 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags")
    bool bAutoAddEnemyTag = true;

    /** 머티리얼 태그 (피격 사운드용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayTags", meta = (Categories = "Material"))
    FGameplayTag MaterialTag;

    //-----------------------------------------------------
    // 충돌 설정
    //-----------------------------------------------------

    /** 대상 메시 컴포넌트 (지정하지 않으면 오너의 첫 번째 메시 사용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TWeakObjectPtr<UMeshComponent> TargetMeshComponent;

    /** ECC_Visibility에 대한 충돌 응답을 Block으로 자동 설정할지 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bAutoSetupCollision = true;

    //-----------------------------------------------------
    // 이벤트 델리게이트
    //-----------------------------------------------------

    /** 데미지를 받았을 때 호출되는 델리게이트 */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDamageReceived OnDamageReceived;

    /** 체력이 0이 되었을 때 호출되는 델리게이트 */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHealthDepleted OnHealthDepleted;

    //-----------------------------------------------------
    // 공개 함수
    //-----------------------------------------------------

    /** 체력 설정 */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float NewHealth);

    /** 체력 회복 */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    /** 현재 체력 반환 */
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    /** 최대 체력 반환 */
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetMaxHealth() const { return MaxHealth; }

    /** 체력 비율 반환 (0.0 ~ 1.0) */
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthRatio() const;

    /** 게임플레이 태그 추가 */
    UFUNCTION(BlueprintCallable, Category = "GameplayTags")
    void AddGameplayTag(FGameplayTag TagToAdd);

    /** 게임플레이 태그 제거 */
    UFUNCTION(BlueprintCallable, Category = "GameplayTags")
    void RemoveGameplayTag(FGameplayTag TagToRemove);

    /** 특정 태그 보유 여부 확인 */
    UFUNCTION(BlueprintPure, Category = "GameplayTags")
    bool HasGameplayTag(FGameplayTag TagToCheck) const;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 체력 복제 함수 */
    UFUNCTION()
    void OnRep_CurrentHealth();

    /** 오너 액터의 TakeDamage와 연결 */
    void SetupOwnerDamageBinding();

    /** 충돌 설정 자동 구성 */
    void SetupCollisionSettings();

    /** 메시 컴포넌트 찾기 */
    UMeshComponent* FindTargetMeshComponent();

    /** 체력 소진 처리 */
    UFUNCTION()
    void HandleHealthDepleted();

    /** 파괴 실행 (지연 후 호출) */
    UFUNCTION()
    void ExecuteDestruction();

public:
    /** 데미지 처리 함수 (오너 액터의 TakeDamage에서 호출) */
    UFUNCTION(BlueprintCallable, Category = "Damage")
    float HandleDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

    //-----------------------------------------------------
    // 헤드샷 설정
    //-----------------------------------------------------

public:
    /** 헤드샷 데미지 배율 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float HeadshotMultiplier = 2.0f;

    /** 헤드샷으로 인식할 본 이름들 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    TArray<FName> HeadshotBoneNames;
};
