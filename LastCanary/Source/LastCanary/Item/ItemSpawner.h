#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Item/ItemBase.h"
#include "ItemSpawner.generated.h"

USTRUCT(BlueprintType)
struct FSpawnableItemInfo
{
    GENERATED_BODY()

    /** 스폰할 아이템 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSubclassOf<AItemBase> ItemClass;

    /** 아이템 데이터 테이블 행 이름 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemRowName;

    /** 스폰 확률 (0.0 ~ 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpawnProbability = 1.0f;

    /** 최소 수량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 MinQuantity = 1;

    /** 최대 수량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 MaxQuantity = 1;

    /** 내구도 범위 (최소) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MinDurability = 100.0f;

    /** 내구도 범위 (최대) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxDurability = 100.0f;

    FSpawnableItemInfo()
    {
        ItemClass = nullptr;
        ItemRowName = NAME_None;
        SpawnProbability = 1.0f;
        MinQuantity = 1;
        MaxQuantity = 1;
        MinDurability = 100.0f;
        MaxDurability = 100.0f;
    }
};

UENUM(BlueprintType)
enum class ESpawnerType : uint8
{
    /** 한 번만 스폰 */
    OneTime         UMETA(DisplayName = "One Time"),
    /** 주기적 스폰 */
    Periodic        UMETA(DisplayName = "Periodic"),
    /** 트리거 기반 스폰 */
    Trigger         UMETA(DisplayName = "Trigger"),
    /** 플레이어 근접 시 스폰 */
    Proximity       UMETA(DisplayName = "Proximity")
};

UCLASS()
class LASTCANARY_API AItemSpawner : public AActor
{
    GENERATED_BODY()

public:
    AItemSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------

    /** 스포너의 메시 컴포넌트 (시각적 표시용) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SpawnerMesh;

    /** 근접 감지용 구체 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* ProximityComponent;

    //-----------------------------------------------------
    // 스포너 설정
    //-----------------------------------------------------

    /** 스포너 타입 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
    ESpawnerType SpawnerType = ESpawnerType::OneTime;

    /** 스폰 가능한 아이템 목록 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
    TArray<FSpawnableItemInfo> SpawnableItems;

    /** 스폰 간격 (Periodic 타입일 때) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings",
        meta = (EditCondition = "SpawnerType == ESpawnerType::Periodic", ClampMin = "0.1"))
    float SpawnInterval = 10.0f;

    /** 최대 스폰 개수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings", meta = (ClampMin = "1"))
    int32 MaxSpawnCount = 1;

    /** 스폰 위치 랜덤 반경 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings", meta = (ClampMin = "0.0"))
    float SpawnRadius = 100.0f;

    /** 근접 감지 거리 (Proximity 타입일 때) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings",
        meta = (EditCondition = "SpawnerType == ESpawnerType::Proximity", ClampMin = "50.0"))
    float ProximityDistance = 500.0f;

    /** 게임 시작 시 자동 스폰 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
    bool bAutoSpawnOnStart = true;

    /** 스폰 시 스포너 숨기기 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Settings")
    bool bHideSpawnerAfterSpawn = false;

    //-----------------------------------------------------
    // 스폰 기능
    //-----------------------------------------------------

    /** 아이템 스폰 실행 */
    UFUNCTION(BlueprintCallable, Category = "Spawner|Actions")
    void SpawnItems();

    /** 특정 아이템 스폰 */
    UFUNCTION(BlueprintCallable, Category = "Spawner|Actions")
    AItemBase* SpawnSpecificItem(const FSpawnableItemInfo& ItemInfo);

    /** 스포너 리셋 */
    UFUNCTION(BlueprintCallable, Category = "Spawner|Actions")
    void ResetSpawner();

    /** 스포너 활성화/비활성화 */
    UFUNCTION(BlueprintCallable, Category = "Spawner|Actions")
    void SetSpawnerEnabled(bool bEnabled);

    //-----------------------------------------------------
    // 이벤트
    //-----------------------------------------------------

    /** 아이템 스폰 시 호출되는 이벤트 */
    UFUNCTION(BlueprintImplementableEvent, Category = "Spawner|Events")
    void OnItemSpawned(AItemBase* SpawnedItem);

    /** 스포너가 비활성화될 때 호출되는 이벤트 */
    UFUNCTION(BlueprintImplementableEvent, Category = "Spawner|Events")
    void OnSpawnerDeactivated();

private:
    //-----------------------------------------------------
    // 내부 상태
    //-----------------------------------------------------

    /** 현재 스폰된 아이템 개수 */
    int32 CurrentSpawnCount;

    /** 스포너 활성화 상태 */
    bool bIsEnabled;

    /** 마지막 스폰 시간 */
    float LastSpawnTime;

    /** 타이머 핸들 */
    FTimerHandle SpawnTimerHandle;

    /** 플레이어가 근접해 있는지 여부 */
    bool bPlayerInProximity;

    //-----------------------------------------------------
    // 내부 함수
    //-----------------------------------------------------

    /** 주기적 스폰 처리 */
    void HandlePeriodicSpawn();

    /** 근접 스폰 처리 */
    void HandleProximitySpawn();

    /** 랜덤 스폰 위치 계산 */
    FVector GetRandomSpawnLocation();

    /** 아이템 선택 (확률 기반) */
    FSpawnableItemInfo* SelectRandomItem();

    /** 오버랩 이벤트 */
    UFUNCTION()
    void OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

#if WITH_EDITOR
    /** 에디터에서 속성 변경 시 호출 */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    private:
        /** 지연된 아이템 속성 적용 */
        void ApplyDelayedItemProperties(AItemBase* Item, const FSpawnableItemInfo& ItemInfo);
};
