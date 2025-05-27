#include "Item/ItemSpawnerComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "LastCanary.h"

UItemSpawnerComponent::UItemSpawnerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(false); // 컴포넌트 자체는 복제 안 함 (생성되는 아이템만 복제)
}

ULCGameInstanceSubsystem* UItemSpawnerComponent::GetGameSubsystem()
{
    if (!CachedGameSubsystem)
    {
        if (UWorld* World = GetWorld())
        {
            CachedGameSubsystem = World->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
        }
    }
    return CachedGameSubsystem;
}

AItemBase* UItemSpawnerComponent::CreateItem(FName ItemRowName, const FVector& SpawnLocation)
{
    return CreateItemWithCustomData(ItemRowName, SpawnLocation, DefaultQuantity, DefaultDurability);
}

AItemBase* UItemSpawnerComponent::CreateItemFromData(const FBaseItemSlotData& SourceItemData, const FVector& SpawnLocation)
{
    return CreateItemWithCustomData(SourceItemData.ItemRowName, SpawnLocation, SourceItemData.Quantity, SourceItemData.Durability);
}

AItemBase* UItemSpawnerComponent::CreateItemWithCustomData(FName ItemRowName, const FVector& SpawnLocation, int32 Quantity, float Durability)
{
    // ⭐ 권한 확인 (서버에서만 실행)
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[ItemSpawnerComponent::CreateItemWithCustomData] Authority가 없습니다. 서버에서만 실행하세요."));
        return nullptr;
    }

    if (ItemRowName.IsNone())
    {
        LOG_Item_WARNING(TEXT("[ItemSpawnerComponent::CreateItemWithCustomData] ItemRowName이 None입니다."));
        return nullptr;
    }

    ULCGameInstanceSubsystem* GameSubsystem = GetGameSubsystem();
    if (!GameSubsystem)
    {
        LOG_Item_WARNING(TEXT("[ItemSpawnerComponent::CreateItemWithCustomData] GameInstanceSubsystem이 없습니다."));
        return nullptr;
    }

    FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(ItemRowName);
    if (!ItemData || !ItemData->ItemActorClass)
    {
        LOG_Item_WARNING(TEXT("[ItemSpawnerComponent::CreateItemWithCustomData] ItemData 또는 ItemActorClass가 없습니다: %s"),
            *ItemRowName.ToString());
        return nullptr;
    }

    // ⭐ 스폰 매개변수 설정 (Owner 설정으로 네트워크 권한 명확화)
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner(); // ⭐ 인벤토리 컴포넌트의 소유자가 아이템 소유자
    SpawnParams.Instigator = Cast<APawn>(GetOwner());
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 아이템 액터 스폰
    AItemBase* SpawnedItem = GetWorld()->SpawnActor<AItemBase>(
        ItemData->ItemActorClass,
        SpawnLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (!SpawnedItem)
    {
        LOG_Item_WARNING(TEXT("[ItemSpawnerComponent::CreateItemWithCustomData] 아이템 스폰 실패"));
        return nullptr;
    }

    // 아이템 설정 적용
    ApplyItemSettings(SpawnedItem, ItemRowName, Quantity, Durability);

    // 물리 시뮬레이션 활성화
    EnablePhysicsSimulation(SpawnedItem);

    LOG_Item_WARNING(TEXT("[ItemSpawnerComponent::CreateItemWithCustomData] 아이템 생성 완료: %s (Q:%d, D:%.1f)"),
        *ItemRowName.ToString(), Quantity, Durability);

    return SpawnedItem;
}

void UItemSpawnerComponent::ApplyItemSettings(AItemBase* Item, FName ItemRowName, int32 Quantity, float Durability)
{
    if (!Item)
        return;

    // 기본 속성 설정
    Item->ItemRowName = ItemRowName;
    Item->Quantity = Quantity;
    Item->Durability = Durability;
    Item->bIsEquipped = false;

    // 데이터 테이블 적용
    Item->ApplyItemDataFromTable();

    // 총기인 경우 총기 데이터 적용
    if (AGunBase* Gun = Cast<AGunBase>(Item))
    {
        Gun->ApplyGunDataFromDataTable();
        UE_LOG(LogTemp, Warning, TEXT("[ItemSpawnerComponent::ApplyItemSettings] 총기 데이터 적용: %s"), *ItemRowName.ToString());
    }

    // 네트워크 업데이트
    Item->ForceNetUpdate();
}

void UItemSpawnerComponent::EnablePhysicsSimulation(AItemBase* Item)
{
    if (!Item)
        return;

    // 총기인 경우
    if (AGunBase* Gun = Cast<AGunBase>(Item))
    {
        if (USkeletalMeshComponent* GunMesh = Gun->GetGunMesh())
        {
            GunMesh->SetSimulatePhysics(true);
            GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            GunMesh->SetCollisionObjectType(ECC_WorldDynamic);
            GunMesh->SetCollisionResponseToAllChannels(ECR_Block);

            // ⭐ 컴포넌트 설정값 사용
            FVector RandomImpulse = FVector(
                FMath::RandRange(-ImpulseRandomRange.X, ImpulseRandomRange.X),
                FMath::RandRange(-ImpulseRandomRange.Y, ImpulseRandomRange.Y),
                FMath::RandRange(100.0f, (float)ImpulseRandomRange.Z)
            );
            GunMesh->AddImpulse(RandomImpulse * DropImpulseStrength / 300.0f);
        }
    }
    // 일반 아이템인 경우
    else if (UStaticMeshComponent* MeshComp = Item->GetMeshComponent())
    {
        MeshComp->SetSimulatePhysics(true);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
        MeshComp->SetCollisionResponseToAllChannels(ECR_Block);

        // ⭐ 컴포넌트 설정값 사용
        FVector RandomImpulse = FVector(
            FMath::RandRange(-ImpulseRandomRange.X, ImpulseRandomRange.X),
            FMath::RandRange(-ImpulseRandomRange.Y, ImpulseRandomRange.Y),
            FMath::RandRange(100.0f, (float)ImpulseRandomRange.Z)
        );
        MeshComp->AddImpulse(RandomImpulse * DropImpulseStrength / 300.0f);
    }
}
