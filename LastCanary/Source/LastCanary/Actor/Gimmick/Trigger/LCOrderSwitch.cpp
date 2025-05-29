#include "Actor/Gimmick/Trigger/LCOrderSwitch.h"
#include "Actor/Gimmick/Manager/LCOrderPuzzleManager.h"

#include "LastCanary.h"

ALCOrderSwitch::ALCOrderSwitch()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ALCOrderSwitch::BeginPlay()
{
	Super::BeginPlay();
}

void ALCOrderSwitch::ActivateGimmick_Implementation()
{
	if (HasAuthority() && PuzzleManager)
	{
		PuzzleManager->NotifySwitchPressed(SwitchIndex);
		LOG_Frame_WARNING(TEXT("[OrderSwitch] Switch %d activated."), SwitchIndex);
	}
}