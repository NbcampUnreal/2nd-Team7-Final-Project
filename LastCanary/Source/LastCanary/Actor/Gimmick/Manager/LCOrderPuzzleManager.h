#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LCOrderPuzzleManager.generated.h"

class ALCOrderPlate;
UCLASS()
class LASTCANARY_API ALCOrderPuzzleManager : public AActor
{
	GENERATED_BODY()

public:
	ALCOrderPuzzleManager();

	virtual void BeginPlay() override;

	/** 플레이어가 발판을 밟았을 때 호출됨 */
	UFUNCTION(BlueprintCallable)
	void NotifySwitchPressed(int32 Index);

protected:
	/** 올바른 순서 인덱스 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
	TArray<int32> ExpectedOrder;

	UPROPERTY(EditAnywhere, Category = "Puzzle")
	TMap<int32, TObjectPtr<ALCOrderPlate>> PlateMap;

	/** 현재 몇 번째까지 맞췄는지 */
	UPROPERTY(VisibleAnywhere)
	int32 CurrentIndex = 0;

	/** 퍼즐 성공 시 작동할 타겟 액터들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
	TArray<AActor*> LinkedTargets;
	
	void ResetPuzzle();
	void OnPuzzleSuccess();
};