#include "Item/ResourceItem/ResourceItemSpawnManager.h"
#include "Item/ItemBase.h"
#include "Item/ItemSpawnerComponent.h"
#include "Item/ResourceNode.h"
#include "Item/NoteItem.h"
#include "DataTable/ResourceNodeSpawnRow.h"
#include "Engine/TargetPoint.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Manager/LCTimeManager.h"
#include "LastCanary.h"

//--------------------------------------------------------------------------------
// [1] 생성자 & BeginPlay
//--------------------------------------------------------------------------------

AResourceItemSpawnManager::AResourceItemSpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bSpawnOnBeginPlay = true;

	ItemSpawnerComponent = CreateDefaultSubobject<UItemSpawnerComponent>(TEXT("ItemSpawnerComponent"));
}

void AResourceItemSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		// 현재 맵에 따른 지역 태그 설정
		SetCurrentMapRegionTag();

		if (ALCTimeManager* TimeManager = Cast<ALCTimeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ALCTimeManager::StaticClass())))
		{
			TimeManager->OnTimePhaseChanged.AddDynamic(this, &AResourceItemSpawnManager::OnDayNightChanged);
		}

		if (bSpawnOnBeginPlay)
		{
			SpawnResourceItemsForTheme();
			SpawnNoteItemsForTheme();
		}

		SpawnResourceNodes(SpawnCount);
	}
}

//--------------------------------------------------------------------------------
// [2] 지역 태그 설정
//--------------------------------------------------------------------------------

void AResourceItemSpawnManager::SetCurrentMapRegionTag()
{
	// 현재 맵 이름 가져오기
	FString CurrentMapName = GetWorld()->GetMapName();

	// 맵 이름을 기반으로 태그 설정
	if (CurrentMapName.Contains("Cave") || CurrentMapName.Contains("cave"))
	{
		CurrentRegionTags.AddTag(FGameplayTag::RequestGameplayTag("ItemSpawn.Map.Cave"));
	}
	else if (CurrentMapName.Contains("Forest") || CurrentMapName.Contains("forest"))
	{
		CurrentRegionTags.AddTag(FGameplayTag::RequestGameplayTag("ItemSpawn.Map.Forest"));
	}
	else if (CurrentMapName.Contains("Ruins") || CurrentMapName.Contains("ruins"))
	{
		CurrentRegionTags.AddTag(FGameplayTag::RequestGameplayTag("ItemSpawn.Map.Ruins"));
	}
}

void AResourceItemSpawnManager::SetCurrentRegionTag(const FString& MapName)
{
	CurrentRegionTags.Reset();

	FString TagName = FString::Printf(TEXT("ItemSpawn.Map.%s"), *MapName);
	FGameplayTag RegionTag = FGameplayTag::RequestGameplayTag(*TagName);

	if (RegionTag.IsValid())
	{
		CurrentRegionTags.AddTag(RegionTag);
		LOG_Item_WARNING(TEXT("[ResourceItemSpawnManager] Set region tag: %s"), *TagName);
	}
	else
	{
		LOG_Item_WARNING(TEXT("[ResourceItemSpawnManager] Invalid region tag: %s"), *TagName);
	}
}

//--------------------------------------------------------------------------------
// [3] 일반 아이템 스폰 관련
//--------------------------------------------------------------------------------

void AResourceItemSpawnManager::SpawnResourceItemsForTheme()
{
	// 태그 기반으로 스폰 가능한 아이템 목록 가져오기
	CachedSpawnableResourceItems = GetSpawnableResourceItemsByTags();

	if (CachedSpawnableResourceItems.Num() == 0)
	{
		LOG_Item_WARNING(TEXT("No spawnable items found for current region tags"));
		return;
	}

	// 스폰 포인트 수집
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResourceItemSpawnPoint::StaticClass(), FoundPoints);

	PendingSpawnPoints.Empty();
	for (AActor* Actor : FoundPoints)
	{
		if (AResourceItemSpawnPoint* Point = Cast<AResourceItemSpawnPoint>(Actor))
		{
			PendingSpawnPoints.Add(Point);
		}
	}

	// 지연 스폰 시작
	if (PendingSpawnPoints.Num() > 0)
	{
		CurrentSpawnIndex = 0;
		GetWorld()->GetTimerManager().SetTimer(
			ResourceSpawnDelayTimer,
			this,
			&AResourceItemSpawnManager::SpawnNextPoint,
			ItemSpawnInterval,
			true
		);
	}
}

