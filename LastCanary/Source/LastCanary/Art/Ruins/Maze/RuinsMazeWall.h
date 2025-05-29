#pragma once

#include "CoreMinimal.h"
#include "Art/Ruins/RuinsGimmickBase.h"
#include "RuinsMazeWall.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ARuinsMazeWall : public ARuinsGimmickBase
{
	GENERATED_BODY()

public:
	ARuinsMazeWall();

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> WallMesh;
};
