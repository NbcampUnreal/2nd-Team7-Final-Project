#include "Item/ResourceNode.h"
#include "Item/ResourceItem/ResourceItemSpawnManager.h"
#include "Item/ItemBase.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"

AResourceNode::AResourceNode()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AResourceNode::BeginPlay()
{
    // 레벨 상에 있는 스폰 매니저 할당
    ResourceItemSpawnManager = Cast<AResourceItemSpawnManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AResourceItemSpawnManager::StaticClass()));
    if (!ResourceItemSpawnManager)
    {
        LOG_Item_WARNING(TEXT("[AResourceNode::BeginPlay] ResourceItemSpawnManager not found in level!"));
    }
}

void AResourceNode::Interact_Implementation(APlayerController* Interactor)
{
    if (ABaseCharacter* Character = Cast<ABaseCharacter>(Interactor->GetPawn()))
    {
        Character->Server_InteractWithResourceNode(this);
    }
}

void AResourceNode::Server_RequestInteract_Implementation(APlayerController* Interactor)
{
    // 서버에서 실제 상호작용 처리
    HarvestResource(Interactor);
}

void AResourceNode::HarvestResource(APlayerController* Interactor)
{
    if (!ResourceItemSpawnManager)
    {
        LOG_Item_WARNING(TEXT("[AResourceNode::Interact_Implementation] ResourceItemSpawnManager not initialized!"));
        return;
    }

    if (PossibleResourceItems.Num() <= 0)
    {
        LOG_Item_WARNING(TEXT("[AResourceNode::Interact_Implementation] 스폰할 아이템 목록이 할당되지 않았습니다."));
        return;
    }

    // 일단은 완전 랜덤
    FName SpawnItemRow = PossibleResourceItems[FMath::RandRange(0, PossibleResourceItems.Num() - 1)];

    // 매니저에 스폰 요청
    FVector SpawnLocation = CalculateResourceSpawnLocation(Interactor);
    ResourceItemSpawnManager->SpawnItemAtLocation(SpawnItemRow, SpawnLocation);

    // TODO : 내구도 차감, 재사용 쿨타임 적용, 디스폰 등 부가 로직
}

FString AResourceNode::GetInteractMessage_Implementation() const
{
    // 플레이어로부터 현재 장착 아이템 태그를 받아와서 비교해야 함
    ABaseCharacter* Character = GetWorld()->GetFirstPlayerController()
        ? Cast<ABaseCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn())
        : nullptr;

    if (!Character)
    {
        return TEXT("상호작용 불가(플레이어 없음)");
    }

    if (AItemBase* Equipped = Character->GetToolbarInventoryComponent()->GetCurrentEquippedItem())
    {
        // 곡괭이 태그와 매칭 여부만 간략히 체크
        if (Equipped->ItemData.ItemType.MatchesTag(RequiredToolTag)) // 예: "Tool.Pickaxe"
        {
            return InteractSuccessMessage.ToString();
        }
    }
    return InteractFailMessage.ToString();
}

FVector AResourceNode::CalculateResourceSpawnLocation(APlayerController* Interactor) const
{
    FVector NodeLocation = GetActorLocation();

    switch (SpawnLocationType)
    {
    case EResourceSpawnLocationType::NodeTop:
        return NodeLocation + FVector(0, 0, 100);

    case EResourceSpawnLocationType::MidpointToCharacter:
    {
        if (Interactor && Interactor->GetPawn())
        {
            FVector CharLocation = Interactor->GetPawn()->GetActorLocation();
            return (NodeLocation + CharLocation) * 0.5f;
        }
        return NodeLocation;
    }

    case EResourceSpawnLocationType::CustomOffset:
        return NodeLocation + CustomSpawnOffset;

    default:
        return NodeLocation;
    }
}