void AResourceItemSpawnManager::SpawnNextPoint()
{
	if (CurrentSpawnIndex < PendingSpawnPoints.Num())
	{
		AResourceItemSpawnPoint* Point = PendingSpawnPoints[CurrentSpawnIndex];
		if (IsValid(Point))
		{
			Point->TrySpawnItemFromManager(CachedSpawnableResourceItems, ItemSpawnerComponent);
		}

		CurrentSpawnIndex++;
	}
	else
	{
		// 모든 스폰 완료, 타이머 정리
		GetWorld()->GetTimerManager().ClearTimer(ResourceSpawnDelayTimer);
		PendingSpawnPoints.Empty();
		CachedSpawnableResourceItems.Empty();
		CurrentSpawnIndex = 0;

		LOG_Item_WARNING(TEXT("[AResourceItemSpawnManager::SpawnNextPoint] All items spawned with delay completed"));
	}
}

void AResourceItemSpawnManager::SpawnResourceItemByRow(FName ItemRowName)
{
	if (ItemSpawnerComponent)
	{
		AItemBase* SpawnedItem = ItemSpawnerComponent->CreateItem(ItemRowName, GetActorLocation());
	}
}

AItemBase* AResourceItemSpawnManager::SpawnItemAtLocation(FName ItemRowName, FVector Location)
{
	if (ItemSpawnerComponent)
	{
		return ItemSpawnerComponent->CreateItemAtLocation(ItemRowName, Location);
	}

	LOG_Item_WARNING(TEXT("[AResourceItemSpawnManager::SpawnItemAtLocation] ItemSpawnerComponent is null"));
	return nullptr;
}

TArray<FName> AResourceItemSpawnManager::GetSpawnableResourceItemsByTags() const
{
	TArray<FName> SpawnableItems;

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] World is null!"));
		return SpawnableItems;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (GI == nullptr)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] GameInstance is null!"));
		return SpawnableItems;
	}

	ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
	if (GISubsystem == nullptr)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] LCGameInstance is null"));
		return SpawnableItems;
	}
	TArray<FName> AllItemNames = GISubsystem->ItemDataTable->GetRowNames();

	for (const FName& ItemName : AllItemNames)
	{
		if (FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(ItemName))
		{
			if (ItemData->AllowedSpawnMaps.IsEmpty())
			{
				continue;
			}

			bool bRegionMatch = ItemData->AllowedSpawnMaps.IsEmpty() || ItemData->AllowedSpawnMaps.HasAnyExact(CurrentRegionTags);

			if (bRegionMatch)
			{
				SpawnableItems.Add(ItemName);
			}
		}
	}

	return SpawnableItems;
}

//--------------------------------------------------------------------------------
// [4] 노트 아이템 스폰
//--------------------------------------------------------------------------------

TArray<FName> AResourceItemSpawnManager::GetSpawnableNotesByTags() const
{
	TArray<FName> NoteItems;

	if (GetWorld() == nullptr)
	{
		return NoteItems;
	}
	if (const ULCGameInstanceSubsystem* GISubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		TArray<FName> AllItemNames = GISubsystem->ItemDataTable->GetRowNames();

		for (const FName& RowName : AllItemNames)
		{
			if (const FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(RowName))
			{
				if (ItemData->bIsNoteItem == false)
				{
					continue;
				}
				if (ItemData->AllowedSpawnMaps.HasAnyExact(CurrentRegionTags) == false)
				{
					continue;
				}

				NoteItems.Add(RowName);
			}
		}
	}

	return NoteItems;
}

void AResourceItemSpawnManager::SpawnNoteItems(int32 Count)
{
	TArray<FName> SpawnableNotes = GetSpawnableNotesByTags();
	if (SpawnableNotes.Num() == 0)
	{
		LOG_Item_WARNING(TEXT("[SpawnNoteItems] No spawnable note items"));
		return;
	}

	// TargetPoint에서 NoteSpawn 태그가 붙은 것만 사용
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ATargetPoint::StaticClass(), FName("NoteSpawn"), FoundPoints);
	if (FoundPoints.Num() == 0)
	{
		LOG_Item_WARNING(TEXT("[SpawnNoteItems] No NoteSpawn TargetPoints"));
		return;
	}

	FoundPoints.Sort([](const AActor& A, const AActor& B) { return FMath::RandBool(); });

	for (int32 i = 0; i < Count && i < FoundPoints.Num(); ++i)
	{
		const FVector SpawnLocation = FoundPoints[i]->GetActorLocation();
		const FName NoteRow = SpawnableNotes[FMath::RandHelper(SpawnableNotes.Num())];

		SpawnItemAtLocation(NoteRow, SpawnLocation);
	}
}

