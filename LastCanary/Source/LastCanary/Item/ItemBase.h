#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataTable/ItemDataRow.h"
#include "Interface/InteractableInterface.h"
#include "Engine/Engine.h"
#include "ItemBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemStateChanged);

UCLASS()
class LASTCANARY_API AItemBase : public AActor, public IInteractableInterface
{
    GENERATED_BODY()

public:
    AItemBase();

protected:
    virtual void BeginPlay() override;

public:
    //-----------------------------------------------------
    // 컴포넌트
    //-----------------------------------------------------

    /** 루트 씬 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    /** 아이템의 스태틱 메시 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    /** 아이템의 스켈레탈 메시 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletalMeshComponent;

    //-----------------------------------------------------
    // 메시 관리
    //-----------------------------------------------------

    /** 현재 활성화된 메시 타입 */
    UPROPERTY(BlueprintReadOnly, Category = "Mesh")
    bool bUsingSkeletalMesh = false;

    /** 활성화된 메시 컴포넌트 반환 (StaticMesh 또는 SkeletalMesh) */
    UFUNCTION(BlueprintPure, Category = "Mesh")
    UPrimitiveComponent* GetActiveMeshComponent() const;

    /** 스태틱 메시 컴포넌트 반환 */
    UStaticMeshComponent* GetMeshComponent() const;

    /** 스켈레탈 메시 컴포넌트 반환 */
    UFUNCTION(BlueprintPure, Category = "Mesh")
    USkeletalMeshComponent* GetSkeletalMeshComponent() const;

protected:
    /** 메시 타입 설정 및 적용 */
    void SetupMeshComponents();

    /** 메시 컴포넌트 활성화/비활성화 */
    void SetMeshComponentActive(UPrimitiveComponent* ActiveComponent, UPrimitiveComponent* InactiveComponent);

    //-----------------------------------------------------
    // 아이템 데이터
    //-----------------------------------------------------
public:
    /** 아이템 데이터 테이블에서의 행 이름 */
    UPROPERTY(ReplicatedUsing = OnRepItemRowName, EditAnywhere, BlueprintReadWrite, Category = Item)
    FName ItemRowName;

    /** 아이템 데이터 테이블 참조 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
    UDataTable* ItemDataTable;

    /** 아이템의 실제 데이터 (데이터 테이블에서 로드됨) */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Item, meta = (ShowOnlyInnerProperties))
    FItemDataRow ItemData;

    //-----------------------------------------------------
    // 아이템 상태
    //-----------------------------------------------------

    /** 아이템이 장착되어 있는지 여부 */
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Item|State")
    bool bIsEquipped;

    /** 아이템의 현재 수량 (스택 가능 아이템용) */
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|State")
    int32 Quantity;

    /** 아이템 내구도/배터리량 (소모성 장비용) */
    UPROPERTY(ReplicatedUsing = OnRepDurability, EditAnywhere, BlueprintReadWrite, Category = "Item|State")
    float Durability;

    /** Durability가 복제될 때 호출되는 함수 */
    UFUNCTION()
    virtual void OnRepDurability();

    //-----------------------------------------------------
    // 이벤트 및 델리게이트
    //-----------------------------------------------------

    /** 아이템 상태 변경 시 호출되는 델리게이트 */
    UPROPERTY(BlueprintAssignable, Category = "Item|Events")
    FOnItemStateChanged OnItemStateChanged;

    //-----------------------------------------------------
    // 아이템 상호작용 함수
    //-----------------------------------------------------

    /** 아이템 사용 함수 - 하위 클래스에서 반드시 구현 */
    UFUNCTION(BlueprintCallable, Category = "Item|Interaction")
    virtual void UseItem() PURE_VIRTUAL(AItemBase::UseItem, return;);

    /** 아이템이 수집 가능한지 확인 */
    UFUNCTION(BlueprintPure, Category = "Item|Interaction")
    bool IsCollectible() const;

    //-----------------------------------------------------
    // InteractableInterface 구현
    //-----------------------------------------------------
public:
    /** 플레이어가 상호작용할 때 호출 (인터페이스 구현) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(APlayerController* Interactor);
    virtual void Interact_Implementation(APlayerController* Interactor);

    /** 상호작용 메시지 반환 (인터페이스 구현) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FString GetInteractMessage() const;
    virtual FString GetInteractMessage_Implementation() const;

protected:
    /** 아이템 습득 시도 (서버에서 실행) */
    UFUNCTION(Server, Reliable, Category = "Item|Pickup")
    void Server_TryPickupByPlayer(APlayerController* PlayerController);
    void Server_TryPickupByPlayer_Implementation(APlayerController* PlayerController);

    /** 실제 아이템 습득 로직 */
    bool Internal_TryPickupByPlayer(APlayerController* PlayerController);

    //-----------------------------------------------------
    // 충돌 이벤트 핸들러
    //-----------------------------------------------------
public:
    /** 아이템과 다른 액터 간의 오버랩 시작 시 호출 */
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /** 아이템과 다른 액터 간의 오버랩 종료 시 호출 */
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    //-----------------------------------------------------
    // 유틸리티 함수
    //-----------------------------------------------------

    /** 데이터 테이블에서 아이템 데이터를 로드하여 적용 */
    UFUNCTION(BlueprintCallable, Category = "Item|Initialization")
    void ApplyItemDataFromTable();

    //-----------------------------------------------------
    // 네트워크 & 에디터 기능
    //-----------------------------------------------------

    /** 리플리케이션 속성 설정 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** ItemRowName이 복제될 때 호출되는 함수 */
    UFUNCTION()
    virtual void OnRepItemRowName();

#if WITH_EDITOR
    /** 에디터에서 속성 변경 시 호출 */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    // virtual void OnConstruction(const FTransform& Transform) override;
#endif

    UPROPERTY(Replicated)
    bool bIgnoreCharacterCollision = false;

    void ApplyCollisionSettings();

//public:
//    /** 클라이언트에 시각적 효과만 전달 (물리 시뮬레이션 없음) */
//    UFUNCTION(NetMulticast, Reliable)
//    void Multicast_SetupVisualEffects(FVector ThrowDirection, float ThrowVelocity, FVector ThrowImpulse);
//    void Multicast_SetupVisualEffects_Implementation(FVector ThrowDirection, float ThrowVelocity, FVector ThrowImpulse);
};
