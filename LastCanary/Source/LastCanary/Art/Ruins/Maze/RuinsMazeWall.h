#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuinsMazeWall.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ARuinsMazeWall : public AActor
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
