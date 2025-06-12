#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Item/ResourceItem/ResourceItemSpawnPoint.h"
#include "ResourceItemSpawnManager.generated.h"

class UItemSpawnerComponent;

UCLASS()
class LASTCANARY_API AResourceItemSpawnManager : public AActor
{
	GENERATED_BODY()
	
public:
    AResourceItemSpawnManager();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnDayNightChanged(EDayPhase NewPhase);

    UPROPERTY()
    int32 CurrentSpawnIndex = 0;

    UPROPERTY()
    TArray<AResourceItemSpawnPoint*> PendingSpawnPoints;

    UPROPERTY()
    TArray<FName> CachedSpawnableItems;

    FTimerHandle SpawnDelayTimer;
public:
    /** 시작 시 자동으로 스폰할지 여부 */
    UPROPERTY(EditAnywhere, Category = "Spawn|Behavior")
    bool bSpawnOnBeginPlay = true;

    UPROPERTY(EditAnywhere, Category = "Spawn|Delay")
    float ItemSpawnInterval = 0.01f;

    /** 현재 테마에 따라 아이템 스폰 */
    UFUNCTION(BlueprintCallable, Category = "Spawn")
    void SpawnItemsForTheme();
    void SpawnNextPoint();
    void SpawnItemByRow(FName ItemRowName);

    UPROPERTY(EditAnywhere, Category = "Spawn|Region", Meta = (Categories = "ItemSpawn.Map"))
    FGameplayTagContainer CurrentRegionTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UItemSpawnerComponent* ItemSpawnerComponent;

    UFUNCTION(BlueprintCallable, Category = "Spawn")
    TArray<FName> GetSpawnableItemsByTags() const;

    /** 현재 맵의 지역 태그 설정 */
    UFUNCTION(BlueprintCallable, Category = "Spawn")
    void SetCurrentRegionTag(const FString& MapName);

    /** 자원 생산 노드에서 요청 시 생성 */
    UFUNCTION(BlueprintCallable, Category = "Spawn")
    AItemBase* SpawnItemAtLocation(FName ItemRowName, FVector Location);

    UPROPERTY(EditAnywhere, Category = "Spawning")
    FName TargetPointTag = FName("ResourceSpawn");

    UPROPERTY(EditAnywhere, Category = "Spawn")
    UDataTable* ResourceNodeSpawnTable;

    UFUNCTION(BlueprintCallable, Category = "Spawn")
    void  SpawnResourceNodes(int32 Count);

    // 몇 개 스폰할지
    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 SpawnCount = 10;
protected:
    /** 현재 맵에 따른 지역 태그 자동 설정 */
    void SetCurrentMapRegionTag();
};
