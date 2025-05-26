#include "Item/ItemSpawner.h"
#include "Character/BaseCharacter.h"
#include "Engine/World.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "LastCanary.h"

AItemSpawner::AItemSpawner()
{
    PrimaryActorTick.bCanEverTick = true;

    // 루트 컴포넌트 설정
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // 스포너 메시 컴포넌트 생성
    SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
    SpawnerMesh->SetupAttachment(RootComponent);
    SpawnerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 근접 감지 컴포넌트 생성
    ProximityComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProximityComponent"));
    ProximityComponent->SetupAttachment(RootComponent);
    ProximityComponent->SetCollisionProfileName(TEXT("Trigger"));
    ProximityComponent->SetSphereRadius(500.0f);

    ProximityComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 오버랩 이벤트 바인딩
    ProximityComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemSpawner::OnProximityBeginOverlap);
    ProximityComponent->OnComponentEndOverlap.AddDynamic(this, &AItemSpawner::OnProximityEndOverlap);

    // 기본값 설정
    CurrentSpawnCount = 0;
    bIsEnabled = true;
    LastSpawnTime = 0.0f;
    bPlayerInProximity = false;

    // 네트워크 복제 활성화
    bReplicates = true;
    bAlwaysRelevant = false;
    AActor::SetReplicateMovement(false); // 스포너는 움직이지 않음
}

void AItemSpawner::BeginPlay()
{
    Super::BeginPlay();

    // 스포너 설정 상태 전체 로그
    UE_LOG(LogTemp, Warning, TEXT("=== [AItemSpawner::BeginPlay] 스포너 설정 확인 시작 ==="));
    UE_LOG(LogTemp, Warning, TEXT("스포너 이름: %s"), *GetName());
    UE_LOG(LogTemp, Warning, TEXT("스포너 타입: %d"), (int32)SpawnerType);
    UE_LOG(LogTemp, Warning, TEXT("스폰 가능한 아이템 개수: %d"), SpawnableItems.Num());

    // 각 아이템 정보 상세 로그
    for (int32 i = 0; i < SpawnableItems.Num(); ++i)
    {
        const FSpawnableItemInfo& ItemInfo = SpawnableItems[i];
        UE_LOG(LogTemp, Warning, TEXT("--- 아이템 [%d] 정보 ---"), i);
        UE_LOG(LogTemp, Warning, TEXT("ItemClass: %s"), ItemInfo.ItemClass ? *ItemInfo.ItemClass->GetName() : TEXT("NULL"));
        UE_LOG(LogTemp, Warning, TEXT("ItemRowName: %s"), *ItemInfo.ItemRowName.ToString());
        UE_LOG(LogTemp, Warning, TEXT("ItemRowName IsValid: %s"), ItemInfo.ItemRowName.IsValid() ? TEXT("True") : TEXT("False"));
        UE_LOG(LogTemp, Warning, TEXT("ItemRowName IsNone: %s"), ItemInfo.ItemRowName.IsNone() ? TEXT("True") : TEXT("False"));
        UE_LOG(LogTemp, Warning, TEXT("ItemRowName String: '%s'"), *ItemInfo.ItemRowName.ToString());
        UE_LOG(LogTemp, Warning, TEXT("SpawnProbability: %f"), ItemInfo.SpawnProbability);
        UE_LOG(LogTemp, Warning, TEXT("MinQuantity: %d"), ItemInfo.MinQuantity);
        UE_LOG(LogTemp, Warning, TEXT("MaxQuantity: %d"), ItemInfo.MaxQuantity);
        UE_LOG(LogTemp, Warning, TEXT("MinDurability: %f"), ItemInfo.MinDurability);
        UE_LOG(LogTemp, Warning, TEXT("MaxDurability: %f"), ItemInfo.MaxDurability);
    }
    UE_LOG(LogTemp, Warning, TEXT("=== 스포너 설정 확인 완료 ==="));

    // 근접 컴포넌트 반경 설정
    ProximityComponent->SetSphereRadius(ProximityDistance);

    // 자동 스폰 처리
    if (bAutoSpawnOnStart && SpawnerType == ESpawnerType::OneTime)
    {
        SpawnItems();
    }
    else if (SpawnerType == ESpawnerType::Periodic)
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AItemSpawner::HandlePeriodicSpawn, SpawnInterval, true);
    }
}

void AItemSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Proximity 타입 처리
    if (SpawnerType == ESpawnerType::Proximity && bIsEnabled)
    {
        HandleProximitySpawn();
    }
}

