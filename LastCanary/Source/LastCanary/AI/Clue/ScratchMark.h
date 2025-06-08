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

private:

};