void AResourceItemSpawnManager::SpawnNoteItemsForTheme()
{
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResourceItemSpawnPoint::StaticClass(), FoundPoints);

	TArray<FName> Notes = GetSpawnableNotesByTags();
	if (Notes.IsEmpty()) return;

	for (AActor* Actor : FoundPoints)
	{
		if (AResourceItemSpawnPoint* Point = Cast<AResourceItemSpawnPoint>(Actor))
		{
			if (Point->SpawnItemType == ESpawnItemType::Note || Point->SpawnItemType == ESpawnItemType::Both)
			{
				Point->TrySpawnItemFromManager(Notes, ItemSpawnerComponent);
			}
		}
	}
}

void AResourceItemSpawnManager::SpawnNoteItemsAtLocation(int32 Count, const FVector& Location)
{
	const TArray<FName> Notes = GetSpawnableNotesByTags();
	if (Notes.IsEmpty())
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		const FName Row = Notes[FMath::RandHelper(Notes.Num())];
		SpawnItemAtLocation(Row, Location); // 기존 함수 재사용
	}
}

//--------------------------------------------------------------------------------
// [5] 자원 노드 스폰
//--------------------------------------------------------------------------------

void AResourceItemSpawnManager::SpawnResourceNodes(int32 Count)
{
	if (ResourceNodeSpawnTable == nullptr)
	{
		return;
	}

	// 1. DataTable Row 가져오기
	TArray<FResourceNodeSpawnRow*> Rows;
	ResourceNodeSpawnTable->GetAllRows(TEXT("SpawnResourceNodes"), Rows);
	if (Rows.Num() == 0)
	{
		return;
	}

	// 2. TargetPoint 목록 수집
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClassWithTag(
		GetWorld(),
		ATargetPoint::StaticClass(),
		TargetPointTag,
		FoundPoints);
	if (FoundPoints.Num() == 0)
	{
		return;
	}

	FoundPoints.Sort([](const AActor& A, const AActor& B) { return FMath::RandBool(); }); // 랜덤 섞기

	// 3. 누적 확률 계산
	float TotalProbability = 0.0f;
	for (auto Row : Rows)
	{
		TotalProbability += Row->SpawnProbability;
	}

	// 4. 스폰 루프
	for (int32 i = 0; i < Count && i < FoundPoints.Num(); ++i)
	{
		// 4-1. 확률 기반 클래스 선택
		float Roll = FMath::FRandRange(0.f, TotalProbability);
		float Accum = 0.0f;
		TSubclassOf<AResourceNode> SelectedClass = nullptr;

		for (auto Row : Rows)
		{
			Accum += Row->SpawnProbability;
			if (Roll <= Accum)
			{
				SelectedClass = Row->ResourceNodeClass;
				break;
			}
		}

		if (SelectedClass == nullptr)
		{
			continue;
		}

		// 4-2. 타깃 포인트에서 무작위 선택 (혹은 순차 선택)
		int32 Index = i % FoundPoints.Num();
		FVector SpawnLocation = FoundPoints[Index]->GetActorLocation();
		FRotator SpawnRotation = FoundPoints[Index]->GetActorRotation();

		// 4-3. 액터 스폰
		FActorSpawnParameters Params;
		GetWorld()->SpawnActor<AResourceNode>(SelectedClass, SpawnLocation, SpawnRotation, Params);
	}
}

//--------------------------------------------------------------------------------
// [6] 낮/밤 시간 변경 시 아이템 반응
//--------------------------------------------------------------------------------

void AResourceItemSpawnManager::OnDayNightChanged(EDayPhase NewPhase)
{
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResourceItemSpawnPoint::StaticClass(), FoundPoints);

	// 태그 기반으로 스폰 가능한 아이템 목록 가져오기
	TArray<FName> AvailableItems = GetSpawnableResourceItemsByTags();

	for (AActor* Actor : FoundPoints)
	{
		if (AResourceItemSpawnPoint* Point = Cast<AResourceItemSpawnPoint>(Actor))
		{
			const ESpawnItemType Type = Point->SpawnItemType;

			// 스폰할 수 있는 아이템 목록
			TArray<FName> Spawnables;

			if (Type == ESpawnItemType::Note)
			{
				Spawnables = GetSpawnableNotesByTags();
			}
			else if (Type == ESpawnItemType::Resource)
			{
				Spawnables = GetSpawnableResourceItemsByTags();
			}
			else if (Type == ESpawnItemType::Both)
			{
				// NOTE: 필요에 따라 둘 중 하나를 랜덤 선택하거나, 리팩토링 가능
				if (FMath::RandBool())
					Spawnables = GetSpawnableNotesByTags();
				else
					Spawnables = GetSpawnableResourceItemsByTags();
			}

			if (!Spawnables.IsEmpty())
			{
				Point->TrySpawnItemFromManager(Spawnables, ItemSpawnerComponent);
			}
		}
	}
}