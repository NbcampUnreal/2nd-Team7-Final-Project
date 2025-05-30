#pragma once

#include "CoreMinimal.h"
#include "Item/ItemBase.h"
#include "DataTable/ResourceItemRow.h"
#include "ResourceItemBase.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API AResourceItemBase : public AItemBase
{
	GENERATED_BODY()
	
public:
    AResourceItemBase();

protected:
    virtual void BeginPlay() override;

public:
    /** 자원 아이템 전용 데이터 (ResourceItemRow 참조) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
    UDataTable* ResourceDataTable;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Resource", meta = (ShowOnlyInnerProperties))
    FResourceItemRow ResourceData;

    /** ResourceID 기준으로 ResourceDataTable에서 로드 */
    UFUNCTION(BlueprintCallable, Category = "Resource|Initialization")
    void ApplyResourceDataFromTable();

    /** 자원 채집 또는 평가용 점수 반환 */
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetResourceScore() const;

    /** 자원 아이템 특화 상호작용 메시지 */
    virtual FString GetInteractMessage_Implementation() const override;

    /** UseItem 오버라이드 */
    virtual void UseItem() override;
};
