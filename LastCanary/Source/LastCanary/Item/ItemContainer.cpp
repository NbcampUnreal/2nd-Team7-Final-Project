#include "Item/ItemContainer.h"
#include "Item/Component/ContainerInteractionComponent.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "DataType/BaseItemSlotData.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

AItemContainer::AItemContainer()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // 기본 컴포넌트 설정
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // 컨테이너 메시 컴포넌트
    ContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerMesh"));
    ContainerMesh->SetupAttachment(RootComponent);
    ContainerMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ContainerMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // 상호작용 박스 컴포넌트
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    SetupInteractionSphere();
}

void AItemContainer::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        InitializeContainer();

        InteractionSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &AItemContainer::OnInteractionSphereBeginOverlap);
        InteractionSphere->OnComponentEndOverlap.AddUniqueDynamic(this, &AItemContainer::OnInteractionSphereEndOverlap);
    }

    SetupInteractionSphere();
}

void AItemContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AItemContainer, ContainerItems);
}

void AItemContainer::Interact_Implementation(APlayerController* Interactor)
{
    if (!Interactor)
    {
        return;
    }

    OnContainerOpened.Broadcast(Interactor);
    Client_ShowContainerUI(Interactor);
    LOG_Item_WARNING(TEXT("[ItemContainer] 컨테이너 열림: %s"), *Interactor->GetName());
}

FString AItemContainer::GetInteractMessage_Implementation() const
{
    return FString::Printf(TEXT("Press [E] to Open %s"), *ContainerName.ToString());
}

void AItemContainer::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AItemContainer::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);
    if (!Character)
    {
        return;
    }

    APlayerController* PlayerController = Character->GetController<APlayerController>();
    if (!PlayerController)
    {
        return;
    }

    ForceCloseContainerForPlayer(PlayerController);

    LOG_Item_WARNING(TEXT("[ItemContainer] 플레이어가 상호작용 범위에서 벗어남: %s"),
        *PlayerController->GetName());
}

void AItemContainer::OnRep_ContainerItems()
{
    UpdateContainerUI();
    LOG_Item_WARNING(TEXT("[ItemContainer] Container items updated on client"));
}

void AItemContainer::InitializeContainer()
{
    if (ContainerItems.Num() == 0)
    {
        ContainerItems.SetNum(MaxSlots);

        for (int32 i = 0; i < MaxSlots; ++i)
        {
            ContainerItems[i] = FContainerItemData();
        }
    }
}

bool AItemContainer::TryAddItemToContainer(const FContainerItemData& ItemData, int32 SlotIndex)
{
    if (!HasAuthority())
    {
        return false;
    }

    if (ItemData.ItemRowName == FName("Default") || ItemData.Quantity <= 0)
    {
        return false;
    }

    // 지정된 슬롯에 추가
    if (SlotIndex >= 0 && ContainerItems.IsValidIndex(SlotIndex))
    {
        FContainerItemData& Slot = ContainerItems[SlotIndex];

        if (!Slot.IsValid())
        {
            Slot = ItemData;
            return true;
        }
        else if (Slot.ItemRowName == ItemData.ItemRowName)
        {
            Slot.Quantity += ItemData.Quantity;
            return true;
        }
    }

    // 빈 슬롯 탐색
    int32 TargetSlot = FindEmptySlot();
    if (TargetSlot != -1)
    {
        ContainerItems[TargetSlot] = ItemData;
        return true;
    }

    return false;
}

bool AItemContainer::TryRemoveItemFromContainer(int32 SlotIndex, int32 Quantity)
{
    if (!HasAuthority() || !ContainerItems.IsValidIndex(SlotIndex))
    {
        return false;
    }

    FContainerItemData& Slot = ContainerItems[SlotIndex];

    if (!Slot.IsValid() || Slot.Quantity < Quantity)
    {
        return false;
    }

    Slot.Quantity -= Quantity;

    if (Slot.Quantity <= 0)
    {
        Slot = FContainerItemData(); // 슬롯 초기화
    }

    return true;
}

bool AItemContainer::TrySwapContainerSlots(int32 FromIndex, int32 ToIndex)
{
    // 컨테이너에서 슬롯간 스왑이 필요한 기능일까?
    return false;
}

