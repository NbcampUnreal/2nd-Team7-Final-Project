#include "Item/Component/ContainerInteractionComponent.h"
#include "Character/BaseCharacter.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Item/ItemContainer.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "GameFramework/PlayerController.h"
#include "LastCanary.h"

UContainerInteractionComponent::UContainerInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(false); // 컴포넌트 자체는 복제하지 않음

    CachedOwnerCharacter = nullptr;
    CachedInventoryComponent = nullptr;
    CurrentOpenContainer = nullptr;
}

void UContainerInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    // 소유자 참조 캐싱
    CacheOwnerReferences();
}

void UContainerInteractionComponent::CacheOwnerReferences()
{
    CachedOwnerCharacter = Cast<ABaseCharacter>(GetOwner());
    if (CachedOwnerCharacter)
    {
        CachedInventoryComponent = CachedOwnerCharacter->GetToolbarInventoryComponent();

        if (!CachedInventoryComponent)
        {
            LOG_Item_WARNING(TEXT("[ContainerInteractionComponent] 소유자 캐릭터에 ToolbarInventoryComponent가 없습니다."));
        }
    }
    else
    {
        LOG_Item_WARNING(TEXT("[ContainerInteractionComponent] 소유자가 BaseCharacter가 아닙니다."));
    }
}

//-----------------------------------------------------
// Server RPC 구현
//-----------------------------------------------------

void UContainerInteractionComponent::Server_InteractWithContainer_Implementation(AItemContainer* Container)
{
    if (!ValidateContainerInteraction(Container))
    {
        LOG_Item_WARNING(TEXT("[Server_InteractWithContainer] 컨테이너 상호작용 검증 실패"));
        return;
    }

    APlayerController* PC = GetOwnerPlayerController();
    if (!PC)
    {
        LOG_Item_WARNING(TEXT("[Server_InteractWithContainer] PlayerController를 찾을 수 없음"));
        return;
    }

    // 컨테이너 열기
    Container->Server_OpenContainer(PC);
    CurrentOpenContainer = Container;

    Client_ShowContainerUI(Container, Container->GetContainerItems());

    LOG_Item_WARNING(TEXT("[Server_InteractWithContainer] 컨테이너 상호작용 성공: %s"),
        *Container->GetContainerName().ToString());
}

void UContainerInteractionComponent::Server_MoveItemToContainer_Implementation(AItemContainer* Container, int32 PlayerSlotIndex, int32 ContainerSlotIndex, int32 Quantity)
{
    if (!ValidateItemTransfer(Container, PlayerSlotIndex, ContainerSlotIndex, Quantity, true))
    {
        LOG_Item_WARNING(TEXT("[Server_MoveItemToContainer] 아이템 이동 검증 실패"));
        return;
    }

    APlayerController* PC = GetOwnerPlayerController();
    if (!PC)
    {
        LOG_Item_WARNING(TEXT("[Server_MoveItemToContainer] PlayerController를 찾을 수 없음"));
        return;
    }

    // 컨테이너의 기존 함수 활용
    Container->Server_MoveItemToContainer(PC, PlayerSlotIndex, ContainerSlotIndex, Quantity);

    LOG_Item_WARNING(TEXT("[Server_MoveItemToContainer] 플레이어 → 컨테이너 이동: 슬롯 %d → %d, 수량: %d"),
        PlayerSlotIndex, ContainerSlotIndex, Quantity);
}

void UContainerInteractionComponent::Server_MoveItemToPlayer_Implementation(AItemContainer* Container, int32 ContainerSlotIndex, int32 PlayerSlotIndex, int32 Quantity)
{
    if (!ValidateItemTransfer(Container, ContainerSlotIndex, PlayerSlotIndex, Quantity, false))
    {
        LOG_Item_WARNING(TEXT("[Server_MoveItemToPlayer] 아이템 이동 검증 실패"));
        return;
    }

    APlayerController* PC = GetOwnerPlayerController();
    if (!PC)
    {
        LOG_Item_WARNING(TEXT("[Server_MoveItemToPlayer] PlayerController를 찾을 수 없음"));
        return;
    }

    // 컨테이너의 기존 함수 활용
    Container->Server_MoveItemToPlayer(PC, ContainerSlotIndex, PlayerSlotIndex, Quantity);

    LOG_Item_WARNING(TEXT("[Server_MoveItemToPlayer] 컨테이너 → 플레이어 이동: 슬롯 %d → %d, 수량: %d"),
        ContainerSlotIndex, PlayerSlotIndex, Quantity);
}

