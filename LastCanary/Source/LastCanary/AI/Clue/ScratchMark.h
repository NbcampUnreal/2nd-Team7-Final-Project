#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScratchMark.generated.h"

class UDecalComponent;
class UMaterialInterface;

UCLASS()
class LASTCANARY_API AScratchMark : public AActor
{
	GENERATED_BODY()
	
public:
    AScratchMark();

protected:
    virtual void BeginPlay() override;

    /** 데칼 컴포넌트 (긁힌 자국을 벽/바닥에 투영) */
    UPROPERTY(VisibleAnywhere, Category = "Clue|Components")
    UDecalComponent* DecalComp;

    /** 사용할 데칼 머티리얼 */
    UPROPERTY(EditAnywhere, Category = "Clue")
    UMaterialInterface* ScratchMarkMaterial;

    /** 데칼 크기 (X,Y,Z) */
    UPROPERTY(EditAnywhere, Category = "Clue", meta = (ClampMin = "1.0"))
    FVector DecalSize = FVector(64.f, 64.f, 64.f);

    /** 생성 후 일정 시간 지나면 자동으로 파괴 (초) */
    UPROPERTY(EditAnywhere, Category = "Clue", meta = (ClampMin = "0.1"))
    float LifeTime = 15.f;

private:
    /** 내부에서 타이머 핸들을 저장할 변수 */
    FTimerHandle LifetimeHandle;

    /** 데칼이 생성된 뒤 일정 시간 후 제거하는 함수 */
    void OnLifeTimeExpired();

};