void AItemSpawner::SpawnItems()
{
    LOG_Item_WARNING(TEXT("[AItemSpawner::SpawnItems] 스폰 시작 - Authority: %s, Enabled: %s, Count: %d/%d"),
        HasAuthority() ? TEXT("True") : TEXT("False"),
        bIsEnabled ? TEXT("True") : TEXT("False"),
        CurrentSpawnCount, MaxSpawnCount);

    if (!bIsEnabled || CurrentSpawnCount >= MaxSpawnCount)
    {
        LOG_Item_WARNING(TEXT("[AItemSpawner::SpawnItems] 스포너 비활성화 상태이거나 최대 스폰 개수에 도달했습니다."));
        return;
    }

    if (SpawnableItems.Num() == 0)
    {
        LOG_Item_WARNING(TEXT("[AItemSpawner::SpawnItems] 스폰할 아이템이 설정되지 않았습니다."));
        return;
    }

    // 서버에서만 실행
    if (!HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[AItemSpawner::SpawnItems] Authority 없음"));
        return;
    }

    // 랜덤 아이템 선택
    FSpawnableItemInfo* SelectedItem = SelectRandomItem();
    if (!SelectedItem)
    {
        LOG_Item_WARNING(TEXT("[AItemSpawner::SpawnItems] 유효한 아이템을 선택하지 못했습니다."));
        return;
    }

    // 아이템 스폰
    AItemBase* SpawnedItem = SpawnSpecificItem(*SelectedItem);
    if (SpawnedItem)
    {
        CurrentSpawnCount++;
        OnItemSpawned(SpawnedItem);

        LOG_Item_WARNING(TEXT("[AItemSpawner::SpawnItems] 아이템 스폰 성공: %s"),
            *SelectedItem->ItemRowName.ToString());

        // 스포너 숨기기 설정 확인
        if (bHideSpawnerAfterSpawn)
        {
            SpawnerMesh->SetVisibility(false);
        }

        // OneTime 타입이면 비활성화
        if (SpawnerType == ESpawnerType::OneTime)
        {
            SetSpawnerEnabled(false);
        }
    }
}

AItemBase* AItemSpawner::SpawnSpecificItem(const FSpawnableItemInfo& ItemInfo)
{
    if (!ItemInfo.ItemClass)
    {
        LOG_Item_WARNING(TEXT("[AItemSpawner::SpawnSpecificItem] ItemClass가 유효하지 않습니다."));
        return nullptr;
    }

    // 스폰 위치 계산
    FVector SpawnLocation = GetRandomSpawnLocation();
    FRotator SpawnRotation = FRotator::ZeroRotator;

    // 아이템 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
    SpawnParams.Owner = this;

    AItemBase* SpawnedItem = GetWorld()->SpawnActor<AItemBase>(ItemInfo.ItemClass, SpawnLocation, SpawnRotation, SpawnParams);

    if (SpawnedItem)
    {
        // 🔥 중요: 속성 설정 순서
        SpawnedItem->SetReplicates(true);
        SpawnedItem->SetReplicateMovement(true);
        SpawnedItem->bAlwaysRelevant = true;

        // 🔥 ItemRowName을 먼저 설정 (OnRep 트리거)
        SpawnedItem->ItemRowName = ItemInfo.ItemRowName;

        // 다른 속성들 설정
        SpawnedItem->Quantity = FMath::RandRange(ItemInfo.MinQuantity, ItemInfo.MaxQuantity);
        SpawnedItem->Durability = FMath::RandRange(ItemInfo.MinDurability, ItemInfo.MaxDurability);

        // 서버에서 데이터 적용
        SpawnedItem->ApplyItemDataFromTable();

        // 총기 특화 처리
        if (AGunBase* Gun = Cast<AGunBase>(SpawnedItem))
        {
            Gun->ApplyGunDataFromDataTable();
        }

        // 네트워크 업데이트
        SpawnedItem->ForceNetUpdate();

        LOG_Item_WARNING(TEXT("[ItemSpawner::SpawnSpecificItem] 아이템 스폰 완료: %s (Q:%d, D:%.1f)"),
            *ItemInfo.ItemRowName.ToString(), SpawnedItem->Quantity, SpawnedItem->Durability);
    }

    return SpawnedItem;
}