void AItemContainer::Server_MoveItemToContainer_Implementation(APlayerController* Player, int32 PlayerSlotIndex, int32 ContainerSlotIndex, int32 Quantity)
{
    ABaseCharacter* Character = Player ? Cast<ABaseCharacter>(Player->GetPawn()) : nullptr;
    if (!Character)
    {
        return;
    }

    UToolbarInventoryComponent* PlayerInventory = Character->GetToolbarInventoryComponent();
    if (!PlayerInventory || !PlayerInventory->ItemSlots.IsValidIndex(PlayerSlotIndex))
    {
        return;
    }

    FBaseItemSlotData& PlayerSlot = PlayerInventory->ItemSlots[PlayerSlotIndex];

    // 플레이어 슬롯 검증
    if (PlayerSlot.ItemRowName == FName("Default") || PlayerSlot.Quantity < Quantity)
    {
        return;
    }
    
    // 장착된 아이템인 경우 먼저 장착 해제
    if (PlayerSlot.bIsEquipped)
    {
        // 현재 장착 슬롯이 이동하려는 슬롯인지 확인
        int32 CurrentEquippedSlot = PlayerInventory->GetCurrentEquippedSlotIndex();
        if (CurrentEquippedSlot == PlayerSlotIndex)
        {
            // 장착 해제
            PlayerInventory->UnequipCurrentItem();

            LOG_Item_WARNING(TEXT("[ItemContainer] 장착된 아이템 해제: %s"),
                *PlayerSlot.ItemRowName.ToString());
        }

        // 장착 플래그 초기화
        PlayerSlot.bIsEquipped = false;
    }

    FContainerItemData ItemToAdd = ConvertToContainerData(PlayerSlot, Quantity);

    if (TryAddItemToContainer(ItemToAdd, ContainerSlotIndex))
    {
        PlayerSlot.Quantity -= Quantity;
        if (PlayerSlot.Quantity <= 0)
        {
            PlayerInventory->SetSlotToDefault(PlayerSlotIndex);
        }

        PlayerInventory->UpdateWeight();
        PlayerInventory->OnInventoryUpdated.Broadcast();
        UpdateContainerUI();
    }
}

void AItemContainer::Server_MoveItemToPlayer_Implementation(APlayerController* Player, int32 ContainerSlotIndex, int32 PlayerSlotIndex, int32 Quantity)
{
    if (!ContainerItems.IsValidIndex(ContainerSlotIndex))
    {
        return;
    }

    FContainerItemData& ContainerSlot = ContainerItems[ContainerSlotIndex];

    if (!ContainerSlot.IsValid() || ContainerSlot.Quantity < Quantity)
    {
        return;
    }

    ABaseCharacter* Character = Player ? Cast<ABaseCharacter>(Player->GetPawn()) : nullptr;
    if (!Character)
    {
        return;
    }

    UToolbarInventoryComponent* PlayerInventory = Character->GetToolbarInventoryComponent();
    if (!PlayerInventory)
    {
        return;
    }

    // 빈 슬롯 찾기
    int32 TargetSlot = PlayerInventory->FindEmptySlot();
    if (TargetSlot == -1)
    {
        return;
    }

    // 구조체 정보를 슬롯에 적용
    FBaseItemSlotData NewSlot;
    ApplyContainerDataToSlot(NewSlot, ContainerSlot);
    NewSlot.Quantity = Quantity;  // 이동할 수량만큼만

    PlayerInventory->ItemSlots[TargetSlot] = NewSlot;

    // 컨테이너에서 제거
    ContainerSlot.Quantity -= Quantity;
    if (ContainerSlot.Quantity <= 0)
    {
        ContainerSlot = FContainerItemData();
    }

    PlayerInventory->UpdateWeight();
    PlayerInventory->OnInventoryUpdated.Broadcast();
    UpdateContainerUI();
}

void AItemContainer::Server_OpenContainer_Implementation(APlayerController* Player)
{
    if (!Player)
    {
        return;
    }

    OnContainerOpened.Broadcast(Player);

    LOG_Item_WARNING(TEXT("[ItemContainer] 컨테이너 열림: %s"), *Player->GetName());
}

void AItemContainer::Server_CloseContainer_Implementation(APlayerController* Player)
{
    if (!Player)
    {
        return;
    }

    OnContainerClosed.Broadcast(Player);

    LOG_Item_WARNING(TEXT("[ItemContainer] 컨테이너 닫힘: %s"), *Player->GetName());
}

