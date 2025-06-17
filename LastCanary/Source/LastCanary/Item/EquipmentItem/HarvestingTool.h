#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "HarvestingTool.generated.h"
 
class AResourceNode;

/**
 * 
 */
UCLASS()
class LASTCANARY_API AHarvestingTool : public AEquipmentItemBase
{
	GENERATED_BODY()
	
public:
    AHarvestingTool();

protected:
    virtual void BeginPlay() override;

public:
    // UseItem 오버라이드 - 자원 채취 실행
    virtual void UseItem() override;

    // 서버에서 자원 채취 요청 처리
    UFUNCTION(Server, Reliable)
    void Server_RequestHarvest();
    void Server_RequestHarvest_Implementation();

    // 자원 노드 탐지 및 채취 로직
    UFUNCTION(BlueprintCallable, Category = "Harvesting")
    AResourceNode* FindNearestResourceNode() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvesting")
    bool bShowDebugTrace = false;

    UFUNCTION(BlueprintCallable, Category = "Harvesting")
    bool CanHarvestNode(AResourceNode* Node) const;

    // 채취 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvesting")
    float HarvestRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvesting")
    float HarvestCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvesting")
    float DurabilityConsumption = 1.0f;

protected:
    // 쿨다운 관리
    UPROPERTY(BlueprintReadOnly, Category = "Harvesting")
    float LastHarvestTime = 0.0f;

    // 내구도 소모 처리
    void ConsumeDurability();

    // 채취 가능 여부 확인
    bool IsHarvestReady() const;

    // 카메라 방향 벡터 계산
    bool PerformHarvestTrace(FHitResult& OutHit, FVector& StartLocation, FVector& EndLocation) const;
};
