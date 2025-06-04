#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemBase.h"
#include "DataType/BaseItemSlotData.h"
#include "ItemSpawnerComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTCANARY_API UItemSpawnerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UItemSpawnerComponent();

    //-----------------------------------------------------
    // 아이템 생성 함수들
    //-----------------------------------------------------

    /** 새 아이템 생성 (기본값 사용) */
    UFUNCTION(BlueprintCallable, Category = "Item Spawner")
    AItemBase* CreateItem(FName ItemRowName, const FVector& SpawnLocation);

    /** 기존 아이템 데이터로 복사본 생성 (드랍용) */
    UFUNCTION(BlueprintCallable, Category = "Item Spawner")
    AItemBase* CreateItemFromData(const FBaseItemSlotData& SourceItemData, const FVector& SpawnLocation);

    /** 커스텀 수량/내구도로 아이템 생성 */
    UFUNCTION(BlueprintCallable, Category = "Item Spawner")
    AItemBase* CreateItemWithCustomData(FName ItemRowName, const FVector& SpawnLocation, int32 Quantity, float Durability);

protected:
    //-----------------------------------------------------
    // 스폰 설정값들 (컴포넌트에서 관리)
    //-----------------------------------------------------

    /** 드랍 시 적용할 물리 임펄스 강도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float DropImpulseStrength = 300.0f;

    /** 임펄스 랜덤 범위 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    FVector ImpulseRandomRange = FVector(300.0f, 300.0f, 500.0f);

    /** 기본 수량 (새 아이템 생성 시) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    int32 DefaultQuantity = 1;

    /** 기본 내구도 (새 아이템 생성 시) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
    float DefaultDurability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
    float DropDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
    float DropHeightOffset = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
    float DropSpreadRadius = 50.0f;

public:
    /** 드랍 설정 접근자들 */
    UFUNCTION(BlueprintPure, Category = "Drop Settings")
    float GetDropDistance() const { return DropDistance; }

    UFUNCTION(BlueprintPure, Category = "Drop Settings")
    float GetDropHeightOffset() const { return DropHeightOffset; }

    UFUNCTION(BlueprintPure, Category = "Drop Settings")
    float GetDropSpreadRadius() const { return DropSpreadRadius; }

private:
    /** 캐시된 게임 서브시스템 */
    UPROPERTY()
    class ULCGameInstanceSubsystem* CachedGameSubsystem;

    /** 게임 서브시스템 가져오기 */
    ULCGameInstanceSubsystem* GetGameSubsystem();

    /** 아이템에 기본 설정 적용 */
    void ApplyItemSettings(AItemBase* Item, FName ItemRowName, int32 Quantity, float Durability);

    /** 물리 시뮬레이션 활성화 */
    void EnablePhysicsSimulation(AItemBase* Item);

protected:
    /** 던지기 기본 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw Settings")
    float BaseThrowVelocity = 300.0f;

    /** 던지기 각도 (위쪽 방향) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw Settings")
    float ThrowAngleDegrees = 25.0f;

    /** 무게 기반 속도 감소 계수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw Settings")
    float WeightMultiplier = 0.1f;

    /** 최소 던지기 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw Settings")
    float MinThrowVelocity = 200.0f;

    /** 최대 던지기 속도 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw Settings")
    float MaxThrowVelocity = 1200.0f;

public:
    /** 무게를 고려한 던지기 속도 계산 */
    UFUNCTION(BlueprintPure, Category = "Throw Settings")
    float CalculateThrowVelocity(float ItemWeight) const;

    /** 던지기 방향 벡터 계산 */
    UFUNCTION(BlueprintPure, Category = "Throw Settings")
    FVector CalculateThrowDirection() const;

protected:
    /** 메시 물리 설정 */
    void SetupMeshPhysics(UPrimitiveComponent* MeshComponent, const FVector& ThrowDirection, float ThrowVelocity, const FVector& ThrowImpulse);
};
