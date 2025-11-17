#include "Actor/Gimmick/Manager/LCOrderPuzzleManager.h"
#include "Interface/GimmickEffectInterface.h"
#include "Actor/Gimmick/Trigger/LCOrderPlate.h"

#include "LastCanary.h"

ALCOrderPuzzleManager::ALCOrderPuzzleManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ALCOrderPuzzleManager::BeginPlay()
{
    Super::BeginPlay();

    for (auto& Elem : PlateMap)
    {
        if (IsValid(Elem.Value))
        {
            Elem.Value->ApplyMaterial(false);
        }
    }
    CurrentIndex = 0;
}

void ALCOrderPuzzleManager::NotifySwitchPressed(int32 Index)
{
	if (ExpectedOrder.IsValidIndex(CurrentIndex)==false)
	{
		return;
	}

	if (ExpectedOrder[CurrentIndex] == Index)
	{
		// 맞은 경우: 선택 상태로 변경
		if (ALCOrderPlate* Plate = PlateMap[Index])
		{
			Plate->ApplyMaterial(true);
		}
		++CurrentIndex;

		if (CurrentIndex == ExpectedOrder.Num())
		{
			OnPuzzleSuccess();
		}
	}
	else
	{
		ResetPuzzle();
	}
}

void ALCOrderPuzzleManager::ResetPuzzle()
{
	// 전체 초기화
	for (auto& Elem : PlateMap)
	{
		if (IsValid(Elem.Value))
		{
			Elem.Value->ApplyMaterial(false);
		}
	}
	CurrentIndex = 0;
}

void ALCOrderPuzzleManager::OnPuzzleSuccess()
{
	LOG_Frame_WARNING(TEXT("[OrderPuzzle] Puzzle completed successfully!"));

	for (AActor* Target : LinkedTargets)
	{
		if (IsValid(Target) && Target->GetClass()->ImplementsInterface(UGimmickEffectInterface::StaticClass()))
		{
			IGimmickEffectInterface::Execute_TriggerEffect(Target);
		}
	}
}