void AItemSpawner::ApplyDelayedItemProperties(AItemBase* Item, const FSpawnableItemInfo& ItemInfo)
{
    if (!IsValid(Item))
    {
        LOG_Item_WARNING(TEXT("[AItemSpawner::ApplyDelayedItemProperties] Item이 유효하지 않습니다."));
        return;
    }

    LOG_Item_WARNING(TEXT("[AItemSpawner::ApplyDelayedItemProperties] 지연 속성 적용 시작 - RowName: %s"),
        *ItemInfo.ItemRowName.ToString());

    // 1. 기본 속성 설정
    Item->ItemRowName = ItemInfo.ItemRowName;

    // 2. 수량 설정
    int32 Quantity = FMath::RandRange(ItemInfo.MinQuantity, ItemInfo.MaxQuantity);
    Item->Quantity = Quantity;

    // 3. 내구도 설정
    float Durability = FMath::RandRange(ItemInfo.MinDurability, ItemInfo.MaxDurability);
    Item->Durability = Durability;

    // 4. 네트워크 복제 강제 업데이트 (서버에서만)
    if (GetNetMode() != NM_Client)
    {
        Item->ForceNetUpdate();
    }

    LOG_Item_WARNING(TEXT("[AItemSpawner::ApplyDelayedItemProperties] 기본 속성 설정 완료 - RowName: %s, 수량: %d, 내구도: %.1f"),
        *ItemInfo.ItemRowName.ToString(), Quantity, Durability);

    // 5. 데이터 테이블 적용 (추가 지연)
    FTimerHandle DataTableDelayHandle;
    GetWorld()->GetTimerManager().SetTimer(DataTableDelayHandle,
        [Item]()
        {
            if (IsValid(Item))
            {
                Item->ApplyItemDataFromTable();

                // GunBase인 경우 추가 처리
                if (AGunBase* Gun = Cast<AGunBase>(Item))
                {
                    Gun->ApplyGunDataFromDataTable();
                }
            }
        },
        0.5f, false);
}

void AItemSpawner::ResetSpawner()
{
    CurrentSpawnCount = 0;
    bIsEnabled = true;
    SpawnerMesh->SetVisibility(true);

    // 타이머 재시작
    if (SpawnerType == ESpawnerType::Periodic)
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AItemSpawner::HandlePeriodicSpawn, SpawnInterval, true);
    }

    LOG_Item_WARNING(TEXT("[AItemSpawner::ResetSpawner] 스포너가 리셋되었습니다."));
}

void AItemSpawner::SetSpawnerEnabled(bool bEnabled)
{
    bIsEnabled = bEnabled;

    if (!bEnabled)
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        OnSpawnerDeactivated();
    }
    else if (SpawnerType == ESpawnerType::Periodic)
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AItemSpawner::HandlePeriodicSpawn, SpawnInterval, true);
    }
}

void AItemSpawner::HandlePeriodicSpawn()
{
    if (bIsEnabled && CurrentSpawnCount < MaxSpawnCount)
    {
        SpawnItems();
    }
}

void AItemSpawner::HandleProximitySpawn()
{
    if (bPlayerInProximity && bIsEnabled && CurrentSpawnCount < MaxSpawnCount)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastSpawnTime >= SpawnInterval)
        {
            SpawnItems();
            LastSpawnTime = CurrentTime;
        }
    }
}

FVector AItemSpawner::GetRandomSpawnLocation()
{
    FVector BaseLocation = GetActorLocation();

    if (SpawnRadius > 0.0f)
    {
        FVector RandomOffset = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.0f, SpawnRadius);
        RandomOffset.Z = 0.0f; // Z축은 고정
        return BaseLocation + RandomOffset;
    }

    return BaseLocation;
}

FSpawnableItemInfo* AItemSpawner::SelectRandomItem()
{
    if (SpawnableItems.Num() == 0)
    {
        return nullptr;
    }

    // 확률 기반 선택
    float TotalProbability = 0.0f;
    for (const FSpawnableItemInfo& Item : SpawnableItems)
    {
        TotalProbability += Item.SpawnProbability;
    }

    if (TotalProbability <= 0.0f)
    {
        return nullptr;
    }

    float RandomValue = FMath::RandRange(0.0f, TotalProbability);
    float AccumulatedProbability = 0.0f;

    for (FSpawnableItemInfo& Item : SpawnableItems)
    {
        AccumulatedProbability += Item.SpawnProbability;
        if (RandomValue <= AccumulatedProbability)
        {
            return &Item;
        }
    }

    // 폴백: 첫 번째 아이템 반환
    return &SpawnableItems[0];
}

void AItemSpawner::OnProximityBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ABaseCharacter* Player = Cast<ABaseCharacter>(OtherActor);
    if (Player && Player->OwnedTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Character.Player"))))
    {
        bPlayerInProximity = true;
        LOG_Item_WARNING(TEXT("[AItemSpawner::OnProximityBeginOverlap] 플레이어가 근접했습니다."));
    }
}

void AItemSpawner::OnProximityEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ABaseCharacter* Player = Cast<ABaseCharacter>(OtherActor);
    if (Player && Player->OwnedTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Character.Player"))))
    {
        bPlayerInProximity = false;
        LOG_Item_WARNING(TEXT("[AItemSpawner::OnProximityEndOverlap] 플레이어가 멀어졌습니다."));
    }
}

#if WITH_EDITOR
void AItemSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    static const FName ProximityDistancePropName = GET_MEMBER_NAME_CHECKED(AItemSpawner, ProximityDistance);

    if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == ProximityDistancePropName)
    {
        if (ProximityComponent)
        {
            ProximityComponent->SetSphereRadius(ProximityDistance);
        }
    }
}
#endif