void UContainerInteractionComponent::Server_CloseContainer_Implementation(AItemContainer* Container)
{
    if (!Container)
    {
        LOG_Item_WARNING(TEXT("[Server_CloseContainer] Container가 null"));
        return;
    }

    APlayerController* PC = GetOwnerPlayerController();
    if (!PC)
    {
        LOG_Item_WARNING(TEXT("[Server_CloseContainer] PlayerController를 찾을 수 없음"));
        return;
    }

    // 컨테이너 닫기
    Container->Server_CloseContainer(PC);
    CurrentOpenContainer = nullptr;

    LOG_Item_WARNING(TEXT("[Server_CloseContainer] 컨테이너 닫기 성공"));
}

void UContainerInteractionComponent::Client_ShowContainerUI_Implementation(AItemContainer* Container, const TArray<FContainerItemData>& Items)
{
    // 소유자가 로컬 플레이어인지 확인
    if (!CachedOwnerCharacter || !CachedOwnerCharacter->IsLocallyControlled())
    {
        return;
    }

    CurrentOpenContainer = Container;

    if (ULCGameInstanceSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
    {
        if (ULCUIManager* UIManager = Subsystem->GetUIManager())
        {
            if (UInventoryMainWidget* MainWidget = UIManager->GetInventoryMainWidget())
            {
                MainWidget->ShowContainerUI(Container, Items);
                LOG_Item_WARNING(TEXT("[ContainerComponent] 컨테이너 UI 표시 성공"));
            }
        }
    }
}

void UContainerInteractionComponent::Client_ForceCloseContainer_Implementation(AItemContainer* Container)
{
    if (!Container)
    {
        LOG_Item_WARNING(TEXT("[Client_ForceCloseContainer] Container가 null"));
        return;
    }

    // 현재 열린 컨테이너와 일치하는지 확인
    if (CurrentOpenContainer == Container)
    {
        CurrentOpenContainer = nullptr;
        LOG_Item_WARNING(TEXT("[Client_ForceCloseContainer] 컨테이너 일치, UI 닫기 시도"));

        // UI 매니저를 통해 컨테이너 UI 닫기
        if (ULCGameInstanceSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
        {
            if (ULCUIManager* UIManager = Subsystem->GetUIManager())
            {
                if (UInventoryMainWidget* MainWidget = UIManager->GetInventoryMainWidget())
                {
                    LOG_Item_WARNING(TEXT("[Client_ForceCloseContainer] MainWidget 발견, HideContainerUI 호출"));
                    MainWidget->HideContainerUI();

                    // 입력 모드를 게임 전용으로 변경
                    if (APlayerController* PC = GetOwnerPlayerController())
                    {
                        FInputModeGameOnly InputMode;
                        PC->SetInputMode(InputMode);
                        PC->SetShowMouseCursor(false);
                    }
                }
                else
                {
                    LOG_Item_WARNING(TEXT("[Client_ForceCloseContainer] MainWidget을 찾을 수 없음"));
                }
            }
            else
            {
                LOG_Item_WARNING(TEXT("[Client_ForceCloseContainer] UIManager를 찾을 수 없음"));
            }
        }
        else
        {
            LOG_Item_WARNING(TEXT("[Client_ForceCloseContainer] GameInstanceSubsystem을 찾을 수 없음"));
        }

        LOG_Item_WARNING(TEXT("[ContainerInteractionComponent] 거리 초과로 컨테이너 UI 강제 닫기"));
    }
    else
    {
        LOG_Item_WARNING(TEXT("[Client_ForceCloseContainer] 컨테이너가 일치하지 않음"));
    }
}

//-----------------------------------------------------
// 헬퍼 함수들
//-----------------------------------------------------

UToolbarInventoryComponent* UContainerInteractionComponent::GetOwnerInventoryComponent() const
{
    if (CachedInventoryComponent)
    {
        return CachedInventoryComponent;
    }

    // 캐시가 없으면 다시 찾기
    if (CachedOwnerCharacter)
    {
        return CachedOwnerCharacter->GetToolbarInventoryComponent();
    }

    return nullptr;
}

APlayerController* UContainerInteractionComponent::GetOwnerPlayerController() const
{
    if (CachedOwnerCharacter)
    {
        return Cast<APlayerController>(CachedOwnerCharacter->GetController());
    }

    return nullptr;
}

ABaseCharacter* UContainerInteractionComponent::GetOwnerCharacter() const
{
    return CachedOwnerCharacter;
}

bool UContainerInteractionComponent::CanInteractWithContainer(AItemContainer* Container) const
{
    if (!Container)
    {
        return false;
    }

    if (!CachedOwnerCharacter || !CachedInventoryComponent)
    {
        return false;
    }

    // 거리 체크 (선택사항)
    float Distance = FVector::Dist(CachedOwnerCharacter->GetActorLocation(), Container->GetActorLocation());
    if (Distance > 300.0f) // 3미터 제한
    {
        return false;
    }

    return true;
}

bool UContainerInteractionComponent::ValidateItemMove(int32 SourceSlotIndex, int32 TargetSlotIndex, int32 Quantity) const
{
    if (Quantity <= 0)
    {
        return false;
    }

    if (SourceSlotIndex < 0 || TargetSlotIndex < 0)
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------
// Private 검증 함수들
//-----------------------------------------------------

bool UContainerInteractionComponent::ValidateContainerInteraction(AItemContainer* Container) const
{
    if (!Container)
    {
        LOG_Item_WARNING(TEXT("[ValidateContainerInteraction] Container가 null"));
        return false;
    }

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[ValidateContainerInteraction] Authority가 없음"));
        return false;
    }

    if (!CachedOwnerCharacter)
    {
        LOG_Item_WARNING(TEXT("[ValidateContainerInteraction] 소유자 캐릭터가 null"));
        return false;
    }

    if (!CachedInventoryComponent)
    {
        LOG_Item_WARNING(TEXT("[ValidateContainerInteraction] 인벤토리 컴포넌트가 null"));
        return false;
    }

    return CanInteractWithContainer(Container);
}

bool UContainerInteractionComponent::ValidateItemTransfer(AItemContainer* Container, int32 SourceSlot, int32 TargetSlot, int32 Quantity, bool bToContainer) const
{
    if (!ValidateContainerInteraction(Container))
    {
        return false;
    }

    if (!ValidateItemMove(SourceSlot, TargetSlot, Quantity))
    {
        LOG_Item_WARNING(TEXT("[ValidateItemTransfer] 기본 아이템 이동 검증 실패"));
        return false;
    }

    // 추가 검증 로직
    if (bToContainer)
    {
        // 플레이어 → 컨테이너
        if (!CachedInventoryComponent->ItemSlots.IsValidIndex(SourceSlot))
        {
            LOG_Item_WARNING(TEXT("[ValidateItemTransfer] 플레이어 슬롯 인덱스가 유효하지 않음: %d"), SourceSlot);
            return false;
        }

        if (TargetSlot >= Container->GetMaxSlots())
        {
            LOG_Item_WARNING(TEXT("[ValidateItemTransfer] 컨테이너 슬롯 인덱스가 유효하지 않음: %d"), TargetSlot);
            return false;
        }
    }
    else
    {
        // 컨테이너 → 플레이어
        if (SourceSlot >= Container->GetMaxSlots())
        {
            LOG_Item_WARNING(TEXT("[ValidateItemTransfer] 컨테이너 슬롯 인덱스가 유효하지 않음: %d"), SourceSlot);
            return false;
        }

        if (!CachedInventoryComponent->ItemSlots.IsValidIndex(TargetSlot))
        {
            LOG_Item_WARNING(TEXT("[ValidateItemTransfer] 플레이어 슬롯 인덱스가 유효하지 않음: %d"), TargetSlot);
            return false;
        }
    }

    return true;
}
