// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableInterface.h"
#include "Components/SphereComponent.h"
#include "ItemContainer.generated.h"

USTRUCT(BlueprintType)
struct FContainerItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    FName ItemRowName = FName("Default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container", meta = (ClampMin = 0))
    int32 Quantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
    float Durability = 100.0f;

    FContainerItemData()
    {
        ItemRowName = FName("Default");
        Quantity = 0;
        Durability = 100.0f;
    }

    bool IsValid() const
    {
        return ItemRowName != FName("Default") && Quantity > 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerOpened, APlayerController*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerClosed, APlayerController*, Interactor);

UCLASS()
class LASTCANARY_API AItemContainer : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AItemContainer();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------

    /** 상호작용 범위 감지용 박스 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Components")
    USphereComponent* InteractionSphere;

    /** 컨테이너 메시*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Components")
    UStaticMeshComponent* ContainerMesh;

    //-----------------------------------------------------
    // 상호작용 설정
    //-----------------------------------------------------

    /** 상호작용 가능 거리 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings")
    float InteractionDistance = 300.0f;

    //-----------------------------------------------------
    // 인터랙션 인터페이스 구현
    //-----------------------------------------------------

public:
    virtual void Interact_Implementation(APlayerController* Interactor) override;
    virtual FString GetInteractMessage_Implementation() const override;

    //-----------------------------------------------------
    // 오버랩 이벤트 핸들러
    //-----------------------------------------------------

    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    //-----------------------------------------------------
    // 컨테이너 데이터
    //-----------------------------------------------------

protected:
    /** 컨테이너에 저장된 아이템 데이터 배열 */
    UPROPERTY(ReplicatedUsing = OnRep_ContainerItems, EditAnywhere, BlueprintReadWrite, Category = "Container|Data")
    TArray<FContainerItemData> ContainerItems;

    /** 컨테이너 최대 슬롯 수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings")
    int32 MaxSlots = 20;

    /** 컨테이너 이름 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container|Settings")
    FText ContainerName = NSLOCTEXT("Container", "DefaultName", "Container");

public:
    /** 컨테이너 데이터 복제 시 호출 */
    UFUNCTION()
    void OnRep_ContainerItems();

    /** 컨테이너 데이터 접근 함수들 */
    UFUNCTION(BlueprintPure, Category = "Container|Data")
    const TArray<FContainerItemData>& GetContainerItems() const { return ContainerItems; }

    UFUNCTION(BlueprintPure, Category = "Container|Data")
    int32 GetMaxSlots() const { return MaxSlots; }

    UFUNCTION(BlueprintPure, Category = "Container|Data")
    FText GetContainerName() const { return ContainerName; }

    //-----------------------------------------------------
    // 컨테이너 조작 함수들 (서버에서만 실행)
    //-----------------------------------------------------

public:
    /** 컨테이너에 아이템 추가 */
    UFUNCTION(BlueprintCallable, Category = "Container|Operations")
    bool TryAddItemToContainer(FName ItemRowName, int32 Quantity, int32 SlotIndex = -1);

    /** 컨테이너에서 아이템 제거 */
    UFUNCTION(BlueprintCallable, Category = "Container|Operations")
    bool TryRemoveItemFromContainer(int32 SlotIndex, int32 Quantity = 1);

    /** 컨테이너 슬롯 간 아이템 교환 */
    UFUNCTION(BlueprintCallable, Category = "Container|Operations")
    bool TrySwapContainerSlots(int32 FromIndex, int32 ToIndex);

    /** 플레이어 인벤토리에서 컨테이너로 아이템 이동 */
    UFUNCTION(Server, Reliable, Category = "Container|Operations")
    void Server_MoveItemToContainer(APlayerController* Player, int32 PlayerSlotIndex, int32 ContainerSlotIndex, int32 Quantity);
    void Server_MoveItemToContainer_Implementation(APlayerController* Player, int32 PlayerSlotIndex, int32 ContainerSlotIndex, int32 Quantity);

    /** 컨테이너에서 플레이어 인벤토리로 아이템 이동 */
    UFUNCTION(Server, Reliable, Category = "Container|Operations")
    void Server_MoveItemToPlayer(APlayerController* Player, int32 ContainerSlotIndex, int32 PlayerSlotIndex, int32 Quantity);
    void Server_MoveItemToPlayer_Implementation(APlayerController* Player, int32 ContainerSlotIndex, int32 PlayerSlotIndex, int32 Quantity);

    //-----------------------------------------------------
    // 컨테이너 UI 관리
    //-----------------------------------------------------

public:
    /** 컨테이너 열기 */
    UFUNCTION(Server, Reliable, Category = "Container|UI")
    void Server_OpenContainer(APlayerController* Player);
    void Server_OpenContainer_Implementation(APlayerController* Player);

    /** 컨테이너 닫기 */
    UFUNCTION(Server, Reliable, Category = "Container|UI")
    void Server_CloseContainer(APlayerController* Player);
    void Server_CloseContainer_Implementation(APlayerController* Player);

    void UpdateContainerUI();

    //-----------------------------------------------------
    // 이벤트 델리게이트
    //-----------------------------------------------------

public:
    UPROPERTY(BlueprintAssignable, Category = "Container|Events")
    FOnContainerOpened OnContainerOpened;

    UPROPERTY(BlueprintAssignable, Category = "Container|Events")
    FOnContainerClosed OnContainerClosed;

protected:
    //-----------------------------------------------------
    // 내부 헬퍼 함수들
    //-----------------------------------------------------

    /** 빈 슬롯 찾기 */
    int32 FindEmptySlot() const;

    /** 스택 가능한 슬롯 찾기 */
    int32 FindStackableSlot(FName ItemRowName) const;

    /** 컨테이너 초기화 */
    void InitializeContainer();

    /** 컨테이너 데이터 유효성 검증 */
    bool ValidateContainerData();

    /** 범위를 벗어난 플레이어의 컨테이너 강제 닫기 */
    void ForceCloseContainerForPlayer(APlayerController* Player);

    /** 상호작용 박스 크기 설정 */
    void SetupInteractionSphere();
};