void AItemContainer::UpdateContainerUI()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
            {
                if (ULCUIManager* UIManager = Subsystem->GetUIManager())
                {
                    if (UInventoryMainWidget* MainWidget = UIManager->GetInventoryMainWidget())
                    {
                        // 컨테이너 UI가 열려있는 경우에만 업데이트
                        if (MainWidget->IsContainerUIOpen())
                        {
                            MainWidget->ShowContainerUI(this, ContainerItems);
                            LOG_Item_WARNING(TEXT("[ItemContainer] 컨테이너 UI 업데이트"));
                        }
                    }
                }
            }
        }
    }
}

void AItemContainer::Client_ShowContainerUI_Implementation(APlayerController* Player)
{
    if (!Player || !Player->IsLocalController())
    {
        return;
    }

    // 클라이언트에서 UI 표시
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (ULCGameInstanceSubsystem* Subsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
            {
                if (ULCUIManager* UIManager = Subsystem->GetUIManager())
                {
                    if (UInventoryMainWidget* MainWidget = UIManager->GetInventoryMainWidget())
                    {
                        MainWidget->ShowContainerUI(this, ContainerItems);
                        LOG_Item_WARNING(TEXT("[ItemContainer] 클라이언트 UI 열림"));
                    }
                }
            }
        }
    }
}

int32 AItemContainer::FindEmptySlot() const
{
    for (int32 i = 0; i < ContainerItems.Num(); ++i)
    {
        if (!ContainerItems[i].IsValid())
        {
            return i;
        }
    }
    return -1;
}

int32 AItemContainer::FindStackableSlot(FName ItemRowName) const
{
    for (int32 i = 0; i < ContainerItems.Num(); ++i)
    {
        const FContainerItemData& Slot = ContainerItems[i];
        if (Slot.IsValid() && Slot.ItemRowName == ItemRowName)
        {
            // TODO: MaxStack 검사 로직 추가
            return i;
        }
    }
    return -1;
}

void AItemContainer::ForceCloseContainerForPlayer(APlayerController* Player)
{
    if (!Player || !HasAuthority())
    {
        return;
    }

    // 클라이언트에게 컨테이너 닫기 알림
    if (ABaseCharacter* Character = Cast<ABaseCharacter>(Player->GetPawn()))
    {
        if (Character->ContainerInteractionComponent)
        {
            Character->ContainerInteractionComponent->Client_ForceCloseContainer(this);
        }
    }

    OnContainerClosed.Broadcast(Player);

    LOG_Item_WARNING(TEXT("[ItemContainer] 거리 초과로 컨테이너 강제 닫기: %s"),
        *Player->GetName());
}

void AItemContainer::SetupInteractionSphere()
{
    if (InteractionSphere)
    {
        InteractionSphere->SetSphereRadius(InteractionDistance);

        LOG_Item_WARNING(TEXT("[ItemContainer] 상호작용 박스 크기 설정: %.1f"), InteractionDistance);
    }
}

FContainerItemData AItemContainer::ConvertToContainerData(const FBaseItemSlotData& SlotData, int32 Quantity) const
{
    FContainerItemData Result;
    Result.ItemRowName = SlotData.ItemRowName;
    Result.Quantity = Quantity;
    Result.Durability = SlotData.Durability;
    Result.CurrentAmmo = SlotData.CurrentAmmo;
    Result.FireMode = SlotData.FireMode;
    Result.bWasAutoFiring = SlotData.bWasAutoFiring;
    return Result;
}

void AItemContainer::ApplyContainerDataToSlot(FBaseItemSlotData& OutSlotData, const FContainerItemData& ContainerData) const
{
    OutSlotData.ItemRowName = ContainerData.ItemRowName;
    OutSlotData.Quantity = ContainerData.Quantity;
    OutSlotData.Durability = ContainerData.Durability;
    OutSlotData.CurrentAmmo = ContainerData.CurrentAmmo;
    OutSlotData.FireMode = ContainerData.FireMode;
    OutSlotData.bWasAutoFiring = ContainerData.bWasAutoFiring;
    OutSlotData.bIsValid = ContainerData.IsValid();
}
