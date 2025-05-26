#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataTable/ItemDataRow.h"
#include "ItemBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemStateChanged);

UCLASS()
class LASTCANARY_API AItemBase : public AActor
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

    /** 아이템의 기본 메시 컴포넌트 */
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* MeshComponent;

    /** 아이템 상호작용 감지용 구체 컴포넌트 */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    class USphereComponent* SphereComponent;

    //-----------------------------------------------------
    // 아이템 데이터
    //-----------------------------------------------------

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
    // 충돌 이벤트 핸들러
    //-----------------------------------------------------

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

    UStaticMeshComponent* GetMeshComponent() const;

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
};
