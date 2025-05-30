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

class AResourceItemBase;
UCLASS()
class LASTCANARY_API AResourceItemSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
    AResourceItemSpawnPoint();

protected:
    virtual void BeginPlay() override;

    bool IsLocationOccupied() const;

public:
    UPROPERTY(EditAnywhere, Category = "Spawn|Condition")
    ESpawnTimeCondition SpawnTimeCondition = ESpawnTimeCondition::Always;

    UPROPERTY()
    AResourceItemBase* SpawnedResourceItem;

    /** 스폰 가능한 아이템들의 RowName 목록 */
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<FName> PossibleItems;

    /** 스폰 확률 (0.0 ~ 1.0) */
    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnProbability = 1.0f;

    /** 아이템 스폰용 클래스 */
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<class AResourceItemBase> ItemClass;

    /** 아이템을 스폰 시도 */
    UFUNCTION(BlueprintCallable)
    void TrySpawnItem();

    /** 외부 매니저가 직접 조건 확인 후 강제 스폰할 수도 있음 */
    void SpawnItemByRow(FName ItemRowName);
};