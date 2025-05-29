#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/LCGimmickInterface.h"
#include "LCOrderSwitch.generated.h"

class ALCOrderPuzzleManager;
UCLASS()
class LASTCANARY_API ALCOrderSwitch : public AActor, public ILCGimmickInterface
{
	GENERATED_BODY()
	
public:
	ALCOrderSwitch();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "OrderPuzzle")
	int32 SwitchIndex = -1;

	UPROPERTY(EditAnywhere, Category = "OrderPuzzle")
	ALCOrderPuzzleManager* PuzzleManager;

	virtual void ActivateGimmick_Implementation() override;
};
