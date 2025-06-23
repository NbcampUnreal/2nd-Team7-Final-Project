#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Item/ResourceItem/ResourceItemSpawnPoint.h"
#include "ResourceItemSpawnManager.generated.h"

class UItemSpawnerComponent;
class ANoteItem;
UCLASS()
class LASTCANARY_API AResourceItemSpawnManager : public AActor
{
	GENERATED_BODY()

public:
	AResourceItemSpawnManager();

protected:
	virtual void BeginPlay() override;

	//-------------------------------------------------------------------------
	// [1] 일반 스폰 설정
	//-------------------------------------------------------------------------
public:
	/** 시작 시 자동으로 스폰할지 여부 */
	UPROPERTY(EditAnywhere, Category = "Spawn|Behavior")
	bool bSpawnOnBeginPlay = true;

	/** 스폰 간격 (지연 스폰용) */
	UPROPERTY(EditAnywhere, Category = "Spawn|Delay")
	float ItemSpawnInterval = 0.01f;

	/** 몇 개의 리소스 노드를 스폰할지 */
	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 SpawnCount = 10;

	/** 스폰 타겟 포인트 태그 */
	UPROPERTY(EditAnywhere, Category = "Spawning")
	FName TargetPointTag = FName("ResourceSpawn");

	/** 현재 맵 지역 태그 */
	UPROPERTY(EditAnywhere, Category = "Spawn|Region", Meta = (Categories = "ItemSpawn.Map"))
	FGameplayTagContainer CurrentRegionTags;

	/** 맵 이름 기반으로 자동 태그 설정 */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SetCurrentRegionTag(const FString& MapName);

protected:
	void SetCurrentMapRegionTag();

	//-------------------------------------------------------------------------
	// [2] 리소스 아이템 스폰 관련
	//-------------------------------------------------------------------------
public:
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnResourceItemsForTheme();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnResourceItemByRow(FName ItemRowName);

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	AItemBase* SpawnItemAtLocation(FName ItemRowName, FVector Location);

	/** 현재 지역에서 스폰 가능한 아이템 필터링 */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	TArray<FName> GetSpawnableResourceItemsByTags() const;

protected:
	void SpawnNextPoint();

	UPROPERTY()
	int32 CurrentSpawnIndex = 0;

	UPROPERTY()
	TArray<AResourceItemSpawnPoint*> PendingSpawnPoints;

	UPROPERTY()
	TArray<FName> CachedSpawnableResourceItems;

	FTimerHandle ResourceSpawnDelayTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UItemSpawnerComponent* ItemSpawnerComponent;

	//-------------------------------------------------------------------------
	// [3] 리소스 노드 스폰
	//-------------------------------------------------------------------------
public:
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void  SpawnResourceNodes(int32 Count);

	UPROPERTY(EditAnywhere, Category = "Spawn")
	UDataTable* ResourceNodeSpawnTable;

	//-------------------------------------------------------------------------
	// [4] 쪽지 아이템 스폰
	//-------------------------------------------------------------------------
public:
	void SpawnNoteItems(int32 Count);
	void SpawnNoteItemsAtLocation(int32 Count, const FVector& Location);
	TArray<FName> GetSpawnableNotesByTags() const;

	UPROPERTY(EditDefaultsOnly, Category = "Note")
	UDataTable* NoteItemDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Note")
	TSubclassOf<ANoteItem> NoteItemClass;

	//-------------------------------------------------------------------------
	// [5] 낮밤에 따른 아이템 반응
	//-------------------------------------------------------------------------
protected:
	UFUNCTION()
	void OnDayNightChanged(EDayPhase NewPhase);
};