#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
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
public:
    /** 테마별로 스폰할 아이템 목록 */
    TMap<FName, TArray<FName>> ThemeItemMap;

    /** 현재 적용할 테마 (예: "Cave", "Forest") */
    UPROPERTY(EditAnywhere, Category = "Spawn|Theme")
    FName CurrentThemeTag;

    /** 시작 시 자동으로 스폰할지 여부 */
    UPROPERTY(EditAnywhere, Category = "Spawn|Behavior")
    bool bSpawnOnBeginPlay = true;

    /** 현재 테마에 따라 아이템 스폰 */
    UFUNCTION(BlueprintCallable, Category = "Spawn")
    void SpawnItemsForTheme();

    /** 테마 아이템 초기화 */
    void InitializeThemeItemMap();

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

protected:
    /** 현재 맵에 따른 지역 태그 자동 설정 */
    void SetCurrentMapRegionTag();
};
