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
    // 권한 확인 (서버에서만 실행)
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

    if (SpawnedItem)
    {
        ApplyItemSettings(SpawnedItem, ItemRowName, Quantity, Durability);
        EnablePhysicsSimulation(SpawnedItem);
    }

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
        LOG_Item_WARNING(TEXT("[UItemSpawnerComponent::CalculateThrowDirection] Owner is null"));
        return FVector::ForwardVector;
    }

    ABaseCharacter* Character = Cast<ABaseCharacter>(OwnerActor);
    if (!Character)
    {
        LOG_Item_WARNING(TEXT("[UItemSpawnerComponent::CalculateThrowDirection] Owner is not BaseCharacter"));
        return FVector::ForwardVector;
    }

    // ⭐ AGunBase::PerformLineTrace와 같은 방식으로 카메라 방향 가져오기
    AController* Controller = Character->GetController();
    if (!Controller)
    {
        LOG_Item_WARNING(TEXT("[UItemSpawnerComponent::CalculateThrowDirection] Controller is null"));
        return FVector::ForwardVector;
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

    // 던지기 각도 추가 (위쪽으로)
    FRotator ThrowRotation = CameraRotation;
    ThrowRotation.Pitch += ThrowAngleDegrees;

    FVector ThrowDirection = ThrowRotation.Vector();

    return ThrowDirection;
}

void UItemSpawnerComponent::EnablePhysicsSimulation(AItemBase* Item)
{
    if (!Item)
    {
        LOG_Item_WARNING(TEXT("[UItemSpawnerComponent::EnablePhysicsSimulation] 아이템이 null입니다."));
        return;
    }

    bool bIgnoreCharacterCollision = false;
    float ItemWeight = 1.0f;

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (ULCGameInstanceSubsystem* GameSubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
            {
                if (const FItemDataRow* ItemData = GameSubsystem->GetItemDataByRowName(Item->ItemRowName))
                {
                    ItemWeight = ItemData->Weight;
                    bIgnoreCharacterCollision = ItemData->bIgnoreCharacterCollision;

                    LOG_Item_WARNING(TEXT("[UItemSpawnerComponent::EnablePhysicsSimulation] 아이템 데이터 로드: %s, 무게: %.2f, 캐릭터 충돌 무시: %s"),
                        *Item->ItemRowName.ToString(), ItemWeight, bIgnoreCharacterCollision ? TEXT("true") : TEXT("false"));
                }
            }
        }
    }

    // 아이템의 충돌 설정 업데이트
    Item->bIgnoreCharacterCollision = bIgnoreCharacterCollision;
    Item->ApplyCollisionSettings();

    float ThrowVelocity = CalculateThrowVelocity(ItemWeight);
    FVector ThrowDirection = CalculateThrowDirection();
    FVector ThrowImpulse = ThrowDirection * ThrowVelocity;

    // 서버에서만 물리 시뮬레이션 실행
    if (GetOwner()->HasAuthority())
    {
        if (UPrimitiveComponent* ActiveMeshComp = Item->GetActiveMeshComponent())
        {
            SetupMeshPhysics(ActiveMeshComp, ThrowDirection, ThrowVelocity, ThrowImpulse);
        }
        else
        {
            LOG_Item_WARNING(TEXT("[EnablePhysicsSimulation] 활성화된 메시 컴포넌트를 찾을 수 없음"));
            return;
        }

        // 물리 적용 후 움직임 복제만 활성화
        Item->SetReplicateMovement(true);
        Item->SetReplicates(true);

        // 강제 네트워크 업데이트
        Item->ForceNetUpdate();
    }

    // 디버그 시각화
    if (AActor* Owner = GetOwner())
    {
        FVector StartLocation = Item->GetActorLocation();
        FVector EndLocation = StartLocation + ThrowDirection * 500.0f;

        FColor DebugColor = bIgnoreCharacterCollision ? FColor::Blue : FColor::Yellow;
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, DebugColor, false, 3.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), StartLocation, 15.0f, 12, FColor::Green, false, 3.0f);
        DrawDebugDirectionalArrow(GetWorld(), StartLocation, EndLocation, 30.0f, FColor::Red, false, 3.0f, 0, 2.0f);

        FString CollisionText = bIgnoreCharacterCollision ? TEXT("NO_COLLISION") : TEXT("COLLISION");
        DrawDebugString(GetWorld(), StartLocation + FVector(0, 0, 50), CollisionText, nullptr, DebugColor, 3.0f);
    }
}

void UItemSpawnerComponent::SetupMeshPhysics(UPrimitiveComponent* MeshComponent, const FVector& ThrowDirection, float ThrowVelocity, const FVector& ThrowImpulse)
{
    if (!MeshComponent)
    {
        LOG_Item_WARNING(TEXT("[UItemSpawnerComponent::SetupMeshPhysics] MeshComponent is null"));
        return;
    }

    if (!GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[UItemSpawnerComponent::SetupMeshPhysics] 클라이언트에서는 물리 설정하지 않음"));
        return;
    }

    // 스켈레탈 메시인 경우 애니메이션 인스턴스만 제거
    if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(MeshComponent))
    {
        if (UAnimInstance* AnimInstance = SkeletalMeshComp->GetAnimInstance())
        {
            SkeletalMeshComp->SetAnimInstanceClass(nullptr);
        }
    }

    MeshComponent->SetSimulatePhysics(true);

    // 물리 활성화 확인 후 속도/임펄스 적용
    if (MeshComponent->IsSimulatingPhysics())
    {
        MeshComponent->SetPhysicsLinearVelocity(ThrowDirection * ThrowVelocity * 0.01f);
        MeshComponent->AddImpulse(ThrowImpulse, NAME_None, true);

        // 안정성 설정
        MeshComponent->SetLinearDamping(0.1f);
        MeshComponent->SetAngularDamping(0.1f);
    }
    else
    {
        LOG_Item_WARNING(TEXT("[UItemSpawnerComponent::SetupMeshPhysics] 물리 시뮬레이션 활성화 실패"));
    }
}
