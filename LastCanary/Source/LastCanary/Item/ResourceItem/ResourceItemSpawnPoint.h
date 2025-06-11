#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceItemSpawnPoint.generated.h"

UENUM(BlueprintType)
enum class ESpawnTimeCondition : uint8
{
    Always      UMETA(DisplayName = "Always"),       // 항상 스폰
    DayOnly     UMETA(DisplayName = "Day Only"),     // 낮에만 스폰됨
    NightOnly   UMETA(DisplayName = "Night Only")    // 밤에만 스폰됨
};

class AItemBase;
class UItemSpawnerComponent;
UCLASS()
class LASTCANARY_API AResourceItemSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
    AResourceItemSpawnPoint();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, Category = "Spawn|Condition")
    ESpawnTimeCondition SpawnTimeCondition = ESpawnTimeCondition::Always;

    UPROPERTY()
    AItemBase* SpawnedResourceItem;

    /** 스폰 가능한 아이템들의 RowName 목록 */
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<FName> PossibleItems;

    /** 스폰 확률 (0.0 ~ 1.0) */
    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnProbability = 1.0f;

    /** 아이템 스폰용 클래스 */
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<AItemBase> ItemClass;

    /** 외부 매니저가 직접 조건 확인 후 강제 스폰할 수도 있음 */
    void SpawnItemByRow(FName ItemRowName);

    /** Always 조건으로 이미 스폰했는지 여부 */
    UPROPERTY()
    bool bHasSpawnedAlwaysItem = false;

    /** 매니저에서 호출하는 스폰 함수 */
    UFUNCTION(BlueprintCallable)
    void TrySpawnItemFromManager(const TArray<FName>& AvailableItems, UItemSpawnerComponent* SpawnerComponent);

    /** 현재 스폰된 아이템 제거 */
    UFUNCTION(BlueprintCallable)
    void ClearSpawnedItem();

    /** 시간 조건 체크 */
    bool CheckTimeCondition() const;

    /** 시간 조건에 맞는 아이템 필터링 */
    TArray<FName> FilterItemsByTimeCondition(const TArray<FName>& AvailableItems) const;
};
