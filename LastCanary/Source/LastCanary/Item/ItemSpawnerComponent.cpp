#include "Item/ItemSpawnerComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Character/BaseCharacter.h"
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

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = nullptr;
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

//void UItemSpawnerComponent::EnablePhysicsSimulation(AItemBase* Item)
//{
//    if (!Item)
//        return;
//
//    // 총기인 경우
//    if (AGunBase* Gun = Cast<AGunBase>(Item))
//    {
//        if (USkeletalMeshComponent* GunMesh = Gun->GetGunMesh())
//        {
//            GunMesh->SetSimulatePhysics(true);
//            GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
//            GunMesh->SetCollisionObjectType(ECC_WorldDynamic);
//            GunMesh->SetCollisionResponseToAllChannels(ECR_Block);
//
//            // ⭐ 컴포넌트 설정값 사용
//            FVector RandomImpulse = FVector(
//                FMath::RandRange(-ImpulseRandomRange.X, ImpulseRandomRange.X),
//                FMath::RandRange(-ImpulseRandomRange.Y, ImpulseRandomRange.Y),
//                FMath::RandRange(100.0f, (float)ImpulseRandomRange.Z)
//            );
//            GunMesh->AddImpulse(RandomImpulse * DropImpulseStrength / 300.0f);
//        }
//    }
//    // 일반 아이템인 경우
//    else if (UStaticMeshComponent* MeshComp = Item->GetMeshComponent())
//    {
//        MeshComp->SetSimulatePhysics(true);
//        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
//        MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
//        MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
//
//        // ⭐ 컴포넌트 설정값 사용
//        FVector RandomImpulse = FVector(
//            FMath::RandRange(-ImpulseRandomRange.X, ImpulseRandomRange.X),
//            FMath::RandRange(-ImpulseRandomRange.Y, ImpulseRandomRange.Y),
//            FMath::RandRange(100.0f, (float)ImpulseRandomRange.Z)
//        );
//        MeshComp->AddImpulse(RandomImpulse * DropImpulseStrength / 300.0f);
//    }
//}

float UItemSpawnerComponent::CalculateThrowVelocity(float ItemWeight) const
{
    // 무게에 반비례하는 속도 계산
    float WeightFactor = FMath::Max(0.1f, 1.0f - (ItemWeight * WeightMultiplier));
    float CalculatedVelocity = BaseThrowVelocity * WeightFactor;

    // 최소/최대 속도 제한
    return FMath::Clamp(CalculatedVelocity, MinThrowVelocity, MaxThrowVelocity);
}

FVector UItemSpawnerComponent::CalculateThrowDirection() const
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        LOG_Item_WARNING(TEXT("[CalculateThrowDirection] Owner is null"));
        return FVector::ForwardVector;
    }

    ABaseCharacter* Character = Cast<ABaseCharacter>(OwnerActor);
    if (!Character)
    {
        LOG_Item_WARNING(TEXT("[CalculateThrowDirection] Owner is not BaseCharacter"));
        return FVector::ForwardVector;
    }

    // ⭐ AGunBase::PerformLineTrace와 같은 방식으로 카메라 방향 가져오기
    AController* Controller = Character->GetController();
    if (!Controller)
    {
        LOG_Item_WARNING(TEXT("[CalculateThrowDirection] Controller is null"));
        return FVector::ForwardVector;
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

    LOG_Item_WARNING(TEXT("[CalculateThrowDirection] Camera Location: (%f, %f, %f)"),
        CameraLocation.X, CameraLocation.Y, CameraLocation.Z);
    LOG_Item_WARNING(TEXT("[CalculateThrowDirection] Camera Rotation: (%f, %f, %f)"),
        CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);

    // 던지기 각도 추가 (위쪽으로)
    FRotator ThrowRotation = CameraRotation;
    ThrowRotation.Pitch += ThrowAngleDegrees;

    FVector ThrowDirection = ThrowRotation.Vector();
    LOG_Item_WARNING(TEXT("[CalculateThrowDirection] Final ThrowDirection: (%f, %f, %f)"),
        ThrowDirection.X, ThrowDirection.Y, ThrowDirection.Z);

    return ThrowDirection;
}

void UItemSpawnerComponent::EnablePhysicsSimulation(AItemBase* Item)
{
    if (!Item)
    {
        LOG_Item_WARNING(TEXT("[EnablePhysicsSimulation] 아이템이 null입니다."));
        return;
    }

    // 아이템 데이터에서 무게 가져오기
    float ItemWeight = 1.0f;
    if (ULCGameInstanceSubsystem* GameSubsystem = GetGameSubsystem())
    {
        if (const FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(Item->ItemRowName))
        {
            ItemWeight = ItemData->Weight;
        }
    }

    // 던지기 속도와 방향 계산
    float ThrowVelocity = CalculateThrowVelocity(ItemWeight);
    FVector ThrowDirection = CalculateThrowDirection();
    FVector ThrowImpulse = ThrowDirection * ThrowVelocity;

    LOG_Item_WARNING(TEXT("[EnablePhysicsSimulation] 아이템: %s, 무게: %.2f, 속도: %.2f"),
        *Item->ItemRowName.ToString(), ItemWeight, ThrowVelocity);

    // ⭐ 무브먼트 리플리케이션을 위한 설정
    Item->SetReplicateMovement(true);
    Item->SetReplicates(true);

    // 총기인 경우
    if (AGunBase* Gun = Cast<AGunBase>(Item))
    {
        if (USkeletalMeshComponent* GunMesh = Gun->GetGunMesh())
        {
            // 물리 설정
            GunMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            GunMesh->SetCollisionObjectType(ECC_WorldDynamic);
            GunMesh->SetCollisionResponseToAllChannels(ECR_Block);
            GunMesh->SetSimulatePhysics(true);

            // ⭐ 즉시 임펄스 적용 (타이머 제거)
            GunMesh->SetPhysicsLinearVelocity(ThrowDirection * ThrowVelocity * 0.01f); // 초기 속도 설정
            GunMesh->AddImpulse(ThrowImpulse, NAME_None, true); // VelChange 사용

            LOG_Item_WARNING(TEXT("[EnablePhysicsSimulation] 총기 임펄스 즉시 적용"));
        }
    }
    // 일반 아이템인 경우
    else if (UStaticMeshComponent* MeshComp = Item->GetMeshComponent())
    {
        // 물리 설정
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComp->SetCollisionObjectType(ECC_WorldDynamic);
        MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        MeshComp->SetSimulatePhysics(true);

        // ⭐ 즉시 임펄스 적용
        MeshComp->SetPhysicsLinearVelocity(ThrowDirection * ThrowVelocity * 0.01f);
        MeshComp->AddImpulse(ThrowImpulse, NAME_None, true); // VelChange 사용

        LOG_Item_WARNING(TEXT("[EnablePhysicsSimulation] 아이템 임펄스 즉시 적용"));
    }

    // ⭐ 디버그 시각화 (던지기 궤적)
    if (AActor* Owner = GetOwner())
    {
        FVector StartLocation = Item->GetActorLocation();
        FVector EndLocation = StartLocation + ThrowDirection * 500.0f;

        DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Yellow, false, 2.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), StartLocation, 15.0f, 12, FColor::Green, false, 2.0f);
        DrawDebugDirectionalArrow(GetWorld(), StartLocation, EndLocation, 30.0f, FColor::Red, false, 2.0f, 0, 2.0f);
    }
}
