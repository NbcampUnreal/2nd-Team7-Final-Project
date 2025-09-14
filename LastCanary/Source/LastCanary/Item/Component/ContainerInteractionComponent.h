#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemContainer.h"
#include "ContainerInteractionComponent.generated.h"

class UToolbarInventoryComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UContainerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UContainerInteractionComponent();

protected:
    virtual void BeginPlay() override;

public:
    //-----------------------------------------------------
    // 컨테이너 상호작용 Server RPC
    //-----------------------------------------------------

    /** 컨테이너와 상호작용 */
    UFUNCTION(Server, Reliable)
    void Server_InteractWithContainer(AItemContainer* Container);
    void Server_InteractWithContainer_Implementation(AItemContainer* Container);

    /** 플레이어 인벤토리에서 컨테이너로 아이템 이동 */
    UFUNCTION(Server, Reliable)
    void Server_MoveItemToContainer(AItemContainer* Container, int32 PlayerSlotIndex, int32 ContainerSlotIndex, int32 Quantity);
    void Server_MoveItemToContainer_Implementation(AItemContainer* Container, int32 PlayerSlotIndex, int32 ContainerSlotIndex, int32 Quantity);

    /** 컨테이너에서 플레이어 인벤토리로 아이템 이동 */
    UFUNCTION(Server, Reliable)
    void Server_MoveItemToPlayer(AItemContainer* Container, int32 ContainerSlotIndex, int32 PlayerSlotIndex, int32 Quantity);
    void Server_MoveItemToPlayer_Implementation(AItemContainer* Container, int32 ContainerSlotIndex, int32 PlayerSlotIndex, int32 Quantity);

    /** 컨테이너 닫기 */
    UFUNCTION(Server, Reliable)
    void Server_CloseContainer(AItemContainer* Container);
    void Server_CloseContainer_Implementation(AItemContainer* Container);

    UFUNCTION(Client, Reliable, Category = "Container|UI")
    void Client_ShowContainerUI(AItemContainer* Container, const TArray<FContainerItemData>& Items);
    void Client_ShowContainerUI_Implementation(AItemContainer* Container, const TArray<FContainerItemData>& Items);

    /** 컨테이너 강제 닫기 (거리 초과 시) */
    UFUNCTION(Client, Reliable, Category = "Container|UI")
    void Client_ForceCloseContainer(AItemContainer* Container);
    void Client_ForceCloseContainer_Implementation(AItemContainer* Container);

    //-----------------------------------------------------
    // 헬퍼 함수들
    //-----------------------------------------------------

    /** 소유자 캐릭터의 인벤토리 컴포넌트 가져오기 */
    UToolbarInventoryComponent* GetOwnerInventoryComponent() const;

    /** 소유자 플레이어 컨트롤러 가져오기 */
    APlayerController* GetOwnerPlayerController() const;

    /** 소유자 캐릭터 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Container Interaction")
    ABaseCharacter* GetOwnerCharacter() const;

    /** 컨테이너 상호작용 가능 여부 확인 */
    UFUNCTION(BlueprintPure, Category = "Container Interaction")
    bool CanInteractWithContainer(AItemContainer* Container) const;

    /** 아이템 이동 유효성 검사 */
    UFUNCTION(BlueprintPure, Category = "Container Interaction")
    bool ValidateItemMove(int32 SourceSlotIndex, int32 TargetSlotIndex, int32 Quantity) const;

private:
    /** 캐싱된 소유자 캐릭터 */
    UPROPERTY()
    ABaseCharacter* CachedOwnerCharacter;

    /** 캐싱된 인벤토리 컴포넌트 */
    UPROPERTY()
    UToolbarInventoryComponent* CachedInventoryComponent;

    /** 현재 열린 컨테이너 */
    UPROPERTY()
    AItemContainer* CurrentOpenContainer;

    /** 소유자 캐릭터 캐싱 */
    void CacheOwnerReferences();

    /** 컨테이너 열기 전 검증 */
    bool ValidateContainerInteraction(AItemContainer* Container) const;

    /** 아이템 이동 전 검증 */
    bool ValidateItemTransfer(AItemContainer* Container, int32 SourceSlot, int32 TargetSlot, int32 Quantity, bool bToContainer) const;
};